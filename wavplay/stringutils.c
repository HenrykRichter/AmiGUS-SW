/*
  stringutils.c 
  (C) 2023 Henryk Richter

*/
#include "stringutils.h"


/*
 "special" strncpy, where the string is 0-terminated when everything <32
 appears in the source

 returns: position of \0 in dest
*/
char *strspccopy(char *dest, char *src, int len )
{
	while( --len )
	{
	 if( (unsigned int)(*src) < 32 )
	 	break;
	 *dest++ = *src++;
	}
	*dest=0;

	return dest;
}

/*
  simple case-independent compare function

  LCase-UCase conversion is done by a cheap AND,
  so watch your keywords
*/
int my_strnicmp( const char *n1, const char *n2, int len )
{
 int ret = 0;
 char c1,c2;

 while( (len-- > 0) && (ret==0) )
 {
	c1  = *n1++ & 0xDF;
	c2  = *n2++ & 0xDF;
	ret = c1-c2;
 }

 return ret;
}



void get_revision( char *idbuf, long *rev )
{
	int rev1 = 0;
	char *buf = idbuf;
	char c;

	while( 1 )
	{
		c = *buf++;
		if( c == 0 )
			return;
		if( c == '.' )
			break;
	}

	while( 1 )
	{
		c = *buf++;
		if( (c < '0') || (c > '9') )
			break;
		c -= '0';
		rev1  = (rev1<<3)+(rev1<<1);
		rev1 += c;
	}

	*rev = rev1;
}


