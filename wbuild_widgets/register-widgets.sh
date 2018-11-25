#!/bin/bash

gen_header()
{
echo "/* generated by register-widgets.sh */"
echo "#ifndef ${OUT^^}_H"
echo "#define ${OUT^^}_H"
echo "#include <X11/IntrinsicP.h>"
echo "#include \"WcCreateP.h\""
for a in ${names[@]}
do
    echo "#include <xtcw/$a.h>"
    if ! [ -f "$INC/$a.h" ]
    then
        cp ../plainc_widgets/$a.h $INC/
        touch $INC/$a.h
    fi
done

echo "void VexmoRegister(XtAppContext app);"

echo "#endif"
}

gen_code()
{
echo "/* generated by wb/register-widgets.sh */"
echo "#include \"xtcw/${OUT}.h\""
echo "#include \"WcCreateP.h\""

echo "void XtcwRegister(XtAppContext app) {"
for a in ${names[@]}
do
    echo "WcRegisterClassPtr(app, \"$a\", ${a,}WidgetClass );"
done
echo "}"
}


src=(*.widget)
names=(${src[@]%%.widget})
#base=(${src[@]%%.widget})
#names=(${base[@]##src/})

OUT=register_wb
INC=$1
SRC=$2
gen_header >$INC/register_wb.h
gen_code   >$SRC/register_wb.c
