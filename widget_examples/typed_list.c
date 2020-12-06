#include "mls.h"

int tlist_create( int length, int size, char *type_name )
{
}

/* verify that the handle is of type type_name, if not exit with error.
   if true return pointer to memory address where list item is stored
*/
int tlist_pointer( int handle, int index, char *type_name )
{
}



int tlist_put( int handle, void *data, char *type_name )
{
}

// call killer for every item in list, than delete the list itself */
int tlist_free( int handle, void *data )
{
}


struct user_id {
    char *user_name;
    // assign with strdup(), free with free() */
    int  id;
};

void user_id_killer(int list, int pos)
{
    struct user_id *p;
    p=tlist_pointer( list, pos, "user_id" );
    free(p->user_name);
}


int test_simple1(int argc, char **argv)
{

    struct user_id user;

    int my_list = tlist_create( 50, sizeof(struct user_id), "user_id",
			       user_id_killer );
    /* problem: update user_id_killer if you update the type user_id */


    user.user_name = strdup( "smith, joe" );
    user.id = 42;
    tlist_put( my_list, &user, "user_id" );
    tlist_free(my_list);
    return 0;
}

#if 0
/* what i like to accomplish */
int test_simple2(int argc, char **argv)
{

    struct user_id user;

    int my_list = tlist_create( 50, struct user_id );

    user.user_name = strdup( "smith, joe" );
    user.id = 42;
    tlist_put( my_list, &user ); /* verify typeof(user) == typeof(my_list) */
    tlist_free(my_list); /* call user_id_killer */
}


int main(int argc, char **argv)
{
    test_simple1();
}
