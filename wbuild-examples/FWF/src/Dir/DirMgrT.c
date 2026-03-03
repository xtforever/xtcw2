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

#include <stdio.h>
#include <Xfwf/DirMgr.h>

static char* usage[] =
{
	"This is the DirMgr test program.  DirMgr is not a widget.",
	"DirMgr is a data structure to manage UNIX directories, and perform ",
	"operations on them.  This program demonstrates the data structure ",
	"briefly.  If you pass in a directory path as the first argument, ",
	"a numeric 'sort mode' (0 = no sorting) as the second argument, and ",
	"an optional shell wildcard pattern (quoted) as the third argument, ",
	"this program will display the directory, and then re-sort in mode 1.",
	NULL
};

int main(argc,argv)
int argc;
char **argv;
{
	DIRECTORY_MGR *dm;
	DIR_ENTRY *de;
	char *path,*pat,*s;
	int i,sort_mode;

	if (argc != 3 && argc != 4)
	{
		for (i = 0; usage[i] != NULL; i++)
		{
			fprintf(stderr,"%s\n",usage[i]);
		}
		fprintf(stderr,"\nusage: %s path sort_mode [pattern]\n",
			argv[0]);
		exit(-1);
	}
	path = argv[1];
	sort_mode = atoi(argv[2]);
	if (argc == 4)
		pat = argv[3];
	    else
		pat = NULL;
	dm = DirectoryMgrSimpleOpen(path,sort_mode,pat);
	if (dm == NULL)
	{
		fprintf(stderr,"Can't open directory mgr for '%s'\n",path);
		exit(-1);
	}
	while (1)
	{
		de = DirectoryMgrNextEntry(dm);
		if (de == NULL) break;
		DirEntryDump(stderr,de);
	}

	printf("Changing sort mode to 1\n");
	DirectoryMgrSimpleResort(dm,1);
	while (1)
	{
		de = DirectoryMgrNextEntry(dm);
		if (de == NULL) break;
		DirEntryDump(stderr,de);
	}


	DirectoryMgrClose(dm);
	return(1);
}
