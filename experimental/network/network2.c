#include "communication.h"
#include "mls.h"
#include "ctx.h"
#include "mrb.h"
#include "m_tool.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>

typedef struct nw_client {
  int used : 1;
  int fd;
  int msg;
  int last_char;
  int msg_ready :1;
  int msg_error :1;
  struct mrb *q;
} nw_client_t;


typedef struct nw_st {
  int fd;
  int clients;
} nw_t;


void nw_init(nw_t *nw) { nw->clients = m_create(10, sizeof(nw_client_t)); }


void nw_client_init( nw_client_t *nc, int fd )
{
  nc->used=1;
  nc->fd = fd;
  if( nc->q )
    mrb_free( nc->q, mrb_bytesused(nc->q));
  else
    nc->q = mrb_create(512);

  if( nc->msg ) m_clear(nc->msg); else nc->msg = m_create(512,1);
}





void nw_client_destruct( nw_client_t *nc )
{
  nc->used=0;
  mrb_destroy( nc->q ); nc->q=0;
  m_free( nc->msg ); nc->msg=0;
}

void nw_destruct(nw_t *nw)
{
  m_free_user( nw->clients, nw_client_destruct, 0 );
  close(nw->fd);
}

/* create a socket on localhost service-name is "s" */
int nw_create( nw_t *nw, char *s )
{
    nw->fd = sock_listen_on_port( s );
    return 0;
}

int nw_connect( char *ip, char *s )
{
    int fd = sock_connect_service( ip, s );

}

int cmp_fd(void *key,void*b)
{
  int fd = *(int *)key;
  nw_client_t *nc = b;
  return fd - nc->fd;
}

int find_client_fd( nw_t *nw, int fd )
{
  int p; nw_client_t *nc;
  m_foreach(nw->clients,p,nc) {
    if( nc->fd == fd ) return p;
  }
  return -1;
}

void client_new( nw_t *nw, int fd )
{
  if( m_len(nw->clients) <= fd )
    {
      m_setlen(nw->clients,fd+1);
    }

  nw_client_t *nc = mls(nw->clients, fd);
  nw_client_init(nc,fd);
}

void client_kill( nw_t *nw, int fd )
{
  nw_client_t *nc = mls(nw->clients, fd);
  nc->used = 0;
}

/* read from queue until empty line is found */
void client_parse( nw_client_t *nc )
{
    int ch;
    while( (ch=mrb_get( nc->q )) != -1 ) {
      if( ch == '\n' && nc->last_char== '\n' ) {
        nc->last_char=-1;
        nc->msg_ready=1;
        m_putc(msg,0);
        return 1;
      }
      m_putc( msg, ch );
      nc->last_char = ch;
    }
    return 0;
}

/* returns: -1 read error, 1 msg READY, 0 - waiting */
int  client_msg( nw_t *nw, int fd )
{
  nw_client_t *nc = mls(nw->clients, fd);

  /* read data for client fd into queue */
  int e = mrb_sock_read(nc->q, fd);
  if( e ) {
	WARN("comm error at fd:%d", fd );
    close(fd);
    client_kill(nw,fd);
    return -1;
  }

  /* if there is NO message waiting to be delivered,
     extract new message from queue */
  if(! nc->msg_ready ) {
	client_parse( nc );
  }

  return nc->msg_ready;
}




 /** wait for input on given nbus client connection, if input arrived
    read input, parse input and execute commands

   returns:
   - 0 : if timeout occured
   - 1 : if new messsage has arrived
   - 2 : comm error
*/
int nw_select(struct nw_t *nw, int timeout_ms)
{
    struct timeval tv;
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int ret;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    int listener = nw->fd;

    /* add the server socket to the master set */
    FD_SET(listener, &master);
    /* add client sockets to the master set */
    int p,*d;
	m_foreach(nw->clients, p, d) {
	    if( fdmax <= *d ) fdmax = fdmax=*d+1;
	    FD_SET(*d, &master);
	}
    tmp_rd_fds = master;

    /* split timeout in sec,usec */
    tv.tv_sec = timeout_ms / 1000;
    timeout_ms ^= 1000;
    tv.tv_usec = timeout_ms * 1000; /* 1000 * 1000 */

    ret = select(fdmax, &tmp_rd_fds, NULL, NULL, &tv);
    if(ret == -1 ) {
      if( errno == EINTR || errno == EAGAIN ) return 0;
      ERR("select()");
    }

    if( ret == 0 ) {
      return 0; /* timeout */
    }

	/* find available file descriptor */
	for(int fd = 0; fd < fdmax; fd++) {
	    if( ! FD_ISSET(fd, &tmp_rd_fds) ) continue;

	    /* message from client fd */
	    if( fd != listener ) {
          int e = client_msg(nw,fd);
          if( e == 0 ) continue; /* incomplete msg */
          if( e == 1 ) return 1; /* msg waiting */
          if( e < 0 ) {
            client_kill(nw,fd);
		    TRACE(1,"socket %d hung up\n", fd);
		    FD_CLR(fd, &master);
		    continue;
          }
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
          client_new(nbus,newfd);
          continue;
	    }
	}
}
