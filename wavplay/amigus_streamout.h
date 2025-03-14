/*

  amigus_streamout.h

  (C) 2025 Henryk Richter, Oliver Achten

  Output a stream over the AmiGUS FIFO engine
 
*/
#ifndef _INC_AMIGUS_STREAMOUT_H
#define _INC_AMIGUS_STREAMOUT_H

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>

#include "amigus_registers.h"

#ifndef _HAVE_AMIGUS_BUFFER
#define _HAVE_AMIGUS_BUFFER
struct AmiGUS_Buffer {
	struct MinNode	n;
	APTR		buf_adr;    /* load buffer memory address */
	ULONG		buf_size;   /* capacity of buffer in bytes */
	ULONG		buf_length; /* number of bytes stored in buffer  */
	ULONG		buf_pos;    /* current position in reading from buffer */
};
#endif /* _HAVE_AMIGUS_BUFFER */

/* cosy all-in one struct for every task */
struct AmiGUS_Wavout {
	struct Node	n; /* used to bind the card as "driver" */

	struct ConfigDev *board_expansionstruct;
	APTR   		board_base; /* convenience from board_expansionstruct->cd_BoardAddr */

	ULONG		board_write_size; /* FIFO write size */
	ULONG		board_watermark;  /* current watermark level */

	struct MinList 	board_fullbufs;
	struct MinList 	board_emptybufs;

	struct Interrupt board_int;
	ULONG		board_underruns;
	ULONG		board_ints;
	ULONG		board_waterfills; /* times the FIFO was zero-padded in ISR */

	struct Task 	*board_sigtask;
	ULONG		board_sigbit;

	UWORD		sample_rate;
};

ULONG agus_ReadReg16(APTR base, ULONG offset);

/*
  shut down AmiGUS usage

  in:
   struct AmiGUS_Wavout from agus_Init_WAVOUT()

  return:
   status code
*/
LONG agus_Exit_WAVOUT( struct AmiGUS_Wavout *wo );


/*
  find AmiGUS card in system and bind as driver to it

  in:
   flags      - flag mask (keep 0, for now)
   nbuffers   - number of buffers to allocate (see struct AmiGUS_Buffer)
   buffersize - number of bytes buffer space for each buffer

*/
struct AmiGUS_Wavout *agus_Init_WAVOUT( ULONG flags, ULONG nbuffers, ULONG buffersize );


/*
  start playback by priming FIFO (thus triggering interrupt)
*/
LONG agus_Start_WAVOUT( struct AmiGUS_Wavout *wo, ULONG fmt, ULONG rate );

#endif /* _INC_AMIGUS_STREAMOUT_H */
