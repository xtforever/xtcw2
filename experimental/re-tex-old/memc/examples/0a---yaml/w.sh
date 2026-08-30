tmp=$(mktemp)

dir=~/git/ansible2/group_vars
src=cluster2
#src="cluster2 cluster3 cluster5 cluster6 cluster7 cluster8 cluster9"

gen_all()
{
  JWT_PASSPHRASE=$(openssl rand -base64 32)
  APP_SECRET=$(openssl rand -base64 32)
  echo -e "phpvalue: |\n  env[JWT_PASSPHRASE]=\"$JWT_PASSPHRASE\"\n  env[APP_SECRET]=\"$APP_SECRET\"\n"
}


gen_appsec()
{
  APP_SECRET=$(openssl rand -base64 32)
  echo -e "env[APP_SECRET]=\"$APP_SECRET\"\n"
}

gen_jwt()
{
  JWT_PASSPHRASE=$(openssl rand -base64 32)
  echo -e "env[JWT_PASSPHRASE]=\"$JWT_PASSPHRASE\"\n"
}

app_missing()
{
   while true; do gen_all; done  | ./cedyaml.exed app-missing --file "$1" --path 'customers: * phpvalue:' --prefix 2 >$tmp
#   while true; do gen_appsec; done  | ./cedyaml.exed app-missing --file "$tmp" --path 'customers: * phpvalue: env[APP_SECRET]' --prefix 3 >$tmp.1
#   while true; do gen_jwt; done  | ./cedyaml.exed app-missing --file "$tmp.1" --path 'customers: * phpvalue: env[JWT_PASSPHRASE]' --prefix 3
#   rm $tmp
#   rm $tmp.1
}


update-old-keys()
{
f=$(mktemp)
cp $1 $f

while read kk key
do
    echo Searching: $kk, for $key
    cp $f $f.1
    ./cedyaml.exed replace --path="customers: $kk: phpvalue: env[JWT_PASSPHRASE]" --file="$f.1" \
		   --repl="env[JWT_PASSPHRASE]=\"$key\"" >$f
done < kunde_pass.txt
cp $f $1
rm $f $f.1
}

for a in $src ; do
  mv $dir/$a $dir/$a.old
  app_missing $dir/$a.old $dir/$a
#  update-old-keys $dir/$a
done
echo $tmp

