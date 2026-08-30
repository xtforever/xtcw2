/*
    squish.c: utility procedures for the file completion widget
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

*/

/*
 * $Log: squish.c,v $
 * Revision 0.14  1993/03/11  10:53:19  thoth
 * squish_filespec doesn't return a value.
 *
 * Revision 0.13  92/05/20  17:13:30  thoth
 * this was released with FWF 3.0
 * 
 * Revision 0.12  92/05/06  11:42:12  thoth
 * added rcsid and log, fixed bugs
 * 
 */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _POSIX_PATH_MAX
#include <sys/param.h>
#define _POSIX_PATH_MAX MAXPATHLEN
#endif

#ifdef SVR4                                     /* BB, 27 Mar '96 */
#define bcopy(b1, b2, length) memcpy(b2, b1, length)
#define bzero(b, length) memset(b, 0, length)
#endif

typedef char	pathT[_POSIX_PATH_MAX+1];

extern char *getod();

static char *find_dotdot(s)
     /* returns a char* pointing to the '/' just after the /.. */
     char	*s;
{
  char	*p;
  for (p=s; p=strchr(p, '/'); p++)
    if (0==strncmp(p, "/../", 4))
      return p+3;
  return NULL;
}

static int	root_dev= -1;

void squish_filespec(spec, opts /*NYI*/)
     char	*spec;
     int	opts; /* expand symlinks to hard paths? */
{
  char	*s,*p;
  pathT	path1, path2;
  struct stat	st1, st2;

  if (root_dev==-1) {
    stat("/", &st2);
    root_dev = st2.st_dev;
  }

  /* perform emacs-style // and /~ string reset */

  for (s=spec + strlen(spec) - 1; s>spec; s--) {
     if ((s[0] == '/' || s[0] == '~') &&
	 s[-1] == '/')
       break;
  }

  if (s>spec) {
    for (p=spec; *s; p++,s++)
      *p = *s;
    *p = 0;
  }

  /* perform ~ expansion if necessary */

  if (spec[0]=='~') {
    if (spec[1]=='/') {
      char	*home = getenv("HOME");
      int	homelen;
      if (home==NULL) {
	home = "";
	homelen = 0;
      } else {
	homelen = strlen(home);
	if (home[homelen-1]=='/')
	  home[--homelen] = 0;
      }
      /* skip the ~ but include the \0 */
      bcopy(spec+1, spec+homelen, strlen(spec));
      bcopy(home, spec, homelen);
    } else {
      struct passwd *pw;
      char	user[64];
      for (s=spec+1,p=user; *s && *s != '/'; s++,p++)
	*p = *s;
      *p = 0;
      pw = getpwnam(user);
      if (pw) {
	int	len = strlen(pw->pw_dir);
	if (pw->pw_dir[len-1]=='/')
	  len--;
	bcopy(s, spec+len, strlen(s)+1);
	bcopy(pw->pw_dir, spec, len);
      }
    }
  }

  /* option processing */

  if (opts!=0) {
    p = strrchr(spec, '/');
    if (p!=NULL) {
      int	slen;

      p++;
      strncpy(path1, spec, p-spec);
      path1[p-spec] = 0;
      s = getod(path1);
      slen = strlen(s);
      if (s && ( opts!=1 || slen < strlen(path1) ) ) {
	strcpy(spec+slen, p); 
	strncpy(spec, s, slen);
	free(s);
	return;
      }
      if (s)
	free(s);
    }
  }

  /* compress "/./" to "/" */

  p = s = spec;

  while (*s) {
    if (0==strncmp(s,"/./",3))
      s+=2;
    *p++ = *s++;
  }
  *p = *s;

  /* squish out the leading ../../ sequences if appropriate */

  s = spec;
  p = path1;
  while (*s && *s=='.' || *s == '/')
    *p++ = *s++;
  *p = 0;

  if (!(path1[0]=='/' && path1[1]==0)
      && 0==stat(path1, &st1) && st1.st_ino==2) {
    if (st1.st_dev==root_dev) {
      p = spec;
      *p++ = '/';
      for (; *s; s++,p++)
	*p = *s;
      *p = 0;
    }
  }

  /* compress "/filename/../" to "/" when appropriate */

  s = spec;

  while ( s = find_dotdot(s) ) {

    strncpy(path1, spec, s-spec);
    strncpy(path2, spec, s-spec);
    path2[s-spec] = path1[s-spec] = 0;

    /* back up two path elements */
    p = (s-spec) - 3 + path2;
    *p=0;
    p = strrchr(path2, '/');
    if (!p || 0==strcmp(p, "/..")) {
      if (0==stat(path1, &st1) && st1.st_ino==2 && st1.st_dev==root_dev) {
	bcopy(spec + strlen(path1), spec, strlen(spec) + 1 - strlen(path1));
	squish_filespec(spec, opts);
	return;
      } else
	continue;
    }
    p[1] = 0;
    /*printf("%s && %s\n", path1, path2); */

    /* only bail on path2 error.  If we get a path one error we will
       assume the compression is legal */
    if (0>stat(path2, &st2))
      return;


    if (0!=stat(path1, &st1) ||
	st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino) {
      char	*p;
      int	offset;
      offset = (s - spec) + 1 - strlen(path2);
      s = spec+strlen(path2);
      for (p = s--; p[offset]; p++)
	p[0] = p[offset];
      *p = 0;
    } else if (opts!=0) {
      /* hmm, this .. does not go up.  we must have moved sideways. */
      char	*realpath;
      realpath = getod(path1);
      if (realpath) {
	int	len1, rplen;
	rplen = strlen(realpath)-1;
	len1 = strlen(path1);
	if (opts==2 ||
	    opts==1 && rplen <= len1) {
	  bcopy(spec + len1, spec + rplen,
		strlen(spec) - len1 + 1);
	  bcopy(realpath, spec, rplen);
	}
	free(realpath);
      }
    }
  }
  /*printf(": %s\n", spec);*/
}
