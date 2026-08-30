#include <stdio.h>
#include <Xfwf/RegExp.h>

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
	"This is the RegExp test program.  RegExp is not a widget.",
	"RegExp is a C front end to the regexp regular expression routines.",
	"These routines make regular expressions more convenient.  If you ",
	"do not have the regular expression routines, then these routines ",
	"become stubs which do nothing.  The code is in the Dir directory ",
	"along with the Directory and DirMgr directory-management data ",
	"structures.",
	"",
	"This test program takes a shell wild card pattern and a string as ",
	"command-line arguments.  The pattern is converted to a regular ",
	"expression, matched against the string, and the result of the match ",
	"is displayed.",
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
	fprintf(stderr,"\nusage: %s shellwildcardpattern string\n",prog);
	exit(1);
}


int main(argc,argv)
int argc;
char **argv;
{
	char fsm[1024],regexp[1024];
	char *string,*re_comp();
	int status;

	if (argc != 3) FatalError(argv[0]);

	RegExpPatternToRegExp(argv[1],regexp);
	string = argv[2];
	RegExpCompile(regexp,fsm,1024);
	status = RegExpMatch(string,fsm);
	printf("String '%s' %s pattern '%s' (regexp '%s')\n",string,
	       (status == TRUE ? "matches" : "doesn't match"),
	       argv[1],regexp);

	return(1);
}
