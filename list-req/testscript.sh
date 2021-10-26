#!/bin/bash

for((i=0;i<100;i++))
do
	echo "Line - $i  -"
done
echo "parameters: $*"
echo "success" 
echo >&2 stderr stream test
sleep 2

