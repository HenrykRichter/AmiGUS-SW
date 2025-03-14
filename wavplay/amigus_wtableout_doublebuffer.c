/*


  amigus_wtableout.c
  (renamed amigus_wtableout_doublebuffer.c)

  (C) 2025 Henryk Richter

  Output a stream over the AmiGUS Wavetable engine

  In the way of resolution (16 Bit instead of 24), the Wavetable engine 
  appears less attractive for stream output than the main FIFO. Also
  interleaved stereo streams are not supported (as of this writing).

  On the other hand, there is an interrupt for each of the 32
  channels. Hence, up to 32 streams could be output concurrently
  and by independent applications (pending a management resource).


  FIXME:   The approach in this file uses double buffering, i.e. 
  write a new pointer and end pointer to the hardware at each 
  interrupt (Paula style). As of this writing, it does not work
  yet as intended. Quite possibly, the interrupt issue 
  (see amigus_wtableout.c) is the culprit here, as well.


*/
#include "amigus_wtableout.h" /* amigus_registers.h, <exec/types.h> */

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <libraries/configvars.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <hardware/intbits.h>

#include "compiler.h"
#include "macros.h"

/* internal proto */
LONG agusw_StartBuffer( struct AmiGUS_WTableOut *wo, LONG membufidx, LONG chan_pair );
void agusw_StopPlay( struct AmiGUS_Hagen_Regs*board, ULONG chanmask );
LONG agusw_WriteBuffer(  struct AmiGUS_WTableOut *wo, LONG write_size, LONG zeropad );
ASM LONG direct_AmiGUS_WTableOut_ISR( ASMR(a1) struct AmiGUS_WTableOut *wo ASMREG(a1) );
extern ULONG AmiGUS_WTableOut_ISR[3]; /* inline ASM, hex coded */

/* internal data */
const char *wot_name = (char *)"AmiGUS WAVTablePlay";


/* this should go down to 1024 for lower latency realtime purposes */
#define PER_INT_SEGMENT_SIZE 16384


void agusw_StopPlay( struct AmiGUS_Hagen_Regs*board, ULONG chanmask )
{
	/* clear only those ints actually used */
	board->INTE0 =  chanmask      & 0xFF; /* disable Voice Ints 0-7   */
	board->INTE1 = (chanmask>>8)  & 0xFF; /* disable Voice Ints 8-15  */
	board->INTE2 = (chanmask>>16) & 0xFF; /* disable Voice Ints 16-23 */
	board->INTE3 = (chanmask>>24) & 0xFF; /* disable Voice Ints 24-31 */

	{
	 int i;
	 for( i=0 ; i < 32 ; i++ )
	 {
	 	if( chanmask & 1 )
		{
		 board->VOICE_BNK = i;
		 board->VOICE_CTRL   = 0; /* stop playback */
		 board->VOICE_PLOOP  = 0;
		 board->VOICE_PSTRT  = 0;
		 board->VOICE_PEND   = 0;
		 board->VOICE_VOLUMEL= 0; /* 0...ffff */
		 board->VOICE_VOLUMER= 0;
		 /* board->VOICE_RATE = 0; */
		}
		chanmask >>= 1;
	 }
	}
}


/*
  internal: load the two channel registers with the new pointers

  This is to be called at start of playback and after preparing a new
  buffer within the interrupt code.
  
  Write new Loop Pointer and Loop End Pointer, then re-write CTRL (for good measure)
*/
LONG agusw_StartBuffer( struct AmiGUS_WTableOut *wo, LONG membufidx, LONG chan_pair )
{
//	LONG bfidx = wo->board_membufidx; /* next buffer to activate */
	ULONG write_addrL,write_addrR;
	struct AmiGUS_Hagen_Regs *board = wo->board_base;

	/* grab current write buffer */
	write_addrL = wo->board_membufs[membufidx][0];
	write_addrR = wo->board_membufs[membufidx][1];

	/* second channel is the "right" channel */
	board->VOICE_BNK = chan_pair+1;
	board->VOICE_PLOOP = write_addrR;
	board->VOICE_PEND  = write_addrR+wo->board_write_size;
	board->VOICE_CTRL  = wo->board_voice_ctrl;

	/* first channel is the "left" channel */
	board->VOICE_BNK = chan_pair;
	board->VOICE_PLOOP = write_addrL;
	board->VOICE_PEND  = write_addrL+wo->board_write_size;
	board->VOICE_CTRL  = wo->board_voice_ctrl; /* HAGEN_CTRLF_16BIT,HAGEN_CTRLF_LOOP,HAGEN_CTRLF_INTERPOL,HAGEN_CTRLF_PBSTART */

	return 0;
}


