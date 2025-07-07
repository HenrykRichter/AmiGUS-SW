#ifndef _INC_MODRENDER_H_
#define _INC_MODRENDER_H_

/* **************************************************************************

   Somewhat generic tracker renderer, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   This functionality assumes to have a Mod playing whose temporal increments
   yield in a number of played samples to be mixed. A software mixer will just
   take the current state of the replay and mix a number of samples conforming
   to the interval length between player ticks.

   ************************************************************************** */

#include "types.h" /* redirects to exec/types.h on Amiga */

/* opaque: specific to active renderer */
struct MODRender;


/* init renderer for a mod 
   in:  mod     - struct MOD (after mod_init() )
        rate    - sampling rate of the renderer
        flags   - init flags
   out: 
  
   the returned renderer is prepared for the actual number of channels
   and will contain placeholders for the samples to be loaded. Please load
   the Mod's samples after the init step.
*/
struct MODRender *modrender_init( struct MOD *mod, LONG rate, ULONG flags );

/* check if the compiled-in renderer is actually a player that
   won't return samples

   in:  rnd  - renderer instance
   out: 1 if we have a renderer, 0 if we have a player
*/
LONG modrender_is_player( struct MODRender *rnd );

/* start playing in player mode

   in:  rnd  - renderer instance
   out: >0 if ok

   notes: the play routine in the renderer will call the mod interval
*/
LONG modrender_start_play( struct MODRender *rnd );

/* stop playing in player mode

   in:  rnd  - renderer instance
   out: 0

*/
LONG modrender_stop_play( struct MODRender *rnd );

/* in play mode: set global volume/balance
 *
 * */
LONG modrender_setvolume( struct MODRender *rnd, LONG lvolume, LONG rvolume );

/*
  Pass the loaded samples to the renderer

  out: number of bytes left to be loaded for current sample

  This call may receive partial data, i.e. you may feed the whole data set
  or just a part.
*/
LONG modrender_loadsample( struct MODRender *rnd, LONG idx, UBYTE *buf, LONG bytes );

/*
   in: rnd  - renderer instance (after loading samples)
       time - time frame in 10us units

  out: number of new available samples from renderer

*/
LONG modrender_render( struct MODRender *rnd, LONG time, WORD *buf, LONG bufsize );


/*
  deallocate, free resources
*/
LONG modrender_free( struct MODRender *rnd );




/* INTERNAL */
#ifdef AMIGA
#include <proto/exec.h>
#else
#include <string.h>
#include <stdlib.h>
//#define mod_AllocVec(a) malloc(a)
#define mod_AllocVec(a) calloc(a,1)
#define mod_FreeVec(a) free(a)
#endif


#endif /* _INC_MODPLAY_H_ */
