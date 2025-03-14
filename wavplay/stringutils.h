/*
  stringutils.h
  (C) 2023 Henryk Richter

*/
#ifndef _INC_STRINGUTILS_H
#define _INC_STRINGUTILS_H

char *strspccopy(char *dest, char *src, int len ); /* strncpy stopping at anything <32 */
void get_revision( char *idbuf, long *rev );
int my_strnicmp( const char *n1, const char *n2, int len );


#endif /* _INC_STRINGUTILS_H */