/*
  Fill up next buffer, zero pad (if necessary)

  -> this just writes data and advances input buffers, buffer swap and hardware registe writes
     happen outside
*/
LONG agusw_WriteBuffer(  struct AmiGUS_WTableOut *wo, LONG write_size, LONG zeropad )
{
	LONG tmp,write_addrL,write_addrR;
	UWORD ws2;
	ULONG *sptr;
	struct AmiGUS_WBuffer *buf;
	struct AmiGUS_Hagen_Regs *board = wo->board_base;

//	volatile ULONG *datptr,*adrptr;
//	adrptr = (volatile ULONG*)&board->WADDR;
//	datptr = (volatile ULONG*)&board->WDATA;

	/* grab current write buffer */
	write_addrL = wo->board_membufs[wo->board_membufidx][0];
	write_addrR = wo->board_membufs[wo->board_membufidx][1];

	while( (buf = GetHead( &wo->board_fullbufs ) ) )
	{
		LONG rem = buf->buf_length - buf->buf_pos; /* remaining in buffer */
		if( rem > 0 )
		{
			tmp  = ( rem > write_size ) ? write_size : rem;	/* number of bytes to copy from current buffer */

			ws2  = tmp>>2;
			sptr = (ULONG*)((UBYTE*)buf->buf_adrL + buf->buf_pos);

			board->WADDR = write_addrL; //*adrptr = write_addrL;
			while( ws2-- )
				board->WDATA = *sptr++; //*datptr = *sptr++;

			ws2  = tmp>>2;
			sptr = (ULONG*)((UBYTE*)buf->buf_adrR + buf->buf_pos);

			board->WADDR = write_addrR; //*adrptr = write_addrR;
			while( ws2-- )
				board->WDATA = *sptr++; //*datptr = *sptr++;

			buf->buf_pos += tmp; 				/* new pos in buffer */
			rem -= tmp; 					/* update remaining in buffer */
			write_size -= tmp; 				/* FIFO bytes still to be written */
		}
		if( rem <= 0 )
		{
		 REMOVE( buf );
		 ADDTAIL( &wo->board_emptybufs, buf );

		 /* signal our task */
		 Signal( wo->board_sigtask, 1<<wo->board_sigbit ); 
		}
		if( write_size <= 0 )
			break;
	}

	if( write_size > 0 )
	{
	 wo->board_underruns++;

	 if( zeropad )
	 {
	 	ws2 = write_size>>2;

		board->WADDR = write_addrL; //*adrptr = write_addrL;
		tmp = ws2;
		while(tmp--)
			board->WDATA = 0; //*datptr = 0;

		board->WADDR = write_addrR; //*adrptr = write_addrR;
		tmp = ws2;
		while(tmp--)
			board->WDATA = 0; //*datptr = 0;

		write_size = 0;
	 }
	}

	return write_size;
}



static void myNewMinList( struct MinList *l )
{
  l->mlh_TailPred = (struct MinNode *)l;
  l->mlh_Head = (struct MinNode *)&l->mlh_Tail;
  l->mlh_Tail = 0;
}



