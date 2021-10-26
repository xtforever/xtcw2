#include "moreutil.h"
#include "mls.h"

#include "subshell.h"
struct fork2_info *SPROC;


int run_script_save_output( char *s, int ret_scriptoutput)
{
    SPROC = fork2_open ( s, NULL );
    if(! SPROC ) {
	return -1;
    }




    fd_set rfds, rfds2;
    struct timeval tv;
    int retval;
    
    /* Watch fd to see when it has input. */
    int max,fd ;
    FD_ZERO(&rfds);
    max=0;
    fd = SPROC->fd[ CHILD_STDOUT_RD ];
    max = Max(max,fd);
    FD_SET(fd, &rfds);
    fd = SPROC->fd[ CHILD_STDERR_RD ];
    max = Max(max,fd);
    FD_SET(fd, &rfds);
    max++;
	
    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    rfds2 = rfds;

    int err = 0;
    while( ! err ) {
	rfds = rfds2;
	retval = select(max, &rfds, NULL, NULL, &tv);
	if (retval < 0 ) {
	    TRACE(1, "Warning: read error on fd:%d", fd );
	    perror("");
	    return -1;
	}

	if( retval == 0 ) {
	    TRACE(1, "Warning: no data available on fd:%d",fd );
	    return 0;
	}

	if( FD_ISSET( SPROC->fd[ CHILD_STDOUT_RD ], &rfds ))
	    {
		err |= fork2_read(SPROC, 0);
	    }

	if( FD_ISSET( SPROC->fd[ CHILD_STDERR_RD ], &rfds ))
	    {
		err |= fork2_read(SPROC, 1);
	    }

	
	/* poll lines from input buffer */
	int line = m_create(100,1);
	while( ! fork2_getline(SPROC, 0, line)  ) {
	    m_put(  ret_scriptoutput, &line);
	    // printf("-- %s --\n", m_str(line) );
	    line = m_create(100,1);
	}
	while( ! fork2_getline(SPROC, 1, line)  ) {
	    fprintf(stderr,"E: %s\n", m_str(line) );
	}
	m_free(line);

    }
}

