#include <stdio.h>
#include <string.h>

int main()
{
    char lst[] = "-1-111111111111111------------";
    int n = strlen(lst);

    int cur=0,next=0;

    /* first not '1' */
    while( cur < n && lst[cur] == '1' ) cur++;
    next = cur;

    int loop=0;
    for(;;) {
	loop++;
	while( next < n && lst[next] == '-' ) next++;
    
	if( next < n && cur < n ) {
	    char tmp = lst[cur];
	    lst[cur] = lst[next];
	    lst[next] = tmp;
	    cur++;
	    next++;
	} else break;
	while( cur < n && lst[cur] == '1' ) cur++;
    }
    printf("%d\n", loop );
    puts(lst);

}

