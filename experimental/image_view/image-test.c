/*
 gcc -std=c99 -g -O2      -Wextra -Wall         -I/usr/X11R6/include     -L/usr/X11R6/lib -lX11 -lXext -lXrender -lXcomposite -lrt    -o imt image-test.c -lX11 -lXrender -lXcomposite -lXext
*/

/**
 * @file benchimagemark.c
 * @brief Benchmark methods to update window content
 *        
 * @date 2009-01-16 created
 * @author Yann Droneaud <ydroneaud@mandriva.com>
 *
 * Note: When compositing is enabled, 
 * the window content below could impact the results
 * It's really better if this content does not change during
 * the test.
 *
 * Compile/Link:
 *  gcc -std=c99 -g -O2   \
 *   -Wextra -Wall	  \
 *   -I/usr/X11R6/include \
 *   -L/usr/X11R6/lib -lX11 -lXext -lXrender -lXcomposite -lrt \
 *   -o benchimagemark benchimagemark.c
 * 
 */

/*
 * Copyright (C) 2009 Mandriva SA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#define _POSIX_SOURCE
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE_EXTENDED
#define _ISOC99_SOURCE

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#include <errno.h>
#include <string.h>

#include <assert.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

/**
 * @brief substract two timespec values and store the result
 *
 * @param[out] res
 * @param[in] before
 * @param[in] after
 *
 */
static inline void
timespec_substract(struct timespec *res,
		   const struct timespec *before,
		   const struct timespec *after)
{
  *res = *after;
  
  if (res->tv_nsec < before->tv_nsec) {
    res->tv_sec -= 1;
    res->tv_nsec += (1000000000L) - before->tv_nsec; 
  } else {
    res->tv_nsec -= before->tv_nsec; 
  }
  
  res->tv_sec -= before->tv_sec;
}

static Display *display = NULL;
static Window   root = None;
static Window   window = None;
static Visual  *visual = NULL;

static XGCValues gcvalues;
static GC gc = None;
  
static XImage *ximage = NULL;
static XImage *shmximage_ximage = NULL;
static XImage *shmximage_pixmap = NULL;

static Pixmap pixmap = None;
static Pixmap shmpixmap = None;

static uint8_t *data = NULL;
static uint8_t *shmdata_ximage = NULL;
static uint8_t *shmdata_pixmap = NULL;

static Picture pixmap_pict;
static Picture shmpixmap_pict;
static Picture window_pict;

/* test id */
static int test = 0;

static void
flush_xevent(Display *d)
{
  /* flush events */				
  for(;;) {
    XEvent e;
    
    /* check X event */
    if (!XPending(d)) {
      break;
    }
    
    XNextEvent(d, &e);
    printf("event %d\n", e.type);
  }
}

/**
 * Number of window updates per test
 *
 */
#define BENCHMARK_RUN 10000

/**
 * start a test:
 *  check for condition,
 *  ensure all requests and events are processed
 *  start clock
 *  run the test
 */
#define TEST_BEGIN(name, cond)			\
  do {						\
   const char * test_name = name;		\
						\
  if (!(cond)) {				\
    printf("%s: skipped\n", test_name);		\
  } else {					\
						\
    struct timespec begin;			\
    struct timespec end;			\
    struct timespec duration;			\
						\
    unsigned int value = 0x1fU;                 \
						\
    test++;					\
						\
    /* be sure no pending request remaining */	\
    XSync(display, False);			\
    flush_xevent(display);                      \
    /* printf("%s: begin\n", test_name); */	\
						\
    clock_gettime(CLOCK_REALTIME, &begin);	\
    for(i = 0; i < BENCHMARK_RUN; i ++)

/**
 * update the data to be put in the window
 *
 */
#define TEST_UPDATE(pointer)			\
  do {							\
    value = value * 1103515245U + 12345U;		\
    memset(pointer, value & 0xffU, width * height * 4);	\
  } while(0)
      
/**
 * end the test:
 *  ensure all requests are processed
 *  compute and report time taken for the test
 */
#define TEST_END()					\
  /* be sure no pending request remaining */		\
    XSync(display, False);				\
    clock_gettime(CLOCK_REALTIME, &end);		\
							\
    timespec_substract(&duration, &begin, &end);	\
							\
    printf("%d: %s: %ld.%09ld\n",                       \
	   test,					\
	   test_name,					\
    duration.tv_sec, duration.tv_nsec);			\
  }							\
} while(0)

