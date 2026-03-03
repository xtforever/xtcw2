#include <stdio.h>
#include <Xfwf/Directory.h>

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

static char* usage[] =
{
	"This is the Directory test program.  Directory is not a widget.",
	"Directory is a low-level data structure to manage UNIX directories ",
	"portably (hopefully).  This program demonstrates the data structure ",
	"briefly, by listing the contents of a directory specified on the ",
	"command line.",
	NULL
};


FatalError(prog)
char *prog;
{
	int i;
	char *s;

	for (i = 0; usage[i] != NULL; i++)
	{
		fprintf(stderr,"%s\n",usage[i]);
	}
	fprintf(stderr,"\nusage: %s pathname\n",prog);
	exit(1);
}


int main(argc,argv)
int argc;
char **argv;
{
	DirEntry entry;
	Directory directory;

	if (argc != 2) FatalError(argv[0]);

	if (DirectoryOpen(argv[1],&directory) == FALSE)
	{
		fprintf(stderr,"Can't open directory '%s'\n",argv[1]);
		FatalError(argv[0]);
	}
	printf("*** Listing Of Directory '%s' ***\n",
	       DirectoryPath(&directory));
	while (DirectoryReadNextEntry(&directory,&entry))
	{
		printf("<Type %5d,  File '%20s',  Mode %3d,  Size %d>\n",
		       DirEntryType(&entry),
		       DirEntryFileName(&entry),
		       DirEntryProt(&entry),
		       DirEntryFileSize(&entry));
	}
	return(1);
}
