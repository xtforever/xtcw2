/*

  network.c - 2018 - 12 - 28 
  (c) jens harms - au1064@gmail.com

  networking interface

  inter client bus protocol 

  
  access control: filesystem attributes (read flag)

  
*/





#include "nbus.h"
#include "mls.h"
#include <stdio.h>



int  conn_open(char *name)
{
    int fd;
    /* 
       fd = open( "/tmp/.ICE-unix/" + $name ) 
    */
    return fd=0;
}




int client()
{
    int ev;
    int bus = nbus_connect("TEMP");
    int ok=0;
    int msg_send=0;

    TRACE(1,"client started");
    
 loop:
    while( (ev=nbus_poll_event(bus)) == 0 ) ;

    
    /* msg_id = (ev >> 8) & 0xff  */
    /* err_id = (ev >> 16) & 0xff */
    /* state  = (ev >> 24) & 0xff */

    /* nbus client loop */
    switch(ev & 0xff) {		/* limit number of events to 255, if you need more you are in trouble */

    case NBUS_ERROR:
	ok=0;
	puts( nbus_error_msg(bus) );
	break;
    case NBUS_MESSAGE:
	TRACE(1,"Message Recv");
	ok=1;
	char *s = nbus_msg(bus);
	puts( s );
    }

    TRACE(1,"ping");
    
    if( ok == 0 ) { msg_send=0; }
    if( ok == 1 && !msg_send ) {
	TRACE(1,"try message sending");
	nbus_printf(bus, "hello server\n"); /* send message to server*/
	msg_send=1;
    }
    goto loop;
}


int main(int argc, char **argv)
{
    int ev;
    
    m_init();
    trace_level=1;
    if( argc > 1 ) {
	client();
	exit(1);
    }

    /* server */
    int bus = nbus_create("TEMP");
    int fd = nbus_get_socket(bus); /* for your own select loop */
    int id;
    char *s;
    (void) fd;
    
 loop:
    TRACE(1,"ping");
    while( (ev=nbus_poll_event(bus)) == 0 ) putchar('.');
    putchar(10);
    switch(ev & 0xff) {		/* limit number of events to 255, if you need more you are in trouble */
    case NEW_CLIENT:		/* a new client wants to talk to us */
	id  = nbus_msg_from_id(bus); 
	nbus_printf_dest(bus, id, "welcome\n" ); /* send welcome message */
	puts("welcome\n");	/* welcome */
	break;
    case CLIENT_EXIT:		/* client left */
	puts("client exit");
	break;
    case CLIENT_REQ:		/* client starts talking */
	s = nbus_msg(bus); /* what is he saying? */
	id  = nbus_msg_from_id(bus); /* get return address */
	TRACE(1,"msg to client: %s", s );
	nbus_printf_dest(bus, id, "you said: %s\n", s ); /* send his message back */
	break;
    default:
	puts("unknow event");
    }
    goto loop;
}
