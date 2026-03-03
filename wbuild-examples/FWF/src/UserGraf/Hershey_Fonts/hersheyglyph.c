#include <stdio.h>
#include "hershey.h"
#include "cyrilc.h"
#include "gothgbt.h"
#include "gothgrt.h"
#include "gothitt.h"
#include "greekc.h"
#include "greekcs.h"
#include "greekp.h"
#include "greeks.h"
#include "italicc.h"
#include "italiccs.h"
#include "italict.h"
#include "math.h"
#include "misc.h"
#include "romanc.h"
#include "romancs.h"
#include "romand.h"
#include "romanp.h"
#include "romans.h"
#include "romant.h"
#include "scriptc.h"
#include "scripts.h"

extern HersheyGlyph OccidentalGlyphs[];
extern HersheyGlyph OrientalGlyphs[];

/* The scale of the characters.  Normal characters are digitized 22 */
/* units tall.  This scales them to be 1.0 unit tall. */
#define SCALE 1.0/22.0

/* Return the points needed to draw a hershey glyph.  If d[i] is 1, */
/* the a segment should be draw from p[i-1] to p[i].  If shift is */
/* non-zero then the origin o is shifted so the glyphs can be printed */
/* as text.  This returns the number of points placed in p[] and d[]. */
int Hershey_Glyph(g, org, p, d, shift)
     HersheyGlyph *g;
     HersheyPoint *org;
     HersheyPoint *p;
     int *d;
     int shift;
 {
  int i;			/* An index counter */
  int move;			/* Is this a move. */
  int npts;

  if (!g) return 0;

  /* If the character should be shifted then move it forward to allow */
  /* space for the previous character. */ 
  if (shift) org->x -= SCALE*g->back;

  /* Put the character in the the p array. */
  move = 1;
  npts = 0;
  for (i=0; i<g->nvtx; ++i) {
    if (g->vtx[i].x == -127) {
      move = 1;
      continue;
    }
    p->x = org->x + SCALE*g->vtx[i].x;
    p->y = org->y + SCALE*g->vtx[i].y;
    if (move) {
      *d = 0;
      move = 0;
    }
    else *d = 1;
    ++p; ++d; ++npts;
  }

  /* If the character should be shifted then move the origin forward */
  /* to allow space for the next character. */
  if (shift) org->x += SCALE*g->forw;

  return npts;
}

/* Plot a character c from the font f and the glyph set g.  The */
/* character placed at the origin o and o is shifted so the next */
/* character will be in the correct position. The p[] and d[] are */
/* defined as in Hershey_Glyph. */
int Hershey_Character(f, g, c, o, p, d)
     HersheyFont *f;
     HersheyGlyph *g;
     int c;
     HersheyPoint *o;
     HersheyPoint *p;
     int *d;
 {
  return Hershey_Glyph(&g[f->glyph[c]],o,p,d,1);
}

/* Plot a string of characters using font f and glyph set g.  The */
/* variables o, p[], and d[] are defined as in Hershey_Character. */
int Hershey_String(f, g, s, o, p, d)
     HersheyFont *f;
     HersheyGlyph *g;
     char *s;
     HersheyPoint *o;
     HersheyPoint *p;
     int *d;
 {
  int npts=0;
  for (; *s;++s) {
    int i = *s;
    i = Hershey_Character(f,g,i,o,p,d);
    p += i;
    d += i;
    npts += i;
  }
  return npts;
}

