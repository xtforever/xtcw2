#!/bin/bash
git status | tee sta

echo "hello, do u  want ice cream ( Y/n/a ) ?"
echo >&2 this-is-an-error-msg
read a
if [ "$a" = "y" ]
then
    echo here is your ice cream
    exit 0
fi

echo "why ? this is strange"
exit 1

