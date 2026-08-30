#!/bin/bash
	
gen_sec()
{
  JWT_PASSPHRASE=$(openssl rand -base64 32)
  APP_SECRET=$(openssl rand -base64 32)
  echo -e "env[JWT_PASSPHRASE]=\"$JWT_PASSPHRASE\"\n"
}

while true; do gen_sec; done  | ./cedyaml.exed --app-missing --file "$1" --path 'customers: * phpvalue: env[JWT_PASSPHRASE]' --prefix 3


