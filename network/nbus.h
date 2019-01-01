#ifndef NBUS_H
#define NBUS_H

int nbus_create( char *s );
int nbus_connect( char *s );
int nbus_get_socket(int bus);
int nbus_poll_event(int bus);
int nbus_accept(int bus);
char *nbus_msg(int bus);
int nbus_msg_sender_fd(int bus);
int nbus_msg_from_id(int bus);
int nbus_printf_dest(int bus, int dest, char *frm, ... );
int nbus_printf(int bus, char *frm, ... );
char* nbus_error_msg(int bus);


enum nbus_event {
    NBUS_WAITING,
    NBUS_ERROR,
    NBUS_MESSAGE,
    NEW_CLIENT,
    CLIENT_REQ,
    CLIENT_EXIT
};


    


#endif
