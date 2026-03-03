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

#ifdef P
# undef P
#endif
#ifdef NeedFunctionPrototypes
#define P(X) X
#else
#define P(X) ()
#endif

/* Return the points needed to draw a hershey glyph.  If d[i] is 1, */
/* the a segment should be draw from p[i-1] to p[i].  If shift is */
/* non-zero then the origin o is shifted so the glyphs can be printed */
/* as text.  This returns the number of points placed in p[] and d[]. */
int Hershey_Glyph P((HersheyGlyph *g, HersheyPoint *o, 
		  HersheyPoint *p, int *d, int shift));

/* Plot a character c from the font f and the glyph set g.  The */
/* character placed at the origin o and o is shifted so the next */
/* character will be in the correct position. The p[] and d[] are */
/* defined as in Hershey_Glyph. */
int Hershey_Character P((HersheyFont *f, HersheyGlyph g[], int c,
			 HersheyPoint *o, HersheyPoint *p, int *d));

/* Plot a string of characters using font f and glyph set g.  The */
/* variables o, p[], and d[] are defined as in Hershey_Character. */
int Hershey_String P((HersheyFont *f, HersheyGlyph g[], char *s,
		      HersheyPoint *o, HersheyPoint *p, int *d));
#endif