int
main(int argc, char *argv[])
{
  int i;
  
  unsigned int depth = 32;
  int screen;
  Colormap colormap;
  
  XSetWindowAttributes winattr;

  int count;
  XVisualInfo *xvinfos;
  XVisualInfo xvinfo_template;

  XRenderPictFormat pf;
  XRenderPictFormat *pict_format = NULL;
  XRenderPictFormat *pict_visualformat = NULL;
    
  XRenderPictureAttributes pict_attr;

  XShmSegmentInfo shminfo_ximage;
  XShmSegmentInfo shminfo_pixmap;
  
  int major;
  int minor;

  int feature_composite = False;
  int feature_render = False;
  int feature_shm_pixmap = False;
  int feature_shm = False;

  unsigned int width = 320;
  unsigned int height = 240;

  int prio;
  int sched;
  struct sched_param param;
  pid_t pid;

  pid = getpid();

  /* open display */
  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't open display '%s'\n", XDisplayName(NULL));
    return 1;
  }

  assert(XBitmapPad(display) == 32);

  /*
   * Check for Composite, Render, MIT-SHM extension,
   * TODO: check version
   */

  if (!XCompositeQueryVersion(display,
			      &major, &minor)) {
    printf("No Composite extension available\n");
  } else {
    printf("Composite: %d.%d\n", major, minor);
    feature_composite = True;
  }

  if (!XRenderQueryVersion(display,
			   &major, &minor)) {
    printf("No Render extension available\n");
  } else {
    printf("Render: %d.%d\n", major, minor);
    feature_render = True;
  }

  if (!XShmQueryVersion(display,
			&major, &minor,
			&feature_shm_pixmap)) {
    printf("no MIT-SHM extension available\n");
  } else {
    printf("MIT-SHM: %d.%d\n", major, minor);
    feature_shm = True;
    if (!feature_shm_pixmap) {
      printf("No support for shared memory pixmap\n");
    } else {
      if (XShmPixmapFormat(display) != ZPixmap) {
	printf("ShmPixmapFormat not ZPixmap\n"); /* should never happen */
      }
    }
  }

  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

  if (feature_render) {
    /* lookup a ARGB picture format */
    pict_format = XRenderFindStandardFormat(display, PictStandardARGB32);
    if (pict_format == NULL) {
      fprintf(stderr, "Can't find standard format for ARGB32\n");
      
      /* lookup an other ARGB picture format */
      pf.type = PictTypeDirect;
      pf.depth = 32;
      
      pf.direct.alphaMask = 0xff;    
      pf.direct.redMask = 0xff;
      pf.direct.greenMask = 0xff;
      pf.direct.blueMask = 0xff;
      
      pf.direct.alpha = 24;    
      pf.direct.red = 16;
      pf.direct.green = 8;
      pf.direct.blue = 0;
      
      pict_format = XRenderFindFormat(display,
				      (PictFormatType |
				       PictFormatDepth |
				       PictFormatRedMask | PictFormatRed |
				       PictFormatGreenMask | PictFormatGreen |
				       PictFormatBlueMask | PictFormatBlue |
				       PictFormatAlphaMask | PictFormatAlpha),
				      &pf,
				      0);

      if (pict_format == NULL) {
	fprintf(stderr, "Can't find format for ARGB32\n");
	return 1;
      }
    }

    /* try to lookup a RGBA visual */
    count = 0;
    xvinfo_template.screen = screen;
    xvinfo_template.depth = 32;
    xvinfo_template.bits_per_rgb = 8;
    
    xvinfos = XGetVisualInfo(display, 
			     (VisualScreenMask | 
			      VisualDepthMask | 
			      VisualBitsPerRGBMask),
			     &xvinfo_template,
			     &count);
    if (xvinfos == NULL) {
      fprintf(stderr, "No visual matching criteria\n");
    } else {
      for(i = 0; i < count; i++) {
	pict_visualformat = XRenderFindVisualFormat(display, xvinfos[i].visual);
	if (pict_visualformat != NULL &&
	    pict_visualformat->id == pict_format->id) {
	  
	  visual = xvinfos[i].visual;
	  break;
	}
      }
    }
  } else {
    /* try to lookup a RGBA visual */
    count = 0;
    xvinfo_template.screen = screen;
    xvinfo_template.depth = 32;
    xvinfo_template.class = TrueColor;
    xvinfo_template.red_mask = 0xff0000;
    xvinfo_template.green_mask = 0x00ff00;
    xvinfo_template.blue_mask = 0x0000ff;
    xvinfo_template.bits_per_rgb = 8;
    
    xvinfos = XGetVisualInfo(display, 
			     (VisualScreenMask |
			      VisualDepthMask |
			      VisualClassMask |
			      VisualRedMaskMask |
			      VisualGreenMaskMask |
			      VisualBlueMaskMask |
			      VisualBitsPerRGBMask),
			     &xvinfo_template,
			     &count);
    if (xvinfos == NULL) {
      fprintf(stderr, "No visual matching criteria\n");
    } else {
      for(i = 0; i < count; i++) {      
	if (xvinfos[i].depth == 32) {
	  visual = xvinfos[i].visual;
	  break;
	}
      }
    }
  }

  if (visual == NULL) {
    printf("No ARGB32 visual, fatal, leaving ...\n");
    return 1;
  }

  /* create/get the colormap associated to the visual */
  colormap = XCreateColormap(display,
			     root,
			     visual,
			     AllocNone);

  XInstallColormap(display,
		   colormap);

  /* create basic XImage used for XPutImage() */
  data = (uint8_t *) malloc(width * height * 4);
  if (data == NULL) {
    perror("malloc()");
    return 1;
  }

  ximage = XCreateImage(display, 
			visual,
			depth, 
			ZPixmap, /* format */
			0,
			(char *)data,
			width,
			height,
			32, /* bitmap pad: 32bits */
		        width * 4);  /* bytes per line */
  if (ximage == NULL) {
    fprintf(stderr, "Can't create XImage\n");
    return 1;
  }

  /*
   * create SHM XImage used for XShmPutImage(),
   * and associated shared memory segment
   */
  if (feature_shm) {
    shminfo_ximage.shmid = -1;
    shminfo_ximage.shmaddr = (char *) -1;
    shminfo_ximage.readOnly = False;
    
    shmximage_ximage = XShmCreateImage(display,
				       visual,
				       depth,
				       ZPixmap,
				       NULL,
				       &shminfo_ximage,
				       width,
				       height);
    
    if (shmximage_ximage == NULL) {
      fprintf(stderr, "Can't create XImage for SHM\n");
      return 1;
    }
    
    shminfo_ximage.shmid =
      shmget(IPC_PRIVATE,
	     shmximage_ximage->bytes_per_line * shmximage_ximage->height,
	     IPC_CREAT | 0600);
    
    if (shminfo_ximage.shmid == -1) {
      perror("shmget()");
      return 1;
    }
    
    shminfo_ximage.shmaddr = shmximage_ximage->data =
      shmat(shminfo_ximage.shmid, NULL, 0);
    
    shmctl(shminfo_ximage.shmid, IPC_RMID, 0);
  
    if (shminfo_ximage.shmaddr == (void *) -1 ||
	shminfo_ximage.shmaddr == NULL) {
      perror("shmat()");
      return 1;
    }

    shmdata_ximage = shmximage_ximage->data;
    
    XShmAttach(display, &shminfo_ximage);
  }

  /*
   * create the shared memory segment for the shared pixmap
   * the XImage created is just for convienience
   */
  if (feature_shm_pixmap) {
    /* for the XShmPixmap */
    shminfo_pixmap.shmid = -1;
    shminfo_pixmap.shmaddr = (char *) -1;
    shminfo_pixmap.readOnly = False;
    
    shmximage_pixmap = XShmCreateImage(display,
				       visual,
				       depth,
				       ZPixmap,
				       NULL,
				       &shminfo_pixmap,
				       width,
				       height);

    if (shmximage_pixmap == NULL) {
      fprintf(stderr, "Can't create XImage for SHM Pixmap\n");
      return 1;
    }
    
    shminfo_pixmap.shmid =
      shmget(IPC_PRIVATE,
	     shmximage_pixmap->bytes_per_line * shmximage_pixmap->height,
	     IPC_CREAT | 0600);
    
    if (shminfo_pixmap.shmid == -1) {
      perror("shmget()");
      return 1;
    }
    
    shminfo_pixmap.shmaddr = shmximage_pixmap->data =
      shmat(shminfo_pixmap.shmid, NULL, 0);
    
    shmctl(shminfo_pixmap.shmid, IPC_RMID, 0);
    
    if (shminfo_pixmap.shmaddr == (void *) -1 ||
	shminfo_pixmap.shmaddr == NULL) {
      perror("shmat()");
      return 1;
    }
    
    XShmAttach(display, &shminfo_pixmap);
    
    shmdata_pixmap = shmximage_pixmap->data;
  }

  /* create the window */
  winattr.background_pixel = XBlackPixel(display, screen);
  winattr.border_pixel = XBlackPixel(display, screen);
  winattr.colormap = colormap;
  winattr.event_mask = 
    ExposureMask |
    VisibilityChangeMask |
    StructureNotifyMask |
    ResizeRedirectMask;
  winattr.override_redirect = True;

  window = 
    XCreateWindow(display,
		  root,
		  0, 0,
		  width, height,
		  0,              /* border_width */
		  depth,          /* depth */
		  InputOutput,    /* class */
		  visual,         /* visual */
		  (CWBackPixel | 
		   CWBorderPixel | 
		   CWEventMask | 
		   CWColormap |
		   CWOverrideRedirect), /* valuemask */
		  &winattr);

  /* create the graphic context */
  gcvalues.foreground = None;
  gcvalues.background = None;
  gcvalues.function = GXcopy;
  gcvalues.plane_mask = XAllPlanes();
  gcvalues.clip_mask = None;
  gcvalues.graphics_exposures = False; /* No NoExpose */
  gc = XCreateGC(display, window,
		 (GCBackground | 
		  GCForeground | 
		  GCFunction | 
		  GCPlaneMask | 
		  GCClipMask |
		  GCGraphicsExposures),
		 &gcvalues);

  /* create pixmaps */
  pixmap = XCreatePixmap(display,
			 window, 
			 width,
			 height,
			 depth);

  if (feature_shm_pixmap) {
    shmpixmap = XShmCreatePixmap(display,
				 window,
				 shminfo_pixmap.shmaddr,
				 &shminfo_pixmap,
				 width,
				 height,
				 depth);
  }

  if (feature_render) {
    /* create XRender Picture objects */
    pict_attr.component_alpha = False;
    
    pixmap_pict = XRenderCreatePicture(display, 
				       pixmap,
				       pict_format,
				       CPComponentAlpha,
				       &pict_attr);
    
    window_pict = XRenderCreatePicture(display, 
				       window,
				       pict_format,
				       CPComponentAlpha,
				       &pict_attr);
  
    if (feature_shm_pixmap) {
      shmpixmap_pict = XRenderCreatePicture(display, 
					    shmpixmap,
					    pict_format,
					    CPComponentAlpha,
					    &pict_attr);
    }
  }

  /* map the window and wait for it */
  XMapWindow(display, window);

  /* wait for map */
  for(;;) {
    XEvent e;

    /* check X event */
    XNextEvent(display, &e);
    
    printf("event %d\n", e.type);

    if (e.type == MapNotify) {
      break;
    }
  }

  /*
   * get high priority
   *
   */
  errno = 0;
  prio = getpriority(PRIO_PROCESS, pid);
  if (errno != 0) {
    fprintf(stderr, "Can't get priority, harmless: %s\n", strerror(errno));
    prio = 0;
  }
  
  prio -= 20;
 
  if (setpriority(PRIO_PROCESS, pid, prio) == -1) {
    fprintf(stderr, "Can't set priority, harmless: %s\n", strerror(errno));
  }

  /*
   * get very high priority
   *
   * any other than SCHED_OTHER could be bad.
   * in fact anything different/higher than the scheduler/priority
   * used by the X server will probably give bad result
   */
  
  memset(&param, 0, sizeof(struct sched_param));
  sched = sched_getscheduler(pid);
  if (sched == -1) {
    fprintf(stderr, "Can't get scheduler, harmless: %s\n", strerror(errno));
  }

  if (sched != SCHED_RR) {
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    if (sched_setscheduler(pid, SCHED_RR, &param) == -1) {
      fprintf(stderr, "Can't set scheduler, harmless: %s\n", strerror(errno));
    }
  } else {
    if (sched_getparam(pid, &param) == -1 || 
	param.sched_priority != sched_get_priority_max(SCHED_RR)) {
      param.sched_priority = sched_get_priority_max(SCHED_RR);
      if (sched_setparam(pid, &param) == -1) {
	fprintf(stderr, "Can't set scheduler parameters, harmless: %s\n",
		strerror(errno));
      }
    }
  }

  if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
    fprintf(stderr, "Can't lock memory, harmless: %s\n",
	    strerror(errno));
  }

  /**
   * TESTS
   *
   * test are enclosed between TEST_BEGIN()/TEST_END()
   */


  /**
   * Put the image directly in the window using XPutImage()
   * No double buffer
   */
  TEST_BEGIN("XPutImage(XImage)", 1)
    {
      TEST_UPDATE(data);

      /* put the image in the pixmap (stored on the server side) */
      XPutImage(display,
		window, 
		gc,
		ximage, 
		0, 0, 
		0, 0, 
		width, height);

      /* 
       * be sure the commands are sent before sending more command
       * since there's no call to XNextEvent(), Xlib won't send the request
       * until a call to XSync() or XFlush()
       *
       * For functions using SHM, an XSync() call is done at the entrance
       */  
      
      XFlush(display);
    }
  TEST_END();

  /**
   * double buffer, use XCopyArea()
   * test write to a double buffer pixmap
   */
  TEST_BEGIN("XPutImage(XImage): Pixmap, XCopyArea(Pixmap)", 1)
    {
      TEST_UPDATE(data);

      /* put the image in the pixmap (stored on the server side) */
      XPutImage(display,
		pixmap,
		gc,
		ximage, 
		0, 0, 
		0, 0, 
		width, height);

      /* copy the data to the window */
      XCopyArea(display, 
		pixmap, window, 
		gc, 
		0, 0, 
		width, height,
		0, 0);
      
      XFlush(display);
    }
  TEST_END();

  /**
   * Double buffer, use XRenderComposite() instead of XCopyArea()
   * test write to a double buffer pixmap, then XRender to the window
   */
  TEST_BEGIN("XPutImage(XImage): Pixmap, XRenderComposite(Pixmap)", 
	     feature_render)
    {
      TEST_UPDATE(data);

      /* put the image in the pixmap (stored on the server side) */
      XPutImage(display,
		pixmap,
		gc,
		ximage, 
		0, 0,
		0, 0,
		width, height);

      XRenderComposite(display,
		       PictOpSrc,
		       pixmap_pict, 
		       None,
		       window_pict, 
		       0, 0,
		       0, 0,
		       0, 0,
		       width, height);

      XFlush(display);
    }
  TEST_END();


  /**
   * Put the content to the window using XShmPutImage()
   * (no double buffer on some plateform, since
   *  the shared memory region could not be in the same memory kind
   *  main memory / video memory)
   * test write to a shared memory segment 
   * copied to the window
   *
   */
    
  TEST_BEGIN("XShmPutImage(XImage SHM)", feature_shm)
    {

      XSync(display, False); /* Synchronize before writing
				to the shared memory */
    
      TEST_UPDATE(shmdata_ximage);
    
      /* put the image in the pixmap (stored on the server side) */
      XShmPutImage(display,
		   window,
		   gc, 
		   shmximage_ximage, 
		   0, 0,
		   0, 0,
		   width, height,
		   False);
    }
  TEST_END();


  /**
   * Use shared memory to copy to the Pixmap
   * (double buffer)
   *
   * test write to a shared memory segment,
   * to be copied to a pixmap and then copied to the window
   */
  TEST_BEGIN("XShmPutImage(XImage SHM): Pixmap, XCopyArea(Pixmap)", 
	     feature_shm)
    {
    
      XSync(display, False); /* Synchronize before writing
				to the shared memory */

      TEST_UPDATE(shmdata_ximage);

      /* put the image in the pixmap (stored on the server side) */
      XShmPutImage(display,
		   pixmap,
		   gc,
		   shmximage_ximage, 
		   0, 0,
		   0, 0, 
		   width, height,
		   False);
  
      /* copy pixmap data to the window */
      XCopyArea(display, 
		pixmap, window, 
		gc, 
		0, 0, 
		width, height, 
		0, 0);
    }
  TEST_END();

  /**
   * Use the shared memory to write in a pixmap,
   * then use XRenderComposite() to put pixmap data on the window
   *
   * test write to a shared memory segment,
   * to be copied to a pixmap and then rendered to the window
   *
   */
  TEST_BEGIN("XShmPutImage(XImage SHM): Pixmap, XRenderComposite(Pixmap)",
	     feature_shm && feature_render)
    {
      XSync(display, False); /* Synchronize before writing
				to the shared memory */

      TEST_UPDATE(shmdata_ximage);

      /* put the image in the pixmap (stored on the server side) */
      XShmPutImage(display,
		   pixmap,
		   gc, 
		   shmximage_ximage, 
		   0, 0, 
		   0, 0, 
		   width, height,
		   False);

      /* copy the pixmap content using XRender */
      XRenderComposite(display,
		       PictOpSrc,
		       pixmap_pict, 
		       None,
		       window_pict, 
		       0, 0,
		       0, 0,
		       0, 0,
		       width, height);
    }
  TEST_END();

  /**
   * write directly to a pixmap using shared memory
   * then copy the pixmap content to the window
   *
   * test write to a shared memory segment 
   * copied the shared pixmap to the window
   *
   */
  TEST_BEGIN("XCopyArea(Pixmap SHM)",
	     feature_shm_pixmap)
    {
      XSync(display, False); /* Synchronize before writing
				to the shared memory */

      /* write the content of the pixmap data */
      TEST_UPDATE(shmdata_pixmap);

      /* copy the shared memory pixmap to the screen */
      XCopyArea(display,
		shmpixmap, window, 
		gc, 
		0, 0, 
		width, height, 
		0, 0);
    }
  TEST_END();

  /**
   * Write directly to the pixmap shared memory, 
   * Then use XRenderComposite to 
   *
   *
   * test write to a shared memory segment,
   * then XRender the shared pixmap to the window
   *
   */
  TEST_BEGIN("XRenderComposite(Pixmap SHM)", 
	     feature_shm_pixmap && feature_render)
    {
      XSync(display, False); /* Synchronize before writing to
				the shared memory */

      /* write the content of the pixmap data */
      TEST_UPDATE(shmdata_pixmap);
      
      XRenderComposite(display,
		       PictOpSrc,
		       shmpixmap_pict, 
		       None,
		       window_pict, 
		       0, 0,
		       0, 0,
		       0, 0,
		       width, height);
    }
  TEST_END();

  XCloseDisplay(display);

  return 0;
}
