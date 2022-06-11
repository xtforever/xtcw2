#!/bin/bash
git status | tee sta

check_item_list()
{
echo item list:
echo 1. line1
echo 1. line2
echo 1. line3
echo 1. line4
echo 1. line5
echo 10. line6
echo Auswahl ?
read ln
echo You selected line: $ln
}


check_item_list


echo "hello, do u  want ice cream ( Y/n/a ) ?"
echo >&2 this-is-an-error-msg
read a
if [ "$a" = "y" ]
then
    echo here is your ice cream
    	echo exit script
	exit 0
fi

echo "why ? this is strange"
exit 1

