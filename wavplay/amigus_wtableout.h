/*

  amigus_wtableout.h

  (C) 2025 Henryk Richter, Oliver Achten

  Output a stream over the AmiGUS Wavetable engine
 
*/
#ifndef _INC_AMIGUS_WTABLEOUT_H
#define _INC_AMIGUS_WTABLEOUT_H

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>

#include "amigus_registers.h"

struct AmiGUS_WBuffer {
	struct MinNode	n;
	APTR		buf_adrL;   /* load buffer memory address left channel  */
	APTR		buf_adrR;   /* load buffer memory address right channel */
	ULONG		buf_size;   /* capacity of buffer in bytes (for each adrL and adrR) */
	ULONG		buf_length; /* number of bytes stored in buffer (single channel) */
	ULONG		buf_pos;    /* current position in reading from buffer */
};

#define MAX_HWBUFS 2
/* cosy all-in one struct for every task */
struct AmiGUS_WTableOut {
	struct Node	n; /* used to bind the card as "driver" */

	struct ConfigDev *board_expansionstruct;
	struct AmiGUS_Hagen_Regs *board_base; /* convenience from board_expansionstruct->cd_BoardAddr */

	ULONG	chan_mask; /* Bits 31..0 corresponding to the actually live channels */
	ULONG   chan_pair; /* pair of channels, i.e. first of the two consecutive indices */
	ULONG   board_buf_size_total;

	ULONG	board_write_size; /* buffer write size (in bytes) for each of the double buffers */
	ULONG	board_membufs[MAX_HWBUFS][2];	/*  */
	ULONG	board_membufidx;		/* next buffer to activate */

	struct MinList 	board_fullbufs;
	struct MinList 	board_emptybufs;

	struct Interrupt board_int;
	ULONG		board_underruns;
	ULONG		board_ints;

	struct Task 	*board_sigtask;
	ULONG		board_sigbit;

	UWORD		sample_rate; /* sample rate (natural) */
	ULONG   	board_rate;  /* sample rate in board units */
	UWORD		board_voice_ctrl; /* HAGEN_CTRLF_16BIT,HAGEN_CTRLF_LOOP,HAGEN_CTRLF_INTERPOL,HAGEN_CTRLF_PBSTART */
	/* DEBUG */
	ULONG	*ptr_debug;
	ULONG    ptr_debugpos;
};

ULONG agus_ReadReg16(APTR base, ULONG offset);

/*
  shut down AmiGUS usage

  in:
   struct AmiGUS_WTableOut from agus_Init_WAVOUT()

  return:
   status code
*/
LONG agus_Exit_WTableOUT( struct AmiGUS_WTableOut *wo );


/*
  find AmiGUS card in system and bind as driver to it

  in:
   flags      - flag mask (keep 0, for now)
   nbuffers   - number of buffers to allocate (see struct AmiGUS_Buffer)
   buffersize - number of bytes buffer space for each buffer

*/
struct AmiGUS_WTableOut *agus_Init_WTableOUT( ULONG flags, ULONG nbuffers, ULONG buffersize );


/*
  start playback by priming FIFO (thus triggering interrupt)
*/
LONG agus_Start_WTableOUT( struct AmiGUS_WTableOut *wo, ULONG fmt, ULONG rate );

#endif /* _INC_AMIGUS_WTABLEOUT_H */
