#!/bin/bash

# Compile the demo
echo "Compiling wlabel_demo..."
gcc -g -DMLS_DEBUG -O0 \
    -Ibuild/include \
    -Ibuild/include/xtcw \
    -Iutils \
    $(pkg-config --cflags freetype2 cairo) \
    -o wlabel_demo wlabel_demo.c \
    build/source/wcreg2.o \
    build/lib/libxtcw.a \
    -lX11 -lXt -lcairo -lXft -lfontconfig -lm -lXmu -lXext -lXaw -lXrender -lXpm -ldl -lreadline

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the demo with the local resource file
echo "Running wlabel_demo..."
export XENVIRONMENT=./WlabelDemo.ad

# Check if we should use xvfb-run (passed as first argument)
if [ "$1" == "--xvfb" ]; then
    timeout 5s xvfb-run ./wlabel_demo -geometry 300x500
else
    ./wlabel_demo -geometry 300x500
fi
