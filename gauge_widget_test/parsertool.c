#include <ctype.h>
#include <stdio.h>
#include <sys/param.h>


int get_num(char **str, int *num)
{
	int n=0;
	char *s = *str;
	while( isdigit(*s) ) {
		n*=10;
		n+=(*s) - '0';
		s++;
	}
	*str = s;
	*num = n;
	return *s;
}

// a-z0-9#
int get_colorname(char **str,char *name,int len )
{
	int i = 0;
	char *s = *str;
	if( *s == '#' ) s++;
	while( isalnum(*s) ) s++;
	snprintf(name,MIN(len, s - *str +1 ), "%s", *str );
	*str = s;
	return *s;
}