/*
  shut down AmiGUS usage

  in:
   struct AmiGUS_WTableOut from agus_Init_WTableOUT()

  return:
   status code
*/
LONG agus_Exit_WTableOUT( struct AmiGUS_WTableOut *wo )
{
	APTR board_base;
	struct MinNode *mbuf;

	if( (LONG)wo <= 0 )
		return 0;

	/* we're leaving... */
	if( wo->board_expansionstruct )
	{
		board_base = wo->board_expansionstruct->cd_BoardAddr;

		/* are we bound ? */
		if( wo->board_expansionstruct->cd_Driver == &wo->n )
		{

/* DEBUG STUFF */
#if 0
	Printf("INTs INTC 0x%lx INTE 0x%lx FIFO %ld\n",
	(ULONG)agusw_ReadReg16(board_base,MAIN_INTC),
	(ULONG)agusw_ReadReg16(board_base,MAIN_INTE),
	(ULONG)agusw_ReadReg16(board_base,MAIN_FIFO_USE));

	agusw_WriteReg16(board_base,MAIN_INTE,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS
	agusw_initFIFO(NULL, wo->board_base, (6144>>2)-1); // Arm FIFO (with zeroes) to kick-off interrupt
	
	Printf("INTs INTC 0x%lx INTE 0x%lx FIFO %ld\n",
	(ULONG)agusw_ReadReg16(board_base,MAIN_INTC),
	(ULONG)agusw_ReadReg16(board_base,MAIN_INTE),
	(ULONG)agusw_ReadReg16(board_base,MAIN_FIFO_USE));

	Printf("%ld underruns at %ld ints\n",(LONG)wo->board_underruns,wo->board_ints);
#endif

			agusw_StopPlay( board_base, (wo->chan_mask)|(wo->chan_mask<<1) ); /* use first two channels only */

			/* we're not touching the board anymore, honest! */
			Forbid();
			 if( wo->board_int.is_Code != NULL ) /* check if we have actually added the interrupt */
				RemIntServer( INTB_PORTS, &wo->board_int );
			 wo->board_int.is_Code = NULL;
			 wo->board_expansionstruct->cd_Driver = NULL;
			Permit();
		}
	}

	if( wo->board_sigbit != -1 )
		FreeSignal( wo->board_sigbit );

	/* free memory buffers (if any) -> interrupt no longer running */
	while( (mbuf = GetHead( &wo->board_fullbufs ) ) )
	{
		REMOVE( mbuf );
		FreeVec( mbuf );
	}
	while( (mbuf = GetHead( &wo->board_emptybufs ) ) )
	{
		REMOVE( mbuf );
		FreeVec( mbuf );
	}


	FreeVec( wo );

	return 0;
}


/*
  find AmiGUS card in system and bind as driver to it

  in:
   flags      - flag mask (keep 0, for now)
   nbuffers   - number of buffers to allocate (see struct AmiGUS_WBuffer)
   buffersize - number of bytes buffer space for each buffer

*/
struct AmiGUS_WTableOut *agus_Init_WTableOUT( ULONG flags, ULONG nbuffers, ULONG buffersize )
{
	struct AmiGUS_WTableOut *ret;
	struct AmiGUS_Hagen_Regs*board;
	struct Library *ExpansionBase;
	LONG status = 0;

	ExpansionBase = OpenLibrary( (STRPTR)"expansion.library", 36 ); /* 2.0+ */
	if( !ExpansionBase )
		return NULL;

	ret = AllocVec( sizeof( struct AmiGUS_WTableOut ), MEMF_PUBLIC|MEMF_CLEAR );
	do
	{
		if( !ret )
			break;

		/* minimum TODO when "ret != NULL": lists and signal(s) */

		myNewMinList( &ret->board_fullbufs  );
		myNewMinList( &ret->board_emptybufs );

		ret->board_underruns = 0; /* yes, allocated with MEMF_CLEAR, just to clarify */
		ret->board_ints = 0;

		ret->board_sigbit = AllocSignal(-1);
		if( ret->board_sigbit == -1 )
		{
			status = -4;
			break;
		}
		ret->board_sigtask = FindTask(NULL);


		/* main stuff */

		/* bind as driver: set a name in the node we're supposed to attach */
		ret->n.ln_Name = (char*)wot_name;

		ret->board_expansionstruct = FindConfigDev( NULL, AMIGUS_MANUFACTURER_ID, AMIGUS_HAGEN_PRODUCT_ID );
		if( !ret->board_expansionstruct )
		{
			status = -5;
			break;
		}

		/* is the board good to use ? */
		ret->board_base = (struct AmiGUS_Hagen_Regs*)ret->board_expansionstruct->cd_BoardAddr;
		if( ret->board_expansionstruct->cd_Flags & (CDF_SHUTUP|CDF_BADMEMORY) )
		{
			status = -1;
			break;
		}

		/* see if anybody is there already, if not then bind driver */
		Forbid();
		 if( ret->board_expansionstruct->cd_Driver == NULL )
			ret->board_expansionstruct->cd_Driver = &ret->n; /* we're a driver ! */
		 else
			status = -2;
		Permit();

		if( status != 0 )
			break;


		/* obtain memory for buffers */
		while( nbuffers-- )
		{
			struct AmiGUS_WBuffer *buf = AllocVec( sizeof( struct AmiGUS_WBuffer ) + 2*buffersize, MEMF_PUBLIC|MEMF_CLEAR );
			if( !buf )
			{
				status = -3;
				break;
			}
			buf->buf_adrL  = (APTR)(buf+1);
			buf->buf_adrR  = (APTR)( (ULONG)buf->buf_adrL + buffersize ); /* consecutive buffers */
			buf->buf_size = buffersize;
			ADDTAIL( &ret->board_emptybufs, buf );
		}


		/* first: (re-)disable ints before we're adding the int server */
		board = ret->board_base;

		ret->chan_pair = 0;         /* pair of channels, i.e. first of the two consecutive indices (0...30) */
		ret->chan_mask = 1<<(ret->chan_pair);    /* occupied/active channels (0,1 here) */
		                                         /* note: only one interrupt enabled for channel pair */

		agusw_StopPlay( board, ret->chan_mask|(ret->chan_mask<<1) ); /* stop only occupied channels */
		board->GLOBAL_VOLUMEL = 0xffff;
		board->GLOBAL_VOLUMER = 0xffff;

		/* Interrupt server, Memory buffers */
		ret->board_int.is_Node.ln_Pri  = 126; /* we're important for real-time stuff */
		ret->board_int.is_Node.ln_Name = (char*)wot_name;
		ret->board_int.is_Node.ln_Type = NT_INTERRUPT;
		ret->board_int.is_Data = ret;
		ret->board_int.is_Code = (void (*)())AmiGUS_WTableOut_ISR;
		AddIntServer(INTB_PORTS, &ret->board_int ); /* INTB_EXTER = INT6, INTB_PORTS = INT2 */

		/* board memory allocation: since there is no central arbitration, just grab some */
		{
		 LONG sz = PER_INT_SEGMENT_SIZE;

		 ret->board_write_size = sz; /* in bytes */
		 ret->board_membufidx = 0;
		 ret->board_membufs[0][0] = 0;        /* start of board memory (32 MB) */
		 ret->board_membufs[0][1] = sz;       /*  */
		 ret->board_membufs[1][0] = sz+sz;    /*  */
		 ret->board_membufs[1][1] = sz+sz+sz; /*  */
		}


	}
	while(0);

	CloseLibrary( ExpansionBase );

	if( status != 0 )
	{
		agus_Exit_WTableOUT( ret );
		ret = (struct AmiGUS_WTableOut *)status;
		//Printf( (STRPTR)"Error: cannot init WAVOUT, exiting with status %ld\n",(LONG)status );
	}

	return ret;
}


/*
  start playback by playing first buffer (empty)

  two formats are supported: 8 bit signed or 16 bit signed (either big or little endian)

  rate <= 192000

*/
LONG agus_Start_WTableOUT( struct AmiGUS_WTableOut *wo, ULONG fmt, ULONG rate )
{
	LONG sample_rate;
	LONG chan_pair = wo->chan_pair;
	ULONG chanmask;
	struct AmiGUS_Hagen_Regs *board = wo->board_base;

	wo->board_voice_ctrl = HAGEN_CTRLF_16BIT|HAGEN_CTRLF_LOOP|HAGEN_CTRLF_INTERPOL|HAGEN_CTRLF_PBSTART;

	/* 1073741824/192000 = 5592.41 */
	if( rate > 192000 )
		sample_rate = 0x3FFFFFFF;
	else
		sample_rate = (1073741824L/192000L)*rate + ((rate*41L)/100L); /* avoids 32 bit overflow */

	wo->sample_rate = rate;
	wo->board_rate  = sample_rate;
	Printf( (STRPTR)"Chosen rate idx 0x%lx decimal %ld\n",sample_rate,rate);

	wo->board_membufidx = 0; /* next buffer to activate (see below) */

	/* fill first buffer, write zeroes if we don't have data yet */
	agusw_WriteBuffer( wo, wo->board_write_size, 1 );

	/* set channel parameters (rate, volume) */

	/* first channel is the "left" channel */
	board->VOICE_BNK = chan_pair;
	board->VOICE_VOLUMEL = 0xFFFF;
	board->VOICE_VOLUMER = 0x0000;
	board->VOICE_RATE    = sample_rate;

	/* second channel is the "right" channel */
	board->VOICE_BNK = chan_pair+1;
	board->VOICE_VOLUMEL = 0x0000;
	board->VOICE_VOLUMER = 0xFFFF;
	board->VOICE_RATE    = sample_rate;

	/* enable only those ints actually used */
	chanmask = wo->chan_mask;
	board->INTE0 = 0x8000|( chanmask      & 0xFF); /* able Voice Ints 0-7   */
	board->INTE1 = 0x8000|((chanmask>>8)  & 0xFF); /* able Voice Ints 8-15  */
	board->INTE2 = 0x8000|((chanmask>>16) & 0xFF); /* able Voice Ints 16-23 */
	board->INTE3 = 0x8000|((chanmask>>24) & 0xFF); /* able Voice Ints 24-31 */

	/* pointers, lengths, ctrl-words (actual start) */
	wo->board_membufidx = 1; /* next buffer to activate */
	agusw_StartBuffer( wo, 0, chan_pair );

#if 0
	/* TEST: CTRL outside of interrupt cycle (didn`t work) */
	board->VOICE_BNK = chan_pair+1;
	board->VOICE_CTRL  = wo->board_voice_ctrl;

	/* first channel is the "left" channel */
	board->VOICE_BNK = chan_pair;
	board->VOICE_CTRL  = wo->board_voice_ctrl; /* HAGEN_CTRLF_16BIT,HAGEN_CTRLF_LOOP,HAGEN_CTRLF_INTERPOL,HAGEN_CTRLF_PBSTART */
#endif	

	return 0;
}



ASM LONG direct_AmiGUS_WTableOut_ISR( ASMR(a1) struct AmiGUS_WTableOut *wo ASMREG(a1) )
{
	struct AmiGUS_Hagen_Regs *board;
	ULONG chan_mask;

	if( !wo )
		return 0;

	/* just acknowledge actually used IRQs (see chan_mask), 
	   return quickly if we don't see an IRQ for us
	*/
	board     = wo->board_base;
	chan_mask = wo->chan_mask; /* occupied/active channels */
	chan_mask |= chan_mask<<1; /* occupied/active channels */
	do
	{
		if( board->INTC0 & 0xFF )
			if( chan_mask & 0xff )
				break;
		chan_mask >>= 8;
		if( !chan_mask )
			return 0;

		if( board->INTC1 & 0xFF )
			if( chan_mask & 0xff )
				break;
		chan_mask >>= 8;
		if( !chan_mask )
			return 0;

		if( board->INTC2 & 0xFF )
			break;
		chan_mask >>= 8;
		if( !chan_mask )
			return 0;

		if( board->INTC3 & 0xFF )
			break;
		return 0;
	} while( 0 );

	wo->board_ints++;

	/* made it this far: have work to do */
	chan_mask = wo->chan_mask;
	chan_mask |= chan_mask<<1; /* occupied/active channels */

	/* only used IRQs are reset */
	board->INTC0 = chan_mask & 0xff;
	board->INTC1 = (chan_mask>>8) & 0xff;
	board->INTC2 = (chan_mask>>16) & 0xff;
	board->INTC3 = (chan_mask>>24) & 0xff;

	/* now swap buffers and re-enable playback for good measure */

	if( agusw_WriteBuffer( wo, wo->board_write_size, 1 ) <= 0 )
	{
		/* the current buffer index wo->board_membufidx was just filled, now announce it to the card */
		agusw_StartBuffer( wo, wo->board_membufidx, wo->chan_pair ); 

		wo->board_membufidx ^= 1; /* swap buffers */
	}

	return 0;
}

/* some glue to set flags correctly (i.e. independent of compiler code placement) */
/* 0x4E71 NOP
   0x4EB9 JSR <direct_func>
   0x4A80 TST.L D0
   0x4E75 RTS
*/
ULONG AmiGUS_WTableOut_ISR[3] = { 0x4E714EB9,(ULONG)direct_AmiGUS_WTableOut_ISR,0x4A804E75};


