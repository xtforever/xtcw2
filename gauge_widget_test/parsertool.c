#include <ctype.h>
#include <stdio.h>
#include <sys/param.h>


int get_num(const char **str, int *num)
{
	int n=0;
	const char *s = *str;
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
int get_colorname(const char **str,char *name,int len )
{

	const char *s = *str;
	if( *s == '#' ) s++;
	while( isalnum(*s) ) s++;
	snprintf(name,MIN(len, s - *str +1 ), "%s", *str );
	*str = s;
	return *s;
}

