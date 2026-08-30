#!/bin/bash
	
gen_sec()
{
  JWT_PASSPHRASE=$(openssl rand -base64 32)
  APP_SECRET=$(openssl rand -base64 32)
  echo -e "env[APP_SECRET]=\"$APP_SECRET\"\n"
}

while true; do gen_sec; done  | ./cedyaml.exed --app-missing --file "$1" --path 'customers: * phpvalue: env[APP_SECRET]' --prefix 3


