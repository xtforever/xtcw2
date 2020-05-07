#ifndef NBUS_H
#define NBUS_H


/* message bus system 

   how does it work:

   with nbus_create you create a listen-socket on localhost
   the server waits for events and reponds to events
   to wait for an event you can use nbus_poll_event or you
   have your own main-loop (hello xt) and you wait for something
   on the socket-fd (nbus_get_socket(b))
   
   the client connects with nbus_connect() 
   messages are interchanged the same way the server works.
   then client waits for events and reponds to events.
   to wait for an event you can use nbus_poll_event or you
   have your own main-loop (hello xt) and you wait for something
   on the socket-fd (nbus_get_socket(b))
   
   Messages are sepearated by newline (character 10).
   

*/



/* create a socket on localhost service-name is "s" */
int nbus_create( char *s );

/* connect to socket "s" on localhost */
int nbus_connect( char *s );
int nbus_get_socket(int bus);
int nbus_poll_event(int bus);
int nbus_accept(int bus);

/* get message send to "bus" */
char *nbus_msg(int bus);
/* get sender-fd of last message on "bus" */
int nbus_msg_sender_fd(int bus);
/* get sender-id of last message on "bus" */
int nbus_msg_from_id(int bus);

int nbus_printf_dest(int bus, int dest, char *frm, ... );
int nbus_printf(int bus, char *frm, ... );
char* nbus_error_msg(int bus);


/* data is waiting to be processed  */
int nbus_server_message(int bus);
int nbus_client_message(int bus, int client_id);
char* nbus_client_get_msg(int bus, int client_id);
int  nbus_client_fd(int bus, int client_id);


enum nbus_event {
    NBUS_WAITING,
    NBUS_ERROR,
    NBUS_MESSAGE,
    NEW_CLIENT,
    CLIENT_REQ,
    CLIENT_EXIT
};

#endif
