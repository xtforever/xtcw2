================================ README.BUILD ================================

	This version of the Free Widgets release has an Imakefile build
	structure.  Previous versions had Makefiles.  Since the build
	requirements of the FWF release differ significantly from the build
	structure of the X Window System, some of the structure of the Imake
	system is different from the standard X Imakefiles.  Furthermore, the
	structure will evolve in the future as people decide what build system
	will be most convenient.  There is still disagreement on this topic
	at the current time.  For more details about the imake structure, read
	the file README.IMAKE.

BUILDING THE FWF RELEASE:

	Normally, the FWF system is packaged as a compressed shell archive
	(shar) file, or tape archive (tar) file.  The typical release is
	a compressed shar file called fwf.shar.Z.

	(1) Uncompress The Compressed Shar File

		uncompress fwf.shar.Z

	(2) Unarchive The File

		sh fwf.shar

	    This should create a directory tree with a top level directory
	    FWF in the current directory.

	(3) Examine The Contents

	    The contents of FWF should contain several README files, a
	    src directory containing the fwf sources, several FWF files
	    intended to configure the build, and a few extra gifts.  The
	    system as of 6/1/1992 contains the following files in FWF:

		FWF.rules               README.FWF              README.TODO
		FWF.tmpl                README.IMAKE            VERSION
		Imakefile               README.INDEX            fwf.man
		README                  README.NOTES            src/
		README.BUILD            README.SUBMISSION	utils/

	(4) Setting The Compilation Options

	    Many people do not need to set any special compilation options,
	    but there are some options which can be set by editing the
	    FWF.tmpl file.

	    The most common change deals with people who do not have the
	    X Pixmap Library installed.  Several FWF widgets use the X
	    Pixmap Library.  Though the library & include file is shipped
	    with many versions of X, some vendors do not supply Xpm.  If
	    you do not have libXpm.a and the include file xpm.h, you will
	    with need to install it, or disable the use of Xpm (causing some
	    widgets to have limited functionality, and some to be omitted
	    entirely).

	    You can obtain XPM via anonymous FTP from export.lcs.mit.edu
	    in the contrib directory, or from avahi.inria.fr in the pub
	    directory.  You can disable Xpm usage by setting HaveXpm to 0 in
	    FWF.tmpl.  If you install Xpm in a non-standard location, you can
	    set XPM_LIBDIR and XPM_INCDIR in FWF.tmpl.

	    If your system doesn't have mkdirhier, you can use the FWF
	    supplied version, by uncommenting the MKDIRHIER line in FWF.tmpl.

	    If you don't want to build the executable test programs (we
	    strongly recommend that you *do* build these to be sure all
	    built correctly, and to see demos of the widgets in action), you
	    can set BuildExecs to 0 in FWF.tmpl.

	    Special compiler options, and choice of compiler can be specified
	    by redefining CC in FWF.tmpl.

	    Unusual installation options can be defined in FWF.tmpl as well.

	(5) Customize Which Widgets To Include In The Library

	    The FWF library is growing larger each release.  Some people
	    may wish to build a library consisting of only some of the FWF
	    widgets.  To do this, edit the file FWF/src/Imakefile and remove
	    the widgets you do not want to build from the SUBDIRS assignment.
	    To build the entire release, you do not need to do anything.

	    If you do decided to prune the library, there is one trouble
	    point.  Some widgets depend on other widgets.  So, if you remove
	    a widget that another widget needs, the build will fail.  For this
	    reason, widets to prune cannot be done completely haphazardly.

	(6) Build The Top Level Makefile

	    The top level Makefile has to be constructed from the top
	    level Imakefile.  By doing this, your system configuration
	    defaults will be incorporated into the Makefile.  This is
	    typically done using the program 'xmkmf' (X Make Makefile):

		xmkmf

	    This saves a backup copy of any old Makefile and runs 'imake'
	    on the Imakefile, specifying the directory to find configuration
	    defaults.  On my system, this executes:

		mv Makefile Makefile.bak
		imake -DUseInstalled -I/usr/local/X11R5/lib/X11/config

	    If you don't have xmkmf, you can directly use imake, but you
	    need to know where your configuration files reside.  If your
	    configuration files reside in /usr/lib/X11/config, you can type:

		imake -DUseInstalled -I/usr/lib/X11/config

	    If you want to specify a different compiler or compiler options,
	    edit the FWF.tmpl file first.

	(7) Build The Child Makefiles

	    Now we need to recursively build the Makefiles down the directory
	    hierarchy.  The standard X11 Imake configuration inserts entries
	    into each Makefile to allow the construction of the subdirectories'
	    Makefiles.  So, you just need to type:

		make Makefiles

	(8) Compile The Widgets

	    Now that the Makefiles are all initialized, we can compile the
	    build tree.  Directories will be created as needed, and include
	    files will be copied into the include directory in the process
	    of compilation.

	    Type:

		make

	    This creates the subdirectories to hold include files, binaries,
	    libraries, and man pages using the 'mkdirhier' scripts which
	    creates directories, and any non-existent parent directories.
	    If you do not have the mkdirhier script, a copy can be found in
	    the utils subdirectory.  You can uncomment the MKDIRHIER line
	    in FWF.tmpl to use this local script.

	    Hopefully all will build.  As time goes on and more and more
	    people use this, we will hopefully get a very reliable build
	    system.

	    If you have problems, re-read section (4) above to see if that
	    solves your problems.  Otherwise, send mail to the mailing list:

		free-widgets-bugs@let.rug.nl

	    to report the problem.

	(9) Where Is Everything?

	    Under FWF, you should have subdirectories bin, include, lib, man

	    These hold binary test programs, include files, libraries, and
	    man pages for the widgets.  These can be installed in standard
	    system locations as shown below.  You can play around in the bin
	    directory and try some of the test programs, though not all of
	    them will make sense if you don't understand what the widgets do.

	(10) Installing

	    The FWF release places all the components of the package (binaries,
	    includes, libraries) all in the FWF subdirectory.  If you want to
	    install these into your standard X11 system installation
	    locations, you can type:

		make install

	    Man pages can be installed with:

		make install.man

	(11) Cleaning Up

	    To erase the extra subdirectories, and restore yourself to a basic
	    source release, type:

		make clean

	(12) Packing The Code Up To Give To Some Else

	    The Imakefiles provide a mechanism to pack the release back into
	    a shar or tar file to give to someone else.  To do this, type:

		make shar
	    or
		make tar

	    The subdirectories will first be "squeaky" cleaned, removing old
	    files *and* Makefiles to save space.  Since the Makefiles will be
	    removed, you will later need to do a 'make Makefiles' again, if
	    you want to rebuild the widgets, or pack the code up again.

	    When done, the shar or tar file will be placed in the directory
	    'export', under the main FWF directory.  The file will include
	    the version number, which is extracted from the VERSION file.

	For more details about the Imake system, you may want to refer
	to the file README.IMAKE

