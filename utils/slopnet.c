#include "slopnet.h"
#include "slop4.h"
#include "mrb.h"
#include "communication.h"
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



static int SLN_SELECT_EXIT = 0;

static int SLN = 0;

int sln_lookup_fd(int fd)
{
    int p;
    struct sln_st *d;
    m_foreach(SLN,p,d)
	if( d->fd == fd ) return p;
    return -1;
}


struct sln_st *sln_get(int n)
{
    if(SLN < 1 || n >= m_len(SLN) || n < 0)
	ERR("");

    return mls(SLN,n);
}

int sln_get_fd(int n)
{
    return sln_get(n)->fd;
}


/** called by slop4::slop_put if a message is complete */
void sln_message_complete_cb(int error, int msg, void *ctx)
{
    int n = (intptr_t) ctx;
    struct sln_st *sln = sln_get(n);
    
    if( error ) {
	WARN("message has crc error");
	return;
    }
    int p;
    int *cur;
    m_foreach(msg,p,cur) {
	if( m_len(*cur) ) { 
	    if(sln->callback)
		sln->callback( error, *cur, n, sln->ctx );
	    else
		WARN("slopnet callback undefined");
	}
    }
}


int sln_init(void)
{
    struct sln_st *sln;
    int pos;
    
    
    if(SLN < 1 ) {
	SLN = m_create(1, sizeof(struct sln_st));
    }

    /* find empty element */
    m_foreach( SLN, pos, sln ) 
	if(! sln->init ) goto found;

    // not_found 
    pos = m_new(SLN,1);
    sln = sln_get(pos);

 found:
    TRACE(1,"create slop connection");
    slop_init(& sln->slop_inp );
    sln->slop_inp.ctx = (void*)(intptr_t) pos;
    sln->slop_inp.cb  = sln_message_complete_cb;
    
    sln->fd = -1;
    sln->buf_in = mrb_create(1024);
    sln->buf_out = mrb_create(1024);
    sln->buf = m_create(100,1);
    sln->init = 1;
    return pos;
}


/* close sln and close fd  */
void sln_free(int n)
{
    struct sln_st *sln = sln_get(n);
    if( sln->fd >= 0 ) {
	close(sln->fd);
	sln->fd = -1;
    }
    sln_free_open_fd(n);
}

/* close sln but do not close fd */
void sln_free_open_fd(int n)
{
    TRACE(1,"kill slop connection %d", n);
    struct sln_st *sln = sln_get(n);
    sln->init = 0;
    free(sln->buf_in); sln->buf_in=0;
    free(sln->buf_out); sln->buf_out=0;
    m_free(sln->buf);
    slop_free(& sln->slop_inp );
}

void sln_destruct(void)
{
    if( SLN < 1 ) return;
    
    struct sln_st *sln;
    int pos;
    m_foreach(SLN, pos, sln) {
	if( sln->init ) {
	    sln_free(pos);
	}
    }
    m_free(SLN);
    SLN=0;
}


void sln_callback(int n, sln_input_t cb, void *ctx )
{
    struct sln_st *sln = sln_get(n);
    sln->callback = cb;
    sln->ctx = ctx;
}


/* returns: 1 if not connected, 0 if connected */
int sln_not_connected(int n)
{
    struct sln_st *sln = sln_get(n);
    return sln->fd < 0;
}


/* returns fd for socket or -1 for error */
int sln_connect(int n, char *host, char *service,
		sln_input_t cb, void *ctx )
{
    struct sln_st *sln = sln_get(n);
    if( sln->fd >= 0 ) close(sln->fd);
    
    sln->callback = cb;
    sln->ctx = ctx;
    sln->init = 2;
    TRACE(1,"slop try connect to server");
    return sln->fd = sock_connect_service(host,service);
}

/* returns fd for socket */
int sln_accept(int n, int fd, sln_input_t cb, void *ctx )
{
    struct sln_st *sln = sln_get(n);
    sln->callback = cb;
    sln->ctx = ctx;
    sln->init = 2;
    return sln->fd = fd;
}


static void slop_parse( struct sln_st *sln )
{
    int ch;
    while( (ch=mrb_get(sln->buf_in)) != -1 ) {
	slop_put( &sln->slop_inp, ch );
    }
}


/* close connection */
void sln_close_fd( int n ) 
{
    struct sln_st *sln = sln_get(n);
    if( sln->fd >= 0 ) {
	WARN("slopnet closing %d", sln->fd );
	close(sln->fd);
	sln->fd=-1;
    }
    sln->init = 1;
}



/* called if a message has arrived */
/* returns -1 if connection is closed */
int sln_input_cb(int n)
{
    struct sln_st *sln = sln_get(n);

    int err = mrb_sock_read(sln->buf_in, sln->fd);
    
    if( err ) {
	sln_close_fd(n);
	return -1;    
    }
    if( read > 0 ) slop_parse(sln);
    return 0;
}

