#include <stdio.h>
#include "hershey.h"

/*
 *  read_char: read in the next character, but eat any newline characters.
 */
int read_char() {
  int i;
  while ((i = getchar()) == '\n');
  return i;
}  

/*
 * scanint: a function to scan an integer, using n characters of
 *          the input file, ignoring newlines. (scanf won't work
 *          because it also ignores blanks)
 */
int scanint (n)
int n;
{
  char            buf[20];
  int             i, c;
  
  for (i = 0; i < n; i++) {
    c = read_char();
    if (c == EOF) return (-1);
    buf[i] = c;
  }
  buf[i] = 0;
  return (atoi (buf));
}

/*
 * Fill the hershey character structure. 
 */
HersheyGlyph *read_glyph(index)
int *index;
{
  int i, j;
  HersheyGlyph *g = (HersheyGlyph *) malloc(sizeof(HersheyGlyph));
  HersheyVertex *v; 

  *index = scanint(5);
  if (*index<1) {
    free(g);
    return 0;
  }
  g->nvtx = scanint(3) - 1;
  g->back = read_char() - 'R';
  g->forw = read_char() - 'R';
  v = g->vtx = (HersheyVertex *) malloc(g->nvtx * sizeof(HersheyVertex));
  for (i=0; i<g->nvtx; ++i) {
    j = read_char();
    /* Next point is a move */
    if (j == 32) {
      v->x = -127;
      v->y = -127;
      j = read_char();		/* throw away the next character. */
    }
    else {
      v->x = j - 'R';
      v->y = 'R' - read_char();
    }
    ++v;
  }

  return g;
}

#define MAX_GLYPHS 8000
HersheyGlyph **read_hershey(max_glyph)
int *max_glyph; {
  HersheyGlyph *glyphs[MAX_GLYPHS];
  HersheyGlyph *g;
  HersheyGlyph **gg;
  int i;
  int index;

  *max_glyph = 0;

  for (i=0; i<MAX_GLYPHS; ++i) glyphs[i] = (HersheyGlyph *)0;

  /* First read in all of the glyphs */
  while ((g = read_glyph(&index)) != 0) {
    glyphs[index] = g;
    if (*max_glyph < index+1) *max_glyph = index+1;
  }

  /* Now transfer them to a permanent memory block */
  gg = (HersheyGlyph **)malloc((*max_glyph) * sizeof(HersheyGlyph *));
  for (i=0; i<*max_glyph; ++i) gg[i] = glyphs[i];

  return gg;
}

void write_vertex(out, name, gg, max_glyph) 
FILE *out;
char *name;
HersheyGlyph **gg;
int max_glyph;
{
  HersheyVertex *v;
  HersheyGlyph *g;
  int i, j;

  for (i=0; i<max_glyph; ++i) {
    g = gg[i];
    if (g) {
      if (g->nvtx>0) {
	fprintf(out,"static HersheyVertex %s%d[%d] = {\n",name,i,g->nvtx);
	v = g->vtx;
	for (j=0; j<g->nvtx; ++j) {
	  if (j) {
	    fprintf(out,",");
	    if (!(j%8)) fprintf(out,"\n");
	  }
	  fprintf(out,"{%d,%d}",v->x,v->y);
	  ++v;
	}
	fprintf(out,"};\n");
      }
    }
  }
}

void write_hershey(out,name,gg,max_glyph) 
FILE *out;
char *name;
HersheyGlyph **gg;
int max_glyph;
{
  int i;
  HersheyGlyph *g;

  fprintf(out,"#include \"Hershey_Fonts/hershey.h\"\n");
  
  write_vertex(out,name,gg,max_glyph);
  
  fprintf(out,"HersheyGlyph %sGlyphs[%d] = {",name,max_glyph);
  for (i=0; i<max_glyph; ++i) {
    g = gg[i];
    if (i) fprintf(out,","); 
    if ((i%4)==0) fprintf(out,"\n");
    if (!g) {
      fprintf(out,"{%d,%d,%d,%d}",0,0,0,0);
    } else if (g->nvtx) {
      fprintf(out,"{%s%d,%d,%d,%d}",name,i, 
	     g->nvtx, g->back, g->forw);
    }
    else {
      fprintf(out,"{%d,%d,%d,%d}",0,g->nvtx,g->back,g->forw);
    }
  }
  fprintf(out,"};\n");
}

extern char* optarg;
extern int optind, opterr;

int main (argc,argv)
int argc;
char **argv;
{
  int i;
  int max_glyph;
  HersheyGlyph **g;
  FILE *out = stdout;
  char name[32];
  
  strcpy(name,"Hersey");
  
  while ((i=getopt(argc,argv,"N:"))!=EOF) {
    switch (i) {
    case 'N':
      strcpy(name,optarg);
    }
  }

  if (argc>optind) out = fopen(argv[optind],"w");
  
  g = read_hershey(&max_glyph);

  write_hershey(out,name,g,max_glyph); 

  return 0;
}

