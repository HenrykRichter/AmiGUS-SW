#ifndef _INC_TYPES_H_
#define _INC_TYPES_H_

/* **************************************************************************

   Somewhat generic tracker player, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   ************************************************************************** */

/* just redirect to exec for basic types on AmigaOS */
#ifdef AMIGA

#include <exec/types.h>

#else /* AMIGA */

// perhaps use system types and refer to uint8_t etc.

typedef unsigned char UBYTE;
typedef signed char BYTE;

typedef unsigned short UWORD;
typedef signed short WORD;

typedef unsigned int ULONG;
typedef signed int LONG;

typedef void * APTR;

#ifdef __cplusplus
typedef char *          STRPTR;         /* string pointer */
#else
typedef unsigned char * STRPTR;         /* string pointer */
#endif

#ifndef TRUE
#define	TRUE 1
#endif

#ifndef FALSE
#define	FALSE 0
#endif

#ifndef NULL
#define	NULL (0)
#endif

#endif /* AMIGA */

#endif /* _INC_TYPES_H_ */
