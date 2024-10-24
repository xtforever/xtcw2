#!/bin/bash
MAKE=./bash-make
LOGF=make.log
TARGET=luarunner
LUA=../../lua
debug_enable=${debug_enable:-1}
prefix=../build
exec_prefix=$prefix
bindir=$exec_prefix/bin
libdir=$exec_prefix/lib
incdir=$prefix/include
srcdir=$prefix/source
docdir=$prefix/doc
libexecdir=$exec_prefix/libexec
PKGDATADIR=$prefix/etc/xtcw

YFLAGS=-d
YACC=bison
LEX=flex
CC=gcc
CFLAGS="$CFLAGS -D_GNU_SOURCE -fms-extensions -I$LUA"
CFLAGS="$CFLAGS -I$incdir -I$incdir/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw"

XORGLIB="-lXt -lXaw -lX11 -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil  -lmysqlclient -ldl"
LOADLIBES="$libdir/libxtcw.a $LUA/liblua.a $XORGLIB"


#LOADLIBES+=~/git/libxt-1.1.5/src/.libs/libXt.a -lICE -lSM

# where to find wbuild and wb.sh
wb_install=$PKGDATADIR
WBUILD=$bindir/wbuild
WBWRAP=$bindir/wb.sh
WBPREFIX=xtcw
WFLAGS="-d . -c . -i ./$WBPREFIX --include-prefix=$WBPREFIX $PKGDATADIR/tex.w $PKGDATADIR/nroff.w"

ccompile()
{
    inf=${IN[0]}
    outf=${OUT[0]}
    echo $CC -c $CFLAGS  $inf -o$outf
    $CC -c $CFLAGS  $inf -o$outf
}

#%_wrap.c: %.i
icompile()
{
    echo swig -lua -o ${OUT[0]} ${IN[0]}
    swig -lua -o ${OUT[0]} ${IN[0]}
}

#xtcw/%.h %.c: %.widget
widgetcompile()
{
    echo $WBWRAP ${IN[0]} 
	wb_search=$srcdir $WBWRAP ${IN[0]} $WBUILD $WFLAGS
}

#.tab.c %.tab.h: %.y
ycompile()
{
    echo     $YACC $YFLAGS ${IN[0]}   
    $YACC $YFLAGS ${IN[0]}   
}

#%.lex.c %.lex.h: %.l
lcompile()
{
    echo
    n=${IN[0]%.l}
    echo $LEX $n.l
    $LEX -o${n}.lex.c --header-file=${n}.lex.h $n.l
}

link-all()
{
    echo gcc  -o ${OUT[0]} ${IN[*]} $LOADLIBES
    gcc  -o ${OUT[0]} ${IN[*]} $LOADLIBES
}

cleanup()
{
    ${RM} -rf $TARGET luaxt_wrap.c widreg.c widreg.h *.o *.do src doc xtcw *~ *.3 *.tex script.fifo *.tab.* *.lex.*
}


DEPS=(m_tool.o opts.o svar2.o widreg.o
njson_read.o njson_lex.lex.o njson_parse.tab.o var5.o m_mysql.o
luaxt_wrap.o luaxt.o lua-var5.o canvas-draw-cb.o msg_box.o luarunner.o )

# create the dependency files for the widgets
export wb_search
LOCALW=(*widget)
for a in ${LOCALW[*]}
do
    b=${a%%.widget}
    superclass=$(source $WBWRAP $a echo)
    echo >$a.d "xtcw/${b}P.h xtcw/${b}.h ${b}.c: $superclass"
    DEPS+=("${b}.o")
done


if [[ $debug_enable ==  1 ]]
then 
    COMPTAG="DEBUG_$(date '+%Y_%m_%d')"
    CFLAGS+=" -g -Wall -DMLS_DEBUG -O0 -DCOMPTAG=\"$COMPTAG\"  "
    OBJ="do"
    for((i=0;i<${#DEPS[*]};i++))
    do
	DEPS[$i]="${DEPS[$i]%.*}.$OBJ"
    done    
else
    COMPTAG="PROD_$(date '+%Y_%m_%d' )"
    CFLAGS+=" -O3 -DCOMPTAG=\"$COMPTAG\""
    OBJ="o"
fi



widreg()
{
	echo widreg.sh ${IN[*]}
  	./widreg.sh ${LOCALW[*]}
}

read -r -d '' global_state <<- EOM 
(
import: "*.d",
gen-each: "run ccompile: .c .$OBJ ${DEPS[*]}",
gen-node: "run link-all:luarunner:${DEPS[*]}",
gen-node: "run widreg:widreg.c widreg.h:${LOCALW[*]}",
nodes: (

     ( REC: "run icompile",
       OUT: ( "luaxt_wrap.c" ),
       IN: ( "luaxt.i" ),
       DEP: ( "luaxt.c" )
     ),

      ( REC: "run lcompile",
       OUT: ( "njson_lex.lex.c" ),
       IN: ( "njson_lex.l" )
      ),

      ( REC: "run ycompile",
       OUT: ( "njson_parse.tab.c" ),
       IN: ( "njson_parse.y" ),
       DEP: ( "njson_lex.lex.c" )
      )

 ) )
EOM


echo $global_state >init-gs




store-result()
{
    ec+=( $1 $2 )
}

run()
{
    eval $@
}

#set -x
ec=()
loop_count=0
nr_jobs=1
while (( loop_count < 10 && nr_jobs > 0 )) 
do
    ((loop_count++))
    echo "LOOP: $loop_count"
    echo "${ec[*]} + $global_state" >last_state
    
    prog=$( echo "${ec[*]} + $global_state" | ${MAKE} 2>>log  | tee -a $LOGF )
    ec=();
    eval "$prog"
done


