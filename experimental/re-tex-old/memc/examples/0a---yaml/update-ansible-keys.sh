#!/bin/bash



fn=$1
if ! [ -r "$fn" ] ; then
  echo >&2 "error: $fn not found"
  exit 1
fi


f=$(mktemp)
cp $fn $f

while read kk key
do
    echo Searching: $kk, for $key
    cp $f $f.1
    ./cedyaml.exed replace --path="customers: $kk: phpvalue: env[JWT_PASSPHRASE]" --file="$f.1" \
		   --repl="env[JWT_PASSPHRASE]=\"$key\"" >$f
done < kunde_pass.txt
cp $f $fn
rm $f
