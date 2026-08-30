#include "sensorreg.h"
#include "mls.h"
#include "conststr.h"

int SENSOR_LIST = 0;

static int sensor_cmp( const void *a0, const void *b0 )
{
	const struct sensor_reg *key    =  a0;
	const struct sensor_reg *member =  b0;
	return m_cmp( key->name, member->name );
}

void sensor_register_c(Widget w, const char *s)
{
	sensor_register(w,s_cstr(s));
}

void sensor_register(Widget w, int name)
{
	if( SENSOR_LIST == 0 ) SENSOR_LIST = m_create(10,sizeof(struct sensor_reg));

	struct sensor_reg r = {.name=name,.w = w  };
	m_binsert( SENSOR_LIST, &r, sensor_cmp, 0 );
}
