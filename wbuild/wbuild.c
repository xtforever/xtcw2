/*   wbuild
     Copyright (C) 1996  Joel N. Weber II <nemo@koa.iolani.honolulu.hi.us>

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>


#include <stdarg.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <wbuild.h>
#include <wsym.h>

#include <limits.h>
#include <libintl.h>
#define _(String) gettext(String)


#include <malloc.h>

FILE *xfopen (char *filename, char *mode)
{

FILE *f;

//assert(filename && mode);
f = fopen(filename, mode);
if (f == NULL)
  {
    fprintf(stderr,"%s %s\n",strerror(errno),filename);
    exit(1);
  }

return f;

}



extern int yyparse();

/* The global variables are used to hold the program's command line
 * arguments (file names), which will be read one by one by the scanner.
 */

extern char *optarg;
extern int optind;
static char **arguments;
static int nrarguments;
extern char *filename;
extern FILE *yyin;
extern int lineno;
int nerrors = 0;
extern time_t filetime;

void get_filetime(char *filename, time_t *result)
{
	struct stat s;
	int status;

	*result = (time_t) 0;
	status = stat(filename, &s);
	if (status == 0)
		*result = s.st_mtime;
}

int yywrap()
{
	fclose(yyin);
	lineno = 1;
	if (optind < nrarguments) {
		if (arguments[optind][0] == '-' && arguments[optind][1] == '\0') {
			filename = "<stdin>";
			filetime = 0;
			yyin = stdin;
			optind++;
			return 0;
		} else if (! (yyin = fopen(arguments[optind], "r"))) {
			perror(arguments[optind]);
			return 1;
		} else {
			filename = arguments[optind];
			get_filetime(filename, &filetime);
			optind++;
			return 0;
		}
	} else {
		return 1;
	}
}

char *build_filename(char *dir, char *name, char *ext)
{
	char *tmp=NULL;
	asprintf(&tmp,"%s/%s%s",dir,name,ext);
	return tmp;

}

char *build_guard(char *prefix, char *filename, char *ending)
{
	char *tmp=NULL, *s;
	asprintf(&tmp,"%s/%s%s",prefix,filename,ending);
	s = tmp;
	while (*s) {
		if (ISALPHA(*s)) {
			*s = toupper(*s);
		} else if (!isdigit(*s)) {
			*s = '_';
		}
		s++;
	}
	return tmp;
}

void process_file(Class c, char *include_dir, char *c_dir, char *doc_dir,
	char *prefix, int lines, int force)
{
	char *name, *filename, *guard;
	time_t desttime;
	Doctype doctype;
	FILE *f;
        void generate_doc2(FILE *f, Doctype dt, Class c);

	if (c->filenamepart)
		name = get(c->filenamepart);
	else
		name = get(c->name);

	if (!c->nocode) {
		filename = build_filename(include_dir, name, ".h");
		get_filetime(filename, &desttime);
		if (force || (desttime < c->filetime)) {
			guard = build_guard(prefix, name, "_H");
			f = xfopen(filename, "w");
			public_header(f, c, prefix, guard);
			fclose(f);
			free(guard);
		}
		free(filename);

		filename = build_filename(include_dir, name, "P.h");
		get_filetime(filename, &desttime);
		if (force || (desttime < c->filetime)) {
			guard = build_guard(prefix, name, "P_H");
			f = xfopen(filename, "w");
			generate_private_header(f, c, prefix, guard);
			fclose(f);
			free(guard);
		}
		free(filename);

		filename = build_filename(c_dir, name, ".c");
		get_filetime(filename, &desttime);
		if (force || (desttime < c->filetime)) {
			f = xfopen(filename, "w");
			generatec(f, c, prefix, lines);
			fclose(f);
		}
		free(filename);
	}

	if (c->nodoc)
		return;

	for (doctype = doctypes; doctype; doctype = doctype->next) {
		if (doc_dir)
			filename = build_filename(doc_dir, name,
				doctype->tag[t_filename][1]);
		else
			filename = build_filename(
				doctype->tag[t_filename][0],
				name, doctype->tag[t_filename][1]);
		get_filetime(filename, &desttime);
		if (force || (desttime < c->filetime)) {
			f = xfopen(filename, "w");
			generate_doc(f,doctype, c);
			fclose(f);
		}
		free(filename);
	}

}

