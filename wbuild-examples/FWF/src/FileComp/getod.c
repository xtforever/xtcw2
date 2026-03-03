/*
    getod.c: library routine to normalize a path.
    Copyright (C) 1992 Robert Forsman

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The next comment contains the original copyright from Berkeley's
    getcwd.c source.  It is now overridden by the GNU Library General
    Public License.

 */

/*
 * Copyright (c) 1989, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static char rcsid[] = "$Id: getod.c,v 0.15 1992/06/30 10:59:01 thoth Exp $ \nModified by Robert Forsman from\n@(#)getcwd.c	5.11 (Berkeley) 2/24/91";

/*
 * $Log: getod.c,v $
 * Revision 0.15  1992/06/30  10:59:01  thoth
 * some OSs don't have the d_namlen struct member.
 *
 * Revision 0.14  92/06/08  22:16:46  thoth
 * portability improvement, used d_ino instead of d_fileno.
 * 
 * Revision 0.13  92/05/06  12:48:14  thoth
 * I inserted the GNU Library General Public License to override the
 * Berkeley copyright notice which is still included in the code (as
 * required by that copyright, and as is consistent with historical
 * honesty).
 * 
 * Revision 0.12  92/05/06  11:40:48  thoth
 * added rcsid and log
 * 
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#ifdef SVR4                                     /* BB, 27 Mar '96 */
#define bcopy(b1, b2, length) memcpy(b2, b1, length)
#define bzero(b, length) memset(b, 0, length)
#endif

#define	ISDOT(dp) \
	(dp->d_name[0] == '.' && (dp->d_name[1] == '\0' || \
	    dp->d_name[1] == '.' && dp->d_name[2] == '\0'))

char *
getod(path)
	char *path;
{
  register struct dirent *dp;
  register DIR *dir;
  register dev_t dev;
  register ino_t ino;
  register int first;
  register char *bpt, *bup;
  struct stat s;
  dev_t root_dev;
  ino_t root_ino;
  size_t ptsize, upsize;
  int save_errno;
  char *pt, *ept, *eup, *up;
  
  /*
   * If no buffer specified by the user, allocate one as necessary.
   * If a buffer is specified, the size has to be non-zero.  The path
   * is built from the end of the buffer backwards.
   */
  
  if (!(pt = (char *)malloc(ptsize = 1024 - 4)))
    return((char *)NULL);
  ept = pt + ptsize;
  
  bpt = ept - 1;
  *bpt = '\0';
/*  *--bpt = '/';  */
  
  /*
   * Allocate bytes (1024 - malloc space) for the string of "../"'s.
   * Should always be enough (it's 340 levels).  If it's not, allocate
   * as necessary.  Special * case the first stat, it's ".", not "..".
   */
  if (!(up = (char *)malloc(upsize = 1024 - 4)))
    goto err;
  eup = up + MAXPATHLEN;
  strcpy(up, path);
  bup = up + strlen(up);
  if (bup[-1] != '/') {
    *bup++ = '/';
    *bup = '\0';
  }
  
  /* Save root values, so know when to stop. */
  if (stat("/", &s))
    goto err;
  root_dev = s.st_dev;
  root_ino = s.st_ino;
  
  errno = 0;			/* XXX readdir has no error return. */
  
  for (first = 1;; first = 0) {
    /* Stat the current level. */
    if (lstat(up, &s))
      goto err;
    
    /* Save current node values. */
    ino = s.st_ino;
    dev = s.st_dev;
    
    /* Check for reaching root. */
    if (root_dev == dev && root_ino == ino) {
      *--bpt = '/';
      /*
       * It's unclear that it's a requirement to copy the
       * path to the beginning of the buffer, but it's always
       * been that way and stuff would probably break.
       */
      (void)bcopy(bpt, pt, ept - bpt);
      free(up);
      return(pt);
    }
    
    /*
     * Build pointer to the parent directory, allocating memory
     * as necessary.  Max length is 3 for "../", the largest
     * possible component name, plus a trailing NULL.
     */
    if (bup + 3  + MAXNAMLEN + 1 >= eup) {
      if (!(up = (char *)realloc(up, upsize *= 2)))
	goto err;
      eup = up + upsize;
    }
    *bup++ = '.';
    *bup++ = '.';
    *bup = '\0';
    
    /* Open and stat parent directory. */
    if (!(dir = opendir(up)) || fstat(dir->dd_fd, &s))
      goto err;
    
    /* Add trailing slash for next directory. */
    *bup++ = '/';
    
    /*
     * If it's a mount point, have to stat each element because
     * the inode number in the directory is for the entry in the
     * parent directory, not the inode number of the mounted file.
     */
    save_errno = 0;
    if (s.st_dev == dev) {
      for (;;) {
	if (!(dp = readdir(dir)))
	  goto notfound;
	if (dp->d_ino == ino)
	  break;
      }
    } else
      for (;;) {
	if (!(dp = readdir(dir)))
	  goto notfound;
	if (ISDOT(dp))
	  continue;
	bcopy(dp->d_name, bup, strlen(dp->d_name) + 1);
	
	/* Save the first error for later. */
	if (lstat(up, &s)) {
	  if (!save_errno)
	    save_errno = errno;
	  errno = 0;
	  continue;
	}
	if (s.st_dev == dev && s.st_ino == ino)
	  break;
      }
    
    /*
     * Check for length of the current name, preceding slash,
     * leading slash.
     */
    if (bpt - pt <= strlen(dp->d_name) + (first ? 1 : 2)) {
      size_t len, off;
      
      if (!ptsize) {
	errno = ERANGE;
	goto err;
      }
      off = bpt - pt;
      len = ept - bpt;
      if (!(pt = (char *)realloc(pt, ptsize *= 2)))
	goto err;
      bpt = pt + off;
      ept = pt + ptsize;
      (void)bcopy(bpt, ept - len, len);
      bpt = ept - len;
    }
/*    if (!first)*/
      *--bpt = '/';
    bpt -= strlen(dp->d_name);
    bcopy(dp->d_name, bpt, strlen(dp->d_name));
    (void)closedir(dir);
    
    /* Truncate any file name. */
    *bup = '\0';
  }
  
 notfound:
  /*
   * If readdir set errno, use it, not any saved error; otherwise,
   * didn't find the current directory in its parent directory, set
   * errno to ENOENT.
   */
  if (!errno)
    errno = save_errno ? save_errno : ENOENT;
  /* FALLTHROUGH */
 err:
  if (ptsize)
    free(pt);
  free(up);
  return((char *)NULL);
}
