#!/bin/bash

RSVG="--background-color=black --dpi=96 --format=png --keep-aspect-ratio"

for a in ./SVG/*.svg
do
        o=${a%%.svg}.xpm
        o=${o##*/}
        out=./XPM/$o
        rsvg-convert $a | pngtopnm | ppmtoxpm | sed -e s/black\"/None\"/g -e s/#000000\"/None\"/g -e s/Black\"/None\"/g >$out
done