struct option const longopts[] = {
	{"version",		no_argument,		0,	'v'},
	{"help",		no_argument,		0,	'h'},
	{"no-lines",		no_argument,		0,	'l'},
	{"include-prefix",	required_argument,	0,	'p'},
	{"only",		required_argument,	0,	'O'},
	{"include-dir",		required_argument,	0,	'i'},
	{"c-dir",		required_argument,	0,	'c'},
	{"doc-dir",		required_argument,	0,	'd'},
	{"documentation-dir",	required_argument,	0,	'd'},
	{"force",		no_argument,		0,	258},
	{"no-init-file",	no_argument,		0,	257},
	{0,			0,			0,	0}
};

/* The main program invokes the parser on all command line arguments
 * and then calls the generator to create all the C and \TeX\ files
 * (unless the parser reported errors).
 *
 * The intrinsic classes are read from a configuration file, which must
 * be present.
 */

static void usage()
{

  fprintf(stderr, _("wbuild usage:\n"
		    "wbuild [options] input-files\n"
		    "-v, --version\t\tPrint version and exit\n"
		    "-h, --help\t\tPrint this message and exit\n"
		    "-l, --no-lines\t\tDon't generate #line directives\n"
		    "-p, --include-prefix\tSpecify prefix for generated #include directives\n"
		    "-O, --only\t\tSet class to output code for\n"
		    "-i, --include-dir\tdirectory for generated include files\n"
		    "-c, --c-dir\t\tdirectory for generated c files\n"
		    "-d, --doc-dir, --documentation-dir\n"
		    "\t\t\tdirectory for generated documentation files\n"
		    "    --force\t\tForce regeneration of all files, ignoring timestamps\n"
		    "    --no-init-file\tDo not read initialization file\n"
		    "\n"
		    ));
  exit (0);
}

static void showversion()
{
	fprintf (stderr, "wbuild - FWF %s %s\n", PACKAGE, VERSION);
	fprintf (stderr, _("\
Copyright (C) %s Joel N. Weber II\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1996");
	fprintf (stderr, _("Written by %s and %s\n"),
		"Joel N. Weber II <devnull@gnu.ai.mit.edu>",
		"Bert Bos");

  exit (0);
}
static void _mkdir(const char *dir) {
        char tmp[PATH_MAX];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}
int main(int argc, char *argv[])
{
	char *include_dir = "../include/xtcw";
	char *c_dir = "../lib";
	char *doc_dir = 0;
	char *prefix = "xtcw";
    	char *only = 0;
	int lines = 1;
	int help = 0, version = 0;
	int force = 0;
	int init_file = 1;
	int c;
	Class class;

	symbol_init();

	arguments = argv;
	nrarguments = argc;

	while ((c = getopt_long(argc, argv, "lhvp:O:i:c:d:", longopts,
			/*& optind */ 0)) != -1)
		switch (c) {
			case 'l': lines = 0; break;
			case 'h': help = 1; break;
			case 'v': version = 1; break;
			case 'p': prefix = optarg; break;
			case 'O': only = optarg; break;
			case 'i': include_dir = optarg; break;
			case 'c': c_dir = optarg; break;
			case 'd': doc_dir = optarg; break;
			case 257: init_file = 0; break;
			case 258: force = 1; break;
			case '?': help = 1; break;
		}
	if (help) {
	  usage();
	} else if (version) {
	  showversion();

	}

        _mkdir( include_dir );
        _mkdir( c_dir );
        _mkdir( doc_dir );

	if (init_file) {
		yyin = fopen(PKGDATADIR "/init.w", "r");
		if (!yyin) {
			perror(PKGDATADIR "/init.w");
			return 1;
		}
	} else {
		yyin = fopen("/dev/null", "r");
		if (!yyin) {
			perror("/dev/null");
			return 1;
		}
		yywrap();
	}
	lineno = 1;
	yyparse();
	if (nerrors)
		return 1;
	nerrors = set_hierarchy();
	if (nerrors)
		return 1;
	for (class = classes; class; class = class->next)
		if (!only || (!strcmp(only, get(class->name))))
			process_file(class, include_dir, c_dir, doc_dir,
				prefix, lines, force);
	if (nerrors)
		return 1;
	return 0;
}

/* The |yywrap| function is called by the scanner when the end of a
 * file is reached. The function sets up a new file and returns zero. If
 * all files have been read, the function returns 1.
 */


/* The |err| function is used by the parser to display error messages.
 */

void err(int fatal, char *format,...)
{
	va_list ap;
	va_start(ap, format);
	nerrors++;
	(void) fprintf(stderr, "%s:%d: ", filename, lineno);
	(void) vfprintf(stderr, format, ap);
	if (fatal) exit(1);
	va_end(ap);
}