/* versuche maximal 2sekunden die daten zu übertragen */
int sock_write(int fd, int msg)
{
    void *buf = m_buf(msg);
    int cnt = m_len(msg);
    int written;
    int timer = 2000;
    
    while( cnt > 0 ) {

	written = write( fd, buf, cnt );
	if( written < 0 ) {
	    if(! timer-- ) return -1; 

	    if( (errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK) )
		usleep(1000);
	    else
		return -1;

	}

	buf += written;
	cnt -= written;	
    }
    return 0;
}


int sln_vaprintf(int n, char *format, va_list ap)
{
    struct sln_st *sln = sln_get(n);
    int m = vas_printf(0,0, format, ap );
    m_clear(sln->buf);
    slop_encode_msg(sln->buf,m_buf(m), m_len(m)-1, 1 );
    m_free(m);
    if( sock_write( sln->fd, sln->buf ) ) {
	sln_close_fd(n);
	return -1;
    }
    return 0;
}

int sln_printf(int n, char *format, ... )
{
    va_list ap;
    va_start(ap,format);
    int e = sln_vaprintf(n, format,ap);
    va_end(ap);
    return e;
}


static int find_sln(int fd)
{
    int p = sln_lookup_fd(fd);
    if( p<0) ERR("unknown fd");
    return p;
}


static void add_new_client(int fd, sln_input_t cb, void *ctx)
{
    int sln = sln_init();
    sln_accept( sln, fd, cb, ctx );
    
    TRACE(1,"new client %d", fd );
}

static int process_incomming_data(int fd)
{
    
    int sln = find_sln(fd);
    int e= sln_input_cb(sln) ;
    if( e ) {
	sln_free(sln);
	TRACE(1,"client %d removed", fd);
    }
    return e;
}

void sln_set_exit_flag(int exit_flag)
{
    SLN_SELECT_EXIT = exit_flag != 0 ;
}

void sln_server_select(int listener, sln_input_t cb, void *ctx)
{
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int newfd;     /* newly accept()ed socket descriptor */

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener +1; /* so far, it's this one*/
     
    for(;  !  SLN_SELECT_EXIT ; )
    {
	tmp_rd_fds = master;	
	if(select(fdmax, &tmp_rd_fds, NULL, NULL, NULL) == -1) {
	    if( errno == EINTR || errno == EAGAIN ) continue; 
	    ERR("select()");
	}

	/* find available file descriptor */
	for(int fd = 0; fd < fdmax; fd++) {
	    if( ! FD_ISSET(fd, &tmp_rd_fds) ) continue;

	    if( fd != listener ) {
		if( process_incomming_data(fd) ) {
		    TRACE(1,"socket %d hung up\n", fd);
		    FD_CLR(fd, &master);	
		}	    
		continue;
	    }
	    
	    /* incomming connection request */
	    newfd = sock_accept_incomming_connection(listener);
	    if( newfd < 0 ) {
		WARN("accept error");
	    } else {
		TRACE(1,"accept new conn");
		FD_SET(newfd, &master); /* add to master set */
		fdmax = MAX( fdmax, newfd +1 );
		add_new_client(newfd, cb, ctx);
	    }
	}
    }
    SLN_SELECT_EXIT=0;
}


int sln_server_loop(char *port,  sln_input_t cb, void *ctx )
{
    int sfd = sock_listen_on_port(port);
    if( sfd < 0 ) ERR("could not bind to %s", port );
    sln_server_select(sfd, cb, ctx); /* never returns */    
    return -1;
}




void sln_client_select(int n)
{
    struct sln_st *sln = sln_get(n);    
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    /* add the listener to the master set */
    FD_SET(sln->fd, &master);

    /* keep track of the biggest file descriptor */
    fdmax = sln->fd +1; /* so far, it's this one*/
     
    for(;  !  SLN_SELECT_EXIT ; )
    {
	tmp_rd_fds = master;	
	if(select(fdmax, &tmp_rd_fds, NULL, NULL, NULL) == -1) {
	    if( errno == EINTR || errno == EAGAIN ) continue; 
	    ERR("select()");
	}

	if( ! FD_ISSET(sln->fd, &tmp_rd_fds) ) continue;	

	sln_input_cb(n);
	
    }
    SLN_SELECT_EXIT=0;
}



/** wait for input on given slopnet connection, if input arrived
    read input, parse input and execute commands

   returns:
   - 0 : if timeout occured
   - 1 : if new messsage has arrived
   - 2 : comm error
*/

int sln_select_timeout(int sln, int timeout_ms)
{
    int listener = sln_get_fd(sln);
    struct timeval tv;
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int ret;
    
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener +1; /* so far, it's this one*/

    for( ; ; )
    {
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = timeout_ms * 1000; /* 1000 * 1000 */
	tmp_rd_fds = master;
	ret = select(fdmax, &tmp_rd_fds, NULL, NULL, &tv);
	if(ret == -1 ) {
	    
	    if( errno == EINTR || errno == EAGAIN ) continue; 
	    ERR("select()");
         }

         if( ret == 0 ) { 
            return 0; /* timeout */
         }
	 int e = sln_input_cb(sln) ; /* read parse and execute */
	 if( e ) {
	     sln_close_fd( sln );
	     WARN("comm error at sln:%d", sln);
	     return 2;
	 }
	
	 return 1;
    }
}
