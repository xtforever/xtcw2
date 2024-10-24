#include "subshell.h"

#include <utmp.h>
#include <pty.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <errno.h>
#include <stdarg.h>

#include "mls.h"
#include "mrb.h"
#include "ctx.h"

int trace_child = 2;

static void xclose( int *fd )
{
    if( *fd < 0 ) return;
    close(*fd);
    *fd = -1;
}




/** create a child process and connect a 3 pipes
    to child 

    create some pipes:
    
    0 R                READ from child STDOUT
    1 W     closed
    2 R     closed
    3 W                WRITE to child  STDIN
    4 R                READ from child STDERR
    5 W     closed            

    0: read <--- pipe < stdout
    1: write --> pipe > stdin
    2: read <--- pipe < stderr

*/
#if 0
static void fork2_exec(struct fork2_info *child, int args )
{
    memset( child->fd, 0xff, sizeof( child->fd ));
    if( pipe2( child->fd, 0 ) ) ERR("pipe2");
    if( pipe2( child->fd+2, 0) ) ERR("pipe2");
    if( pipe2( child->fd+4, 0) ) ERR("pipe2");

    pid_t cpid = fork();
    if (cpid == -1) ERR("fork");
    if (cpid == 0) {            /* Child reads from pipe */
        dup2(child->fd[1],1);   /* make STDOUT==1 same as WRITE-TO==1 end
				   of pipe-A  */

	dup2(child->fd[2],0);   /* make STDIN==0 same as READ-FROM==0 end
	                         of pipe-B */

	/* stdin = read-end of pipe a */
	dup2(child->fd[5], 2);  /* stderr = write-end of pipe c */

        xclose(child->fd+0);    /* Close unused read end pipe-A */
        xclose(child->fd+3);    /* Close unused write end pipe-B */
        xclose(child->fd+4);    /* Close unused read end pipe-c */

	#if 0
        if( trace_child >= trace_level ) {
            char **sp; int i;
	    fprintf(stderr, "exec: " );    
	    m_foreach( args, i, sp ) {
		fprintf(stderr, "%s ", *sp );
	    }
            fprintf(stderr, "\n" );
        }
	#endif

        execvp( STR(args,0), m_buf(args) );
        perror( "can not exec child process" );
        _exit(EXIT_FAILURE);    /* never reached */
    }
    xclose( child->fd+1 );      /* close unused WRITE end of pipe-A */
    xclose( child->fd+2 );      /* close unused READ  end of pipe-B */
    xclose( child->fd+5 );      /* close unused WRITE end of pipe-C */
    child->stat = CHILD_RUNNING;
    child->pid = cpid;
}
#endif

static void execute(struct fork2_info *child, int args)
{
    int rc = forkpty(&child->masterfd, NULL, NULL, NULL);
    if ( rc < 0 ) {
	ERR("can not fork");
    }

    if ( rc == 0 ) {
	execvp( STR(args,0), m_buf(args) ); // never returns
	ERR("execvp not executed");
    }
    
    // Parent
    child->stat = CHILD_RUNNING;
    child->pid = rc;
    return;
}




struct fork2_info *fork2_open(char *filename, ...)
{
    char *name;
    va_list ap;
    int args = m_create(10,sizeof(char*));
    name = strdup(filename);
    m_put(args,&name);

    va_start(ap,filename);
    while( (name = va_arg( ap, char* )) != NULL )
    {
        name = strdup(name);
	m_put(args, &name);
    }
    name = NULL;
    m_put(args,&name);
    va_end(ap);

    struct fork2_info *child = fork2_open2( args );
    m_free_strings(args,0);
    return child;
}

struct fork2_info *fork2_open2( int args )
{
    char *name;
    name = NULL;
    m_put(args,&name);
    struct fork2_info *child = calloc(1,sizeof (struct fork2_info));
    child->pipe_buf[0] = mrb_create(MRB_BUFSIZE);
    child->pipe_buf[1] = mrb_create(MRB_BUFSIZE);

    execute(child,args);
    // fork2_exec(child, args );
    return child;
}

void fork2_open3( struct fork2_info *child, int args )
{
    char *name;
    name = NULL;
    m_put(args,&name);
    child->pipe_buf[0] = mrb_create(MRB_BUFSIZE);
    child->pipe_buf[1] = mrb_create(MRB_BUFSIZE);
    execute(child,args);
}


/** @returns -1 if pid is running, otherwise it's exit code */
int check_child( int pid )
{
    int wstatus;
    if( waitpid( pid, &wstatus, WNOHANG ) == pid ) {
	return WEXITSTATUS(wstatus);
    }
    return -1;
}

