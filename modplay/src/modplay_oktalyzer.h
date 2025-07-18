#ifndef _MODPLAY_OKTALYZER_H
#define _MODPLAY_OKTALYZER_H
/* **************************************************************************

   Oktalyzer loader 
   (C) 2025 Henryk Richter

   ************************************************************************** */
#include "modplay.h"

/* external proto */
LONG mod_check_ok_internal( struct MOD *mod, UBYTE *buf, LONG bufsize );
struct MOD *mod_ok_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset );

#endif /* _MODPLAY_OKTALYZER_H */