PROBLEMS

	1) What are those .w files?

	The .w files are the input to a program called Wbuild. Wbuild
	generates C code from .w files, just like Yacc generates C
	from .y files and Lex from .l files.

	The .w files are included for people who want to modify the
	widgets or write new ones. If you only want to compile and use
	the widgets, you don't need Wbuild. If Make complaints that it
	can't find Wbuild, then there is something wrong with the time
	stamps of the .c files. The following should help:

	       find . -name '*.[ch]' -exec touch {} \;

	Or simply delete all .w files.

	Wbuild is available by Anonymouse FTP from ftp.let.rug.nl,
	directory pub/FWF.


	2) Is it possible to compile just a few widgets?

	Yes, but with a few restrictions. In principle, you can skip
	everything you don't want by editing the list of widgets in
	src/Imakefile.

	However, some widgets assume the presence of utility functions
	or of other widgets. The directories misc, scroll, tabstring,
	converters and Dir contain code that is used by many other
	widgets. You should probably not remove those. Widgets also
	need their superclasses. For example, Slider4 is a subclass of
	Slider2, Label, Board, Frame and Common. In order to compile
	Slider4 you'll have to compile all its superclasses first.

	3) What is "ssp"? Make says it can't find it.

	"ssp" is used in the Wbuild.rule file. If you're not using
	Wbuild, you shouldn't need ssp either. "ssp" (Single SPace) is
	a command that is available on System V derived Unixes to
	remove multiple empty lines. On BSD systems you can replace it
	with "cat -s". See Wbuild.rule in the src directory.
