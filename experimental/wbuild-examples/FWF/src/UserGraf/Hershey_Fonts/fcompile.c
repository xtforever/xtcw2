#include <stdio.h>
#include <string.h>
#include "hershey.h"

/* Variables used by getopt to parse the command line.  Why the H**L */
/* aren't these in an include file someplace. */
extern char *optarg;
extern int optind, opterr;

/* The base and cap line positions for tiny, small and normal fonts. */
/* This data is constant, but some old compilers choke on const static */
/* initializations.  */ 
static int baseline[3] = {-5, -6, -9};
static int capline[3] = {4, 7, 12};

main(argc, argv) 
int argc;
char **argv;
{
  int i;
  FILE *in;			/* The input file */
  FILE *out = stdout;		/* The output file */
  char string[256];		/* A string to read the hmp. */
  HersheyFont font;		/* The hmp file is parsed into this font. */
  char fontname[256];		/* The font name being read. */
  int char_size = 2;		/* The size tiny=0, small=1, normal=2 */
  int MAX_CHAR = 12000;		/* The maximum characters per font. */
  int errflg = 0;		/* Flag a parse error. */
  font.nchar = 32;		/* Most fonts start with ASCII 32. */
  fontname[0] = 0;		/* No font name yet. */

  while ((i=getopt(argc,argv,"tsnN:b:c:f:o:h:")) != EOF) {
    switch (i) {
    case 'o':			/* Open an output file. */
      out = fopen(optarg,"w");
      break;
    case 'f':			/* Set the starting character number. */
      font.nchar = atoi(optarg);
      break;
    case 'N':
      strcpy(fontname,optarg);
      break;
    case 't':			/* Tiny Character Size. */
      char_size = 0;
      break;
    case 's':			/* Small Character Size. */
      char_size = 1;
      break;
    case 'n':			/* Normal Character Size. */
      char_size = 2;
      break;
    case 'b':			/* Custom Base Line. */
      baseline[char_size] = atoi(optarg);
      break;
    case 'c':			/* Custom Cap Line. */
      capline[char_size] = atoi(optarg);
      break;
    case '?':			/* Error. */
      ++errflg;
    default:
      break;
    }
  }

  /* Check the number of arguments. */
  if (argc != optind+1) ++errflg;

  /* Check if an error has occurred.  If it has print an error message. */
  if (errflg) {
    fprintf(stderr,
	    "usage: %s -tsn [-o <hfile>][-f <first>] hmp_file\n",
	    argv[0]);
    exit(1);
  }
  
  /* Open the input file from the command line. */
  in = fopen(argv[optind],"r");

  /* Get the font name from the file name. */
  if (strlen(fontname)<1) {
    char *c;
    if (c=strrchr(argv[optind],'/')) 
      strcpy(fontname,(char *) c+1);
    else 
      strcpy(fontname,argv[optind]);
    if (c=strrchr(fontname,'.')) 
      *c = 0;
  }

  /* Allocate and initialize space for the font. */
  font.glyph = (int *) malloc(MAX_CHAR * sizeof(int));
  for (i=0; i<MAX_CHAR; ++i) font.glyph[i] = 0;

  /* Set the font base line and cap line. */
  font.baseline = baseline[char_size];
  font.capline = capline[char_size];
  
  /***********************/
  /* Read in the .hmp file.  This is the only interesting part of this */
  /* program.  Below is an example of an .hmp file.  It contains glyph */
  /* indices of the characters that make up a font.  Typically fonts */
  /* define characters ' ' through '~', but may contain any number of */
  /* characters.   There may be a better way to read this file, but I */
  /* this method from hertovplot.c written J. Dillenger. */
  /* romanc.hmp:
     2199    2214    2213    2275    2274    2271    2272    2251
     2221    2222    2219    2232    2211    2231    2210    2220
     2200-2209
     2212    2213    2241    2238    2242    2215    2273
     2001-2026
     2223    804     2224    2262    999     2252
     2101-2126
     2225    2229    2226    2246    2218
     */
  while (fscanf(in,"%s",string) != EOF) {
    char *j = strrchr(string,'-');
    if (j) 
      for (i = atoi(string); i<=atoi((char *) j+1); ++i) 
	font.glyph[font.nchar++] = i;
    else 
      font.glyph[font.nchar++] = atoi(string);
  }
  /************************/

  /* Write the font data to the output file. */
  fprintf(out,"#include \"hershey.h\"\n");
  fprintf(out,"static int %s_glyphs[%d] = {",fontname,font.nchar);
  for (i=0; i<font.nchar; ++i) {
    if (i) fprintf(out,",");
    if (!(i%15)) fprintf(out,"\n");
    fprintf(out,"%d",font.glyph[i]);
  }
  fprintf(out,"};\n");
  fprintf(out,"HersheyFont %s = {\"%s\", %s_glyphs, %d, %d, %d};\n",fontname,
	  fontname,fontname,font.nchar,font.baseline,font.capline);
  exit(0);
}