/* non blocking read from child pipe
   returns
   >=0: bytes read
   -1: read error
*/
static int mrb_read(int fd, struct mrb *mrb )
{
    // mrb_sock_read( mrb, fd );

    int free_space;


    fd_set rfds;
    struct timeval tv;
    int retval;
    
    /* Watch fd to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    if (retval < 0 ) {
	TRACE(1, "Warning: read error on fd:%d", fd );
	perror("");
	return -1;
    }

    if( retval == 0 ) { // timeout
	TRACE(1, "Warning: no data available on fd:%d",fd );
	return mrb_error( mrb );
    }


    TRACE(1, "rd:%d wr:%d size:%d, used:%d", mrb->rd, mrb->wr, mrb->size,   mrb_bytesused(mrb) );
    char *buf  = mrb_maxsize(mrb, &free_space);
    TRACE(trace_child,"chunksize: %d", free_space );
    if( free_space == 0 ) return 0;
    int nread;
    nread = read( fd, buf, free_space );

    TRACE(trace_child, "read: %d %s", nread, nread < 0 ? "read error" : "OK" );
    if( nread < 0 )  {
        return -1;
    }
    
    if( nread == 0 ) {
	TRACE(1,"read returns 0, could be the child exited");
	return mrb_error( mrb );
    }

    mrb_error_clear( mrb );
    mrb_alloc( mrb, nread );
    
    // int count = Min( 20, nread ); 
    // write( 2, buf + nread - count, count ); write(2, "\n", 1 ); 

    return nread;
}


/** kill child and free all resources */
void fork2_close( struct fork2_info *child )
{
    fork2_close3(child);
    free(child);
}


void fork2_kill( struct fork2_info *child )
{
    TRACE(1,"");
    int status,err;
    if( child->stat == CHILD_RUNNING ) {
        kill( child->pid, SIGKILL );
        TRACE(trace_child, "sending kill" );
        err=waitpid(child->pid,&status,0);
        TRACE(trace_child, "waitpid = %d", err );
	child->exit_value = status;
        child->stat = 0;
    }
}

void fork2_close3( struct fork2_info *child )
{
    TRACE(1,"");

    fork2_kill(child);
    
    for(int i=0;i<2;i++) {
	if(child->pipe_buf[i]) { free(child->pipe_buf[i]); }
	child->pipe_buf[i]=0;
    }
    
    xclose(& child->masterfd );

}




/** Ein newline ab |*pos| suchen.
    returns: 1 - zeile bis pos ausgeben, 0 - warten auf weitere daten
*/
static int find_newline(struct mrb *c, int *pos)
{
    int ch;
    for(;;) {
        ch = mrb_peek(c, pos );
        if( ch == 10 ) return 1;
        if( ch < 0 ) {
	    TRACE(1,"peek returns -1 on pos %d", *pos);
            /* ist puffer voll aber kein newline vorhanden? overflow error */
            if( *pos == mrb_bufsize(c) ) {
		WARN("read overflow in find newline at end of buffer");
		return 1;	
	    }
            return 0;
        }
    }
}

/* nächste zeile aus dem eingabe-puffer holen
   m - marray of char, wird immer gelöscht
   returns 0: keine weiteren zeilen im puffer
*/
static int  mrb_getline(struct mrb *c, int m, int *pos)
{

    if(!c) {
	WARN("reading from dead pipe" );
	return 0;    	
    }

    TRACE(1, "rd:%d wr:%d size:%d, used:%d, pos:%d", c->rd, c->wr, c->size,   mrb_bytesused(c), *pos );
    if(! find_newline( c, pos ) ) {
	TRACE(1,"new pos: %d", *pos);
	return 0;
    }
    
    TRACE(1, "buffer read bytes: %d", *pos );
    m_clear(m);
    while( *pos > 0 ) {
	int ch = mrb_get(c);
	if( ch < 0 ) { WARN("line not terminated by 0x0a"); break; }
	if( ch==13 ) continue;
	if( ch==10 ) break;
	m_put(m, &ch);
	(*pos)--;
    }
    m_putc(m,0);	
    *pos = 0;
    TRACE(1, "rd:%d wr:%d size:%d, used:%d", c->rd, c->wr, c->size,   mrb_bytesused(c) );
    
    return m_len(m);
}

/** read
 * returns" -1 on read error, -2 if child is not running and no more data available
 */
int fork2_read(struct fork2_info *child, int pipe )
{
    int err;
    
    int fd = child->masterfd;
    if( pipe ) {
	pipe = 1;
	ERR("stderr not supported");
    }
    

    /* pipe is closed */
    if( fd <0
	||  child->pipe_buf[pipe] == 0 ) return -1;
    
    /* read data */
    if( (err=mrb_read( fd, child->pipe_buf[pipe] )) < 0 )
        {
            TRACE(trace_child, "error reading from child" );
            return -1;
        }

    /* read() returns 0 */
    if( err == 0 ) {
	TRACE(trace_child, "no more data, checking  child" );
    }
    
    TRACE(1,"read success, leave");
    return 0; /* OK */
}



int fork2_getchar(struct fork2_info *child, int pipe )
{
    TRACE(1,"");
    if( pipe ) pipe = 1;
    return mrb_get(child->pipe_buf[pipe]);
}

/* nächste zeile aus dem eingabe-puffer holen
   m - marray of char, wird immer gelöscht
   returns 1: keine weiteren zeilen im puffer, sonst 0
*/
int fork2_getline( struct fork2_info *child, int pipe, int lnbuf )
{
    if(! child ) return 1;
    if( pipe ) { pipe = 1; TRACE(1,"stderr" ); }
    else { TRACE(1,"stdout" ); }
    return mrb_getline(child->pipe_buf[pipe], lnbuf, child->scan_pos+pipe) == 0;
}

int fork2_write( struct fork2_info *child, char *s )
{
    dprintf(child->masterfd,"%s", s);
    fsync(child->masterfd);
    return 0;
}




static int SUBSHELL =0;
static int SIGNAL_SIGCHLD = 0;

static struct fork2_info *shell_ctx(int n)
{
    return mls(SUBSHELL,n);
}

static void shell_free_cb(int *ctx, int n)
{
    struct fork2_info *sh = shell_ctx(n);
    fork2_close3(sh);
}

void shell_close( int n )
{
    ctx_free( &SUBSHELL, n, shell_free_cb );
    SIGNAL_SIGCHLD = 0;
}

/**
 * @returns -1 for error
 */
int  shell_create( int args )
{
    int h = ctx_init( &SUBSHELL, 2, sizeof(struct  fork2_info));
    struct fork2_info *sh = shell_ctx(h);
    fork2_open3(sh, args );
    return h;
}

/** execute (mstr) filename 
 * @returns -1 for error
 */
int  shell_create1( int filename )
{
    int h = ctx_init( &SUBSHELL, 2, sizeof(struct  fork2_info));
    struct fork2_info *sh = shell_ctx(h);
    int args = m_create(1,sizeof(char*));
    char *buf = m_str(filename);
    m_put(args, & buf );
    fork2_open3(sh, args );
    m_free(args);
    return h;
}


/** 
 * solange noch daten im puffer sind, diese ausgeben
 *  danach neue daten einlesen
 * falls ein lesefehler auftritt oder der signal-handler gemeldet hat,
 * das keine daten mehr vorhanden sind die shell beenden
 * @returns 1 - daten gefunden, 0 warten auf daten, -1 error
*/
int  shell_getline(int h, int p, int buf)
{
    int err;
    struct fork2_info *sh = shell_ctx(h);

    // alle daten aus dem puffer ausliefern
    if( fork2_getline( sh, p, buf ) == 0 ) return 1;

    // neue daten einlesen
    err = fork2_read( sh, p );
    if( err < 0 ) {
	return -1;
    }

    // jetzt wieder alle daten aus dem puffer ausliefern,
    // wenn neue daten aufgelaufen sind, und diese funktion
    // jetzt anzeigt das keine weiteren daten vorhanden sind und dann auch keine
    // durch select angemeldet werden steht das hauptprogramm  fuer immer
    if( fork2_getline( sh, p, buf ) == 0 ) return 1;

    return 0;
}

int  shell_write(int h, char *msg)
{
    TRACE(1,"");
    struct fork2_info *sh = shell_ctx(h);
    return fork2_write(sh,msg);
}

struct mrb *shell_queue(int h, int p)
{
    return shell_ctx(h)->pipe_buf[ p == CHILD_STDERR_RD ];
}

int shell_fd(int h, int n)
{
    return shell_ctx(h)->masterfd;
}

/** @returns 0 - pid gefunden, 1 - pid nicht gefunden
 */
int  shell_signal(int pid, int exit_value)
{
    TRACE(1, "pid: %d", pid);
    int p;
    struct fork2_info *sh;
    m_foreach( SUBSHELL, p, sh) {
	if(!sh->init) continue;
	if( sh->pid == pid ) {
	    sh->stat = 0;
	    sh->exit_value=exit_value;
	    return 0;
	}
    }
    return 1;
}

void shell_signal_cb(int dum)
{
    TRACE(1,"%d", dum);
    TRACE(1,"sigchld");
    SIGNAL_SIGCHLD=1;
    return;
}

/* returns 0 if child still running */
int shell_check(int h)
{
    return SIGNAL_SIGCHLD;
}
