#ifndef _MODPLAY_S3M_H
#define _MODPLAY_S3M_H
/* **************************************************************************

   Screamtracker III loader 
   (C) 2024 Henryk Richter

   ************************************************************************** */
#include "modplay.h"

/* external proto */
LONG mod_check_s3m_internal( struct MOD *mod, UBYTE *buf, LONG bufsize );
struct MOD *mod_s3m_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset );

#endif /* _MODPLAY_S3M_H */
