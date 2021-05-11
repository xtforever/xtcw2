#define _GNU_SOURCE
#include <stdio.h>

#include "nbus.h"
#include "ctx.h"
#include "mrb.h"
#include "communication.h"
#include "mls.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>


typedef enum nbus_type
    {
	NBUS_SERVER = 0,
	NBUS_CLIENT = 1
    } nbus_type;


struct nbus_ctx {
    int used;
    char *name;
    
    nbus_type state;
    int msg;
    int msg_from_id;
    int clients;
    int srv_fd;    
    int status_code;
    int id;
    
};

typedef enum nbus_socket {
    NBUS_CLOSED,
    NBUS_OPEN
} nbus_socket;


struct nbus_client {
    int used;
    nbus_socket state;
    int fd;
    int msg;
    int msg_ready;
    int error;
    char *error_msg;
    struct mrb *q;
};

static int NBUS = 0;    




static struct nbus_client *i_get_client(struct nbus_ctx *nbus, int client_id)
{
    return mls(nbus->clients, client_id);    
}

static int i_new_client(struct nbus_ctx *nbus)
{
    return ctx_init(&nbus->clients,10,sizeof(struct nbus_client));
}


static int i_add_new_client(struct nbus_ctx *nbus, int fd)
{
    int id = i_new_client(nbus);
    struct nbus_client *nc = i_get_client(nbus,id);
    nc->fd = fd;
    nc->q = mrb_create(512);    
    nc->msg = m_create(512,1);
    nbus->msg_from_id = id;
    nc->state = NBUS_OPEN;
    return id;
}


static void client_open_connection(struct nbus_client *nc, int fd)
{
    nc->fd = fd;
    nc->state = NBUS_OPEN;
}

static void client_close_connection(struct nbus_client *nc)
{
    if( nc->state == NBUS_OPEN ) {
	close(nc->fd);
	nc->fd=-1;
	nc->state = NBUS_CLOSED;
    }
}



static void remove_client(struct nbus_ctx *nbus, int client_id)
{
    struct nbus_client *nc = i_get_client(nbus,client_id);

    client_close_connection(nc);
    free(nc->q);
    m_free(nc->msg);
    nc->used=0;
}



static struct nbus_ctx *i_get_nbus(int bus)
{
    return mls(NBUS,bus);
}

static int i_nbus_create(void)
{
    return ctx_init(&NBUS, 10, sizeof(struct nbus_ctx));
}

static  struct nbus_ctx  *i_nbus_create_init(char *s)
{
    int bus_id = i_nbus_create();
    struct nbus_ctx *nbus = i_get_nbus(bus_id);
    nbus->name = strdup(s);
    nbus->id = bus_id;
    return nbus;
}

int nbus_create( char *s )
{
    struct nbus_ctx  *nbus = i_nbus_create_init(s);
    nbus->srv_fd = sock_listen_on_port( s );
    return nbus->id;
}

int nbus_connect( char *s )
{
    struct nbus_ctx  *nbus = i_nbus_create_init(s);
    nbus->state=NBUS_CLIENT;
    int fd = sock_connect_service("localhost", s );
    i_add_new_client(nbus,fd);
    return nbus->id;
}

int nbus_get_socket(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    return nbus->srv_fd;
}

static int extract_msg(struct mrb *q, int msg)
{
    int ch;
    while( (ch=mrb_get(q)) != -1 ) {
	if( ch == '\n' ) { m_putc(msg,0); return 1; }
	m_putc( msg, ch );
    }
    return 0;
}

static int i_client_find(struct nbus_ctx *nbus, int fd)
{
    int p; struct nbus_client *d;
    m_foreach(nbus->clients, p, d) {
	if(d->used && d->fd == fd ) return p;
    }
    ERR("client for fd=%d not found", fd );
    return -1;
}

/* deliver next message if buffer is empty 
   returns: 1 - message is ready, 0 no message found 
*/
static int fetch_message_from_buffer( struct nbus_client *nc )
{
    if(! nc->msg_ready ) {
	nc->msg_ready = extract_msg( nc->q, nc->msg );
    }
    return nc->msg_ready;
}


