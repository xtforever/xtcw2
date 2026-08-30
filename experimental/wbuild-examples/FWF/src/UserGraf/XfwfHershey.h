/*

    Copyright (C) 1994 Clark McGrew

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

#ifndef HERSHEY_H_SEEN
#define HERSHEY_H_SEEN

typedef struct hpoint HersheyPoint;
typedef struct hfont HersheyFont;
typedef struct hchar HersheyGlyph;
typedef struct hvertex HersheyVertex;

struct hpoint {
  float x, y;
};

struct hvertex {
  char x, y;
};  

struct hchar {
  HersheyVertex *vtx;		/* Pointer to the vertex Information. */
  unsigned short nvtx;		/* Number of vertices. */
  char back, forw;		/* The forward and backward extent. */
};

struct hfont {
  char *name;
  int *glyph;
  int nchar;
  int baseline;
  int capline;
};

#ifndef PROTO
#ifdef NeedFunctionPrototypes
#define PROTO(X) X
#else
#define PROTO(X) ()
#endif
#endif
/* Return the points needed to draw a hershey glyph.  If d[i] is 1, */
/* the a segment should be draw from p[i-1] to p[i].  If shift is */
/* non-zero then the origin o is shifted so the glyphs can be printed */
/* as text.  This returns the number of points placed in p[] and d[]. */
int XfwfHershey_Glyph PROTO((HersheyGlyph *g, HersheyPoint *o, HersheyPoint *p, int *d, int shift));

/* Plot a character c from the font f and the glyph set g.  The */
/* character placed at the origin o and o is shifted so the next */
/* character will be in the correct position. The p[] and d[] are */
/* defined as in Hershey_Glyph. */
int XfwfHershey_Character PROTO((HersheyFont *f, HersheyGlyph g[], int c, HersheyPoint *o, HersheyPoint *p, int *d));

/* Plot a string of characters using font f and glyph set g.  The */
/* variables o, p[], and d[] are defined as in Hershey_Character. */
int XfwfHershey_String PROTO((HersheyFont *f, HersheyGlyph g[], char *s, HersheyPoint *o, HersheyPoint *p, int *d));
#endif
