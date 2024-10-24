#include "sensorreg.h"
#include "conststr.h"
#include "mls.h"

int main()
{
	int p;
	struct sensor_reg *d;
	m_init();
	conststr_init();
	
	sensor_register_c( 0, "best" );
	sensor_register_c( 1, "cest" );
	sensor_register_c( 2, "aest" );	
	m_foreach( SENSOR_LIST, p, d ) {
		printf("%ld %s\n", (long)d->w, CHARP(d->name) );
	}
	m_destruct();
}
