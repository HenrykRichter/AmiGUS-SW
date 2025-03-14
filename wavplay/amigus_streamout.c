/*

  amigus_streamout.c

  (C) 2025 Henryk Richter, Oliver Achten

  Output a stream over the AmiGUS FIFO engine
 
*/
#include "amigus_streamout.h" /* amigus_registers.h, <exec/types.h> */

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


/*
  defined FIFO fill before playback

  inputs:
   addr    - bogus, pass NULL
   baseadd - AmiGUS board base address (Expansion 2782/16)
   size    - number of longs to write + 1
*/
void agus_initFIFO( APTR addr, APTR baseadd, LONG size )
{
    volatile ULONG *dptr_long = (volatile ULONG*)( ((UBYTE*)baseadd) + MAIN_FIFO_DATA);

    while( size-- >= 0 )
	*dptr_long = 0;

}

/*
  input:
   addr    - data source address
   baseadd - AmiGUS board base address (Expansion 2782/16)
   size    - number of longs to write
*/
void agus_writeFIFO( APTR addr, APTR baseadd, LONG size )
{
    volatile ULONG *dptr_long = (volatile ULONG*)( ((UBYTE*)baseadd) + MAIN_FIFO_DATA);
    ULONG *sptr_long = (ULONG*)addr;

    while( size-- )
	*dptr_long = *sptr_long++;

}

/* Register Access Funtions */
ULONG agus_ReadReg16(APTR base, ULONG offset)
{
	return *((UWORD *)((ULONG)base+offset));	
}


/* Register Access Funtions */
void agus_WriteReg16(APTR base, ULONG offset, UWORD val)
{
	*((UWORD *)((ULONG)base+offset)) = val;	
}

