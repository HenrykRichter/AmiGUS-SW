#ifndef _MODPLAY_PROTRACKER_H
#define _MODPLAY_PROTRACKER_H
/* **************************************************************************

   Protracker (and similar) loader 
   (C) 2024 Henryk Richter

   ************************************************************************** */
#include "modplay.h"

/* external proto */
LONG mod_check_pt_internal( struct MOD *mod, UBYTE *buf, LONG bufsize );
struct MOD *mod_pt_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset );

#endif /* _MODPLAY_PROTRACKER_H */