/* returns: -1 read error, 1 msg READY, 0 - waiting */
static int msg_from_client(struct nbus_ctx *nbus, int fd)
{
    /* read data for client fd */
    int client_id = i_client_find(nbus,fd);
    struct nbus_client *nc = i_get_client(nbus,client_id);		
    int e = mrb_sock_read(nc->q, nc->fd);
    if( e ) {
	WARN("comm error at fd:%d", nc->fd );
	remove_client(nbus,client_id);
	nc->error_msg = "comm error or broken link";
	return -1;
    }

    /* if there is NO message waiting to be delivered,
       extract new message from queue */
    return fetch_message_from_buffer(nc);
}


static int get_client_id(int bus, int fd)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    return i_client_find(nbus,fd);
}



static int nbus_client_msg_ready(struct nbus_ctx *nbus, int fd)
{
    int client_id = i_client_find(nbus,fd);
    if( client_id < 0 ) return 0;
    struct nbus_client *d = i_get_client(nbus, client_id);
    return d->msg_ready;
}


/* returns: -1 read error, 1 - mesg ready, 0 - waiting */
/*
static int msg_read( int fd, struct mrb *q, int msg )
{
    int error = mrb_sock_read( nc->q, fd );
    if( error ) return -1;
}
*/





static int i_server_select(struct nbus_ctx *nbus, int timeout_ms)
{
    struct timeval tv;
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int newfd;     /* newly accept()ed socket descriptor */

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    int listener = nbus->srv_fd;
    
    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener +1; /* so far, it's this one*/

    /* add clients */
    int p; struct nbus_client *d;
    if( nbus->clients ) {
	m_foreach(nbus->clients, p, d) {
	    if( d->used == 0 || d->state != NBUS_OPEN ) continue;	    
	    if( fdmax <= d->fd ) fdmax = d->fd+1;
	    FD_SET(d->fd, &master);
	}
    }
    
    for(; ; )
    {
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = timeout_ms * 1000; /* 1000 * 1000 */
	tmp_rd_fds = master;
	int sel = select(fdmax, &tmp_rd_fds, NULL, NULL, &tv); 
	if( sel == -1) {
	    if( errno == EINTR || errno == EAGAIN ) return 0;
	    ERR("select()");
	}
	if( sel == 0 ) return 0; /* timeout */

	/* find available file descriptor */
	for(int fd = 0; fd < fdmax; fd++) {
	    if( ! FD_ISSET(fd, &tmp_rd_fds) ) continue;

	    /* message from client fd */
	    if( fd != listener ) {
		nbus->msg_from_id = i_client_find(nbus,fd);
		int e = msg_from_client(nbus,fd);
		if( e == 1 ) return nbus->status_code = CLIENT_REQ;
		if( e == 0 ) continue;		
		if( e < 0 ) {
		    TRACE(1,"socket %d hung up\n", fd);
		    FD_CLR(fd, &master);	
		    return nbus->status_code = CLIENT_EXIT;		    
		}
		continue;
	    }

	    /* fd == listener */
	    /* incomming connection request */
	    newfd = sock_accept_incomming_connection(listener);
	    if( newfd < 0 ) {
		WARN("accept error");
	    } else {
		TRACE(1,"accept new conn");
		FD_SET(newfd, &master); /* add to master set */
		fdmax = Max( fdmax, newfd +1 );
		i_add_new_client(nbus,newfd);
		return nbus->status_code = NEW_CLIENT;
	    }
	}
    }
}





/** wait for input on given nbus client connection, if input arrived
    read input, parse input and execute commands

   returns:
   - 0 : if timeout occured
   - 1 : if new messsage has arrived
   - 2 : comm error
*/
static int select_timeout(struct nbus_ctx *nbus, int timeout_ms)
{
    struct timeval tv;
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int ret;
    
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    struct nbus_client *nc = i_get_client(nbus,0);		
    int listener = nc->fd;
    
    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener +1; /* so far, it's this one*/

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = timeout_ms * 1000; /* 1000 * 1000 */
    tmp_rd_fds = master;

    nbus->status_code = 0;
    ret = select(fdmax, &tmp_rd_fds, NULL, NULL, &tv);
    if(ret == -1 ) {	    
	if( errno == EINTR || errno == EAGAIN ) return 0;
	ERR("select()");
    }

    if( ret == 0 ) { 
	return 0; /* timeout */
    }

    int e = msg_from_client(nbus,listener);
    if( e == 0 ) return 0;
    nbus->msg_from_id = 0;
    if( e == 1 ) return nbus->status_code = NBUS_MESSAGE;
    if( e < 0 ) {
	TRACE(1,"socket %d hung up\n", listener);
	return nbus->status_code = NBUS_ERROR;
    }
	 
    return 0;
}

