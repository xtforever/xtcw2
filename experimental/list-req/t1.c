#include "mls.h"
#include "moreutil.h"

int main()
{
    m_init(); trace_level=1;
    int lines=m_create(1,sizeof(int));
    run_script_save_output( "./testscript.sh", lines );
    int p,*d;
    m_foreach(lines,p,d) {
	printf("-- %s --\n", m_str(*d) );
	m_free(*d);
    }

    m_free( lines );
    m_destruct();
}