void agus_WriteReg32(APTR base, ULONG offset, ULONG val)
{
	*((ULONG *)((ULONG)base+offset)) = val;	
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
   struct AmiGUS_Wavout from agus_Init_WAVOUT()

  return:
   status code
*/
LONG agus_Exit_WAVOUT( struct AmiGUS_Wavout *wo )
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
	(ULONG)agus_ReadReg16(board_base,MAIN_INTC),
	(ULONG)agus_ReadReg16(board_base,MAIN_INTE),
	(ULONG)agus_ReadReg16(board_base,MAIN_FIFO_USE));

	agus_WriteReg16(board_base,MAIN_INTE,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS
	agus_initFIFO(NULL, wo->board_base, (6144>>2)-1); // Arm FIFO (with zeroes) to kick-off interrupt
	
	Printf("INTs INTC 0x%lx INTE 0x%lx FIFO %ld\n",
	(ULONG)agus_ReadReg16(board_base,MAIN_INTC),
	(ULONG)agus_ReadReg16(board_base,MAIN_INTE),
	(ULONG)agus_ReadReg16(board_base,MAIN_FIFO_USE));

	Printf("%ld underruns at %ld ints\n",(LONG)wo->board_underruns,wo->board_ints);
#endif


			/* disable WAV out */
			agus_WriteReg16(board_base,MAIN_SMPL_RATE,0x0);						// Disable playback	
			agus_WriteReg16(board_base,MAIN_INTC,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS	
			agus_WriteReg16(board_base,MAIN_INTE,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS
			agus_WriteReg16(board_base,MAIN_FIFO_RES,0x0);						// Reset FIFO state

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


extern ULONG AmiGUS_Wavout_ISR[3];
const char *wo_name = (char *)"AmiGUS WAVPlay";
/*
  find AmiGUS card in system and bind as driver to it

  in:
   flags      - flag mask (keep 0, for now)
   nbuffers   - number of buffers to allocate (see struct AmiGUS_Buffer)
   buffersize - number of bytes buffer space for each buffer

*/
struct AmiGUS_Wavout *agus_Init_WAVOUT( ULONG flags, ULONG nbuffers, ULONG buffersize )
{
	struct AmiGUS_Wavout *ret;
	struct Library *ExpansionBase;
	LONG status = 0;

	ExpansionBase = OpenLibrary( (STRPTR)"expansion.library", 36 ); /* 2.0+ */
	if( !ExpansionBase )
		return NULL;

	ret = AllocVec( sizeof( struct AmiGUS_Wavout ), MEMF_PUBLIC|MEMF_CLEAR );
	do
	{
		if( !ret )
			break;

		/* minimum TODO when "ret != NULL": lists and signal(s) */

		myNewMinList( &ret->board_fullbufs  );
		myNewMinList( &ret->board_emptybufs );

		ret->board_underruns = 0; /* yes, allocated with MEMF_CLEAR, just to clarify */
		ret->board_ints = 0;
		ret->board_waterfills = 0;

		ret->board_sigtask = FindTask(NULL);
		ret->board_sigbit = AllocSignal(-1);
		if( ret->board_sigbit == -1 )
		{
			status = -4;
			break;
		}


		/* main stuff */

		/* bind as driver: set a name in the node we're supposed to attach */
		ret->n.ln_Name = (char*)wo_name;

		ret->board_expansionstruct = FindConfigDev( NULL, AMIGUS_MANUFACTURER_ID, AMIGUS_MAIN_PRODUCT_ID );
		if( !ret->board_expansionstruct )
		{
			status = -5;
			break;
		}

		/* is the board good to use ? */
		ret->board_base = ret->board_expansionstruct->cd_BoardAddr;
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
			struct AmiGUS_Buffer *buf = AllocVec( sizeof( struct AmiGUS_Buffer ) + buffersize, MEMF_PUBLIC|MEMF_CLEAR );
			if( !buf )
			{
				status = -3;
				break;
			}
			buf->buf_adr  = (APTR)(buf+1);
			buf->buf_size = buffersize;
			ADDTAIL( &ret->board_emptybufs, buf );
#if 0			
			{ /* DEBUG */
			 ULONG *blub = (ULONG*)buf;
			 ULONG bla = blub[-1];
			 Printf( (STRPTR),"AllocVec Size %ld, buffersize %ld\n",bla,buffersize);
			}
#endif
		}


		/* first: (re-)disable ints before we're adding the int server */
		agus_WriteReg16(ret->board_base,MAIN_INTC,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS		
		agus_WriteReg16(ret->board_base,MAIN_INTE,INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_FULL|INT_FLG_FIFO_WTMK);	// Disable INTS

		/* Interrupt server, Memory buffers */
		ret->board_int.is_Node.ln_Pri  = 126; /* we're important for real-time stuff */
		ret->board_int.is_Node.ln_Name = (char*)wo_name;
		ret->board_int.is_Node.ln_Type = NT_INTERRUPT;
		ret->board_int.is_Data = ret;
		ret->board_int.is_Code = (void (*)())AmiGUS_Wavout_ISR;
		AddIntServer(INTB_PORTS, &ret->board_int ); /* INTB_EXTER = INT6, INTB_PORTS = INT2 */

		/* init registers */
		ret->board_watermark  = FIFO_WTMK; /* default: 2k WORDS */
		agus_WriteReg16(ret->board_base,MAIN_SMPL_RATE,0x0);			// Disable playback		
		agus_WriteReg16(ret->board_base,MAIN_FIFO_RES,0x0);			// Reset FIFO state
		agus_WriteReg16(ret->board_base,MAIN_FIFO_WTMK,ret->board_watermark);	// Set FIFO watermark
		agus_WriteReg16(ret->board_base,MAIN_INTE,INT_FLG_MASK_SET|INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_WTMK); // INT ENABLE

		ret->board_write_size = (FIFO_SIZE-FIFO_WTMK)*2; /* in bytes */

		/* good to go: last thing to do is to prime the FIFO */
	}
	while(0);

	CloseLibrary( ExpansionBase );

	if( status != 0 )
	{
		agus_Exit_WAVOUT( ret );
		ret = (struct AmiGUS_Wavout *)status;
		//Printf( (STRPTR)"Error: cannot init WAVOUT, exiting with status %ld\n",(LONG)status );
	}

	return ret;
}


/*
  start playback by priming FIFO (thus triggering interrupt)
*/
LONG agus_avail_rates[10][2] = {
{SMPL_RATE_8000	,8000 },
{SMPL_RATE_11025,11025},
{SMPL_RATE_16000,16000},
{SMPL_RATE_22050,22050},
{SMPL_RATE_24000,24000},
{SMPL_RATE_32000,32000},
{SMPL_RATE_44100,44100},
{SMPL_RATE_48000,48000},
{SMPL_RATE_96000,96000},
{SMPL_RATE_96000,0}}; /* end marker */


LONG agus_Start_WAVOUT( struct AmiGUS_Wavout *wo, ULONG fmt, ULONG rate )
{
	LONG bestidx = 0;
	LONG dst1,dst = 100000L;
	LONG idx=0;
	LONG sample_rate;
	

	while( agus_avail_rates[idx][1] != 0 )
	{
		if( agus_avail_rates[idx][1] > rate )
			dst1 = agus_avail_rates[idx][1] - rate;
		else	dst1 = rate - agus_avail_rates[idx][1];

		if( dst1 < dst )
		{
			dst = dst1;
			bestidx = idx;
		}
	
		idx++;
	}
	sample_rate = agus_avail_rates[bestidx][0];
//	Printf("Chosen rate idx %ld decimal %ld\n",sample_rate,agus_avail_rates[bestidx][1]);

	agus_initFIFO(NULL, wo->board_base, (8184>>2)-1); // Arm FIFO (with zeroes) to kick-off interrupt
	agus_WriteReg16(wo->board_base,MAIN_SMPL_FMT,fmt);			// Set sample rate		
	agus_WriteReg16(wo->board_base,MAIN_SMPL_RATE,(sample_rate|0xc000) );	// Start playback
	wo->sample_rate = (sample_rate|0xc000);
	
	return 0;
}



ASM LONG direct_AmiGUS_Wavout_ISR( ASMR(a1) struct AmiGUS_Wavout *wo ASMREG(a1) )
{
	struct AmiGUS_Main_Regs *board;
	struct AmiGUS_Buffer *buf;
	LONG ints,tmp,write_size;

	if( !wo )
		return 0;
	/* once we're live as interrupt, we at least need to assume a valid board base */
	board = (struct AmiGUS_Main_Regs *)wo->board_base;

	/* is the FIFO watermark IRQ or EMPTY IRQ on ? */
	ints = board->INTC; /* = agus_ReadReg16(board, MAIN_INTC ); */
	if( !(ints & (INT_FLG_FIFO_EMPTY|INT_FLG_FIFO_WTMK) ) )
		return 0; /* no, nothing to do */

	wo->board_ints++;

	if( ints & INT_FLG_FIFO_EMPTY )
	{
		wo->board_underruns++;
		Signal( wo->board_sigtask, 1<<wo->board_sigbit ); /* wake up our task */
	}

	/* this is tricky: we got an interrupt but might not have the ability to
	                   feed the FIFO if no "ready" buffer is available
	                   -> see below for waterfill emergency method
	*/
//	write_size  = wo->board_write_size; /* (FIFO_SIZE-FIFO_WTMK)*2; in bytes */
	write_size = ( (FIFO_SIZE - 16 - board->FIFO_USE)<<1 ) & 0xFFFC;
	while( (buf = GetHead( &wo->board_fullbufs ) ) )
	{
		LONG rem = buf->buf_length - buf->buf_pos; /* remaining in buffer */
		if( rem > 0 )
		{
			tmp  = ( rem > write_size ) ? write_size : rem;	/* number of bytes to copy from current buffer */

			agus_writeFIFO( (UBYTE*)buf->buf_adr + buf->buf_pos, board, tmp>>2 );

			buf->buf_pos += tmp; 				/* new pos in buffer */
			rem -= tmp; 					/* update remaining in buffer */
			write_size -= tmp; 				/* FIFO bytes still to be written */
		}
		if( rem <= 0 )
		{
		 REMOVE( buf );
		 ADDTAIL( &wo->board_emptybufs, buf );
		}
		if( write_size <= 0 )
			break;
	}

	/* clear watermark interrupt and underrun interrupt */
	board->INTC = (INT_FLG_FIFO_WTMK|INT_FLG_FIFO_EMPTY); /* agus_WriteReg16( board, MAIN_INTC, (INT_FLG_FIFO_WTMK|INT_FLG_FIFO_EMPTY) ); */

#if 1
	/* underrun check: if we couldn`t fill the FIFO over the water mark,
	   then dump zeros into the FIFO to keep the cycle running (with an unfortunate pause) */
	if( ints & INT_FLG_FIFO_EMPTY )
	{
		ULONG fu = board->FIFO_USE;
		if( fu <= wo->board_watermark )
		{
			wo->board_waterfills++;
			agus_initFIFO( NULL, board, (FIFO_SIZE - fu - 4)>>2 ); /* -4: step back and don`t overfill FIFO */
		}

		board->SMPL_RATE = wo->sample_rate; /* resume playback */
	}
#endif

	/* signal our task */
	Signal( wo->board_sigtask, 1<<wo->board_sigbit ); 

	return 0;
}

/* some glue to set flags correctly (i.e. independent of compiler code placement) */
/* 0x4E71 NOP
   0x4EB9 JSR <direct_func>
   0x4A80 TST.L D0
   0x4E75 RTS
*/
ULONG AmiGUS_Wavout_ISR[3] = { 0x4E714EB9,(ULONG)direct_AmiGUS_Wavout_ISR,0x4A804E75};