static int nbus_read_timeout(int bus, int timeout_ms)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    if( nbus->state == NBUS_CLIENT ) {
	return select_timeout(nbus, timeout_ms);
    }
    
    return i_server_select(nbus,timeout_ms);    
}

int nbus_poll_event(int bus)
{
    return nbus_read_timeout(bus,300);
}


/* fetch message from client and remove it from queue */
static char *poll_msg(struct nbus_client *nc)
{
    char *s = m_buf(nc->msg);
    m_clear(nc->msg);
    nc->msg_ready=0;
    return s;
}

char* nbus_msg(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    int client_id = nbus->msg_from_id;
    struct nbus_client *nc = i_get_client(nbus,client_id);
    return poll_msg(nc);    
}

int nbus_msg_from_id(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    return nbus->msg_from_id;
}

int nbus_client_fd(int bus, int client_id)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    struct nbus_client *nc = i_get_client(nbus,client_id);
    return nc->fd;
}

int nbus_msg_sender_fd(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    int client_id = nbus->msg_from_id;
    return nbus_client_fd( bus, client_id );
}

char* nbus_error_msg(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    int client_id = nbus->msg_from_id;
    struct nbus_client *nc = i_get_client(nbus,client_id);
    return nc->error_msg;
}


int nbus_puts_dest(int bus, int id, char *s )
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    struct nbus_client *nc = i_get_client(nbus,id);
    int fd = nc->fd;
    
    if( nc->state != NBUS_OPEN ) {
	WARN("try to send over not connected socket");
	return 1;
    }
    ASSERT(fd>=0);
    if( send(fd , s, strlen(s), MSG_NOSIGNAL ) < 0 ) {
	client_close_connection(nc);
	return 1;
    }

    return 0;
}

int nbus_printf_dest(int bus, int dest, char *frm, ... )
{
    char *s;
    va_list ap;
    va_start(ap,frm);
    vasprintf( &s, frm, ap );
    va_end(ap);
    int e = nbus_puts_dest(bus, dest, s );
    free(s);
    return e;
}


int nbus_printf( int bus, char *frm, ... )
{
    char *s;
    va_list ap;
    va_start(ap,frm);
    vasprintf( &s, frm, ap );
    va_end(ap);
    int e = nbus_puts_dest(bus, 0, s );
    free(s);
    return e;
}

/* return ptr to client message */
char* nbus_client_get_msg(int bus, int client_id)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    struct nbus_client *nc = i_get_client(nbus,client_id);
    if( nc->msg_ready == 0 ) return "";
    return poll_msg(nc);
}

/* connection request is waiting on the server port,
   accept connection and return client_id
*/
int nbus_server_message(int bus)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    int fd = nbus->srv_fd;
    int newfd = sock_accept_incomming_connection(fd);
    if( newfd < 0 ) {
	WARN("accept error");
	return -1;
    }

    TRACE(1,"accept new conn");
    int id = i_add_new_client(nbus,newfd);
    nbus->status_code = NEW_CLIENT;
    return id;
}


/* data is waiting on client socket,
   fetch data and set current status

   returns: client status
   - NBUS_WAITING - messge was incomplete
   - CLIENT_EXIT  - client connection lost
   - CLIENT_REQ   - message ready
*/
int nbus_client_message(int bus, int client_id)
{
    struct nbus_ctx *nbus = i_get_nbus(bus);
    struct nbus_client *nc = i_get_client(nbus,client_id);
    int fd = nc->fd;
    nbus->msg_from_id = client_id;
    int e = msg_from_client(nbus,fd);
    if( e == 1 ) return nbus->status_code = CLIENT_REQ;
    if( e < 0 ) {
	TRACE(1,"socket %d hung up\n", fd);
	return nbus->status_code = CLIENT_EXIT;		    
    }
    return NBUS_WAITING;
}

