
/* **************************************************************************

   Somewhat generic tracker renderer, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   This functionality assumes to have a Mod playing whose temporal increments
   yield in a number of played samples to be mixed. A software mixer will just
   take the current state of the replay and mix a number of samples conforming
   to the interval length between player ticks.

 ************************************************************************** */
#include "modplay.h"
#include "modrender_common.h"
#include "amigus_registers.h"

#include "types.h" /* redirects to exec/types.h on Amiga */

#include <proto/exec.h>
#include <proto/expansion.h>
#include <exec/nodes.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <libraries/configvars.h>
#include <stdio.h>

#define CLIP3( _a_, _min_, _max_ ) ((_a_) < (_min_) ) ? (_min_) : ( ((_a_) > (_max_) ) ? (_max_) : (_a_) )

#ifndef HAGEN_TIMER_TIMEBASE
#define HAGEN_TIMER_TIMEBASE 24576000
#endif


/* 
 * Options (as compile-time defines 
 * 
 */

#define BOARD_SMPL_ALIGN 4 /* align samples to 32 bit (this must be 2^n) */
#define BOARD_SMPL_PAD  32 /* padding between samples in AGUS memory (room for delay based fx) */
#define BOARD_END_OFFSET 2 /* end ptr of samples TODO: check if this applies to 8 Bit, too) */

/* allow for some mixing headroom: limit volume to -6dB, if defined */
#define VOLUME_LIMIT_HALF

/* debug: re-structure samples to support "start_of_sample" loops wrt. Firmware at and before 4-Jul */
/* FIXME: due to the "temporary" nature of this workaround, whole samples need to be passed to _loadsample() */
#define LOOP_WRKAROUND
//#undef LOOP_WRKAROUND

/* debug: don`t enable Interrupt if set */
#define DRY_RUN
#undef DRY_RUN

#ifdef DRY_RUN
#define D(_x_) do { printf _x_; } while(0)
#else
#define D(_x_)
#endif


/* re-define some fields in the sample structure */
#define SDRAM_LOC    ext_ptr1
#define SDRAM_ENDPTR ext_data1
#define SDRAM_REPPTR ext_data2
#define SDRAM_REPEND ext_data3
#define SDRAM_LOADBYTES ext_data4 


/* MODRender for AmiGUS */
struct MODRender {
	struct Node n;
	struct MOD *mod;         /* mod we're playing */
	struct ConfigDev *board_expansionstruct;
	struct AmiGUS_Hagen_Regs *board_base;

	LONG   board_sigbit;

	ULONG board_mem_pos; /* remember first free location in board RAM */
	ULONG smpl_pad;      /* padding (free space) between samples */

	ULONG rate;
	ULONG flags;	/* flags passed down from outside */

	/* play data */
	ULONG time_10us;
	struct Interrupt l2int;

	UBYTE playing;
	UBYTE unused[3];
};

ASM LONG modrender_amigus_inthandler( ASMR(a1) struct MODRender *rnd ASMREG(a1) );
extern ULONG modrender_amigus_ISR[3];
void modrender_amigus_updateregs(struct MODRender *rnd);


/* this is a player that won't return samples on "render" calls */
LONG modrender_is_player( struct MODRender *rnd )
{
	return 1;
}


/*
in: rnd  - renderer instance (after loading samples)
time - time frame in 10us units or number of samples to render (when negative)

out: number of new available samples from renderer

note: time should be reasonably small, not more than 0.5s (i.e. you'd need around 0.02s, anyway)
 */
LONG modrender_render( struct MODRender *mrnd, LONG time_10us, WORD *buf, LONG bufsize )
{
	/* we're a player, not a renderer */
#ifdef DRY_RUN
	modrender_amigus_inthandler( mrnd );
#endif
	return mrnd->time_10us;
}


/* set volume/balance by explicit left/right volume
 *
 * levels provided are 16 bit, min. 0, max. 65535
 *
 * */
LONG modrender_setvolume( struct MODRender *rnd, LONG lvolume, LONG rvolume )
{
	struct AmiGUS_Hagen_Regs *board;

	if( !(rnd) )
		return -1;

	lvolume = CLIP3( lvolume, 0, 65535 );
	rvolume = CLIP3( rvolume, 0, 65535 );

	if( lvolume < 0 )
		lvolume = 0;
	if( rvolume < 0 )
		rvolume = 0;
		
#ifdef VOLUME_LIMIT_HALF
	lvolume >>= 1;
	rvolume >>= 1;
#endif

	board = rnd->board_base;
	board->GLOBAL_VOLUMEL = lvolume;
	board->GLOBAL_VOLUMER = rvolume;

	return 0;
}


/* start playing */
LONG modrender_start_play( struct MODRender *rnd )
{
	struct AmiGUS_Hagen_Regs *board;
	UWORD  vol;

	if( !(rnd) )
		return -1;
	if( !(rnd->mod) )
		return -2;
	if( !(rnd->mod->samples ) )
		return -3;
	if( rnd->playing != 0 )
		return 1;	/* all good, we're already playing */

	board = rnd->board_base;

#ifdef VOLUME_LIMIT_HALF
	vol = 0x7fff;
#else
	vol = 0xFFFF;
#endif	
	board->GLOBAL_VOLUMEL = vol;
	board->GLOBAL_VOLUMER = vol;

	rnd->playing = 1;

#ifndef DRY_RUN
	rnd->l2int.is_Code = (void(*)(void))modrender_amigus_ISR; /* calls modrender_amigus_inthandler() */
	rnd->l2int.is_Data = rnd;
	rnd->l2int.is_Node.ln_Pri = 90; /* we're somewhat important :-) */
	rnd->l2int.is_Node.ln_Type = NT_INTERRUPT;
	AddIntServer( INTB_PORTS, &rnd->l2int ); 

	/* set up timer with defaults */
	board->HT_CONTROL = 0; /* clear timer first */
	board->INTC0 = 0x4000;

// ;Protracker Tempo: speed >32
// TimerVal = TimerBase / (Tempo*2/5) = TimerBase * 2.5 / Tempo
//
// time_s =2.5/TEMPO
//;time_us=2.5e6/TEMPO
//;32/16 divide compatible approach (divu.w):
//;time_10us=250000UL/(UWORD)TEMPO
//
// given time_10us:
// TimerVal = TimerBase * time_10us / 100000
// TimerVal = (TimerBase/1000) * time_10us / 100

	{
	 ULONG timerval = ((HAGEN_TIMER_TIMEBASE/1000) * rnd->time_10us )/100;
	 board->HT_RELOAD = timerval;
	 //printf("timer reload val %ld\n",timerval);
	}

	/* enable timer interrupt */
	board->INTE0 = 0xC000; /* enable HT interrupt */
	board->HT_CONTROL = 0x8000; /* enable timer */

#endif /* DRY_RUN */

	return 1;
}


/* stop/pause playing */
LONG modrender_stop_play( struct MODRender *rnd )
{
	struct AmiGUS_Hagen_Regs *board;

	if( !(rnd) )
		return -1;
	if( !(rnd->mod) )
		return -2;
	if( !(rnd->mod->samples ) )
		return -3;
	if( rnd->playing != 1 )
		return 0;

	board = rnd->board_base;

	/* disable timer first */
	board->HT_CONTROL = 0; /* disable timer */
	board->INTE0 = 0x4000; /* disable interrupt (clear timer bit) */
	board->INTE0 = 0;      /* extra measure (workaround): clear all bits if AmiGUS is still in "write" mode */
	board->INTC0 = 0x4000; /* clear request */

	/* crank up the volume */
	board->GLOBAL_VOLUMEL = 0x0;
	board->GLOBAL_VOLUMER = 0x0;

#ifndef DRY_RUN
	RemIntServer( INTB_PORTS, &rnd->l2int );
#endif
	rnd->playing = 0;

	return 0;
}


/* not called directly, see ISR below */
ASM LONG modrender_amigus_inthandler( ASMR(a1) struct MODRender *rnd ASMREG(a1) )
{
	struct MOD *mod = rnd->mod;
	ULONG time_10us;

#ifndef DRY_RUN
	struct AmiGUS_Hagen_Regs *board = rnd->board_base;

	/* TODO: Check if we got the right interrupt (board->INTC0 & 0x4000) */
	if( !(board->INTC0 & 0x4000) )
		return 0;
	board->INTC0 = 0x4000; /* clear request */
#endif

	time_10us = mod_playinterval( mod );
	if( time_10us != rnd->time_10us )
	{
		if( time_10us > 0 )
		{
		 ULONG timerval = ((HAGEN_TIMER_TIMEBASE/1000) * time_10us )/100;
		 rnd->time_10us = time_10us; 
#ifndef DRY_RUN
		 board->HT_RELOAD = timerval;
#endif
		}
	}

	/* update card registers */
	modrender_amigus_updateregs( rnd );

	return 0;
}

/* some glue to set flags correctly (i.e. independent of compiler code placement) */
/* 0x4E71 NOP
   0x4EB9 JSR <direct_func>
   0x4A80 TST.L D0
   0x4E75 RTS
*/
ULONG modrender_amigus_ISR[3] = { 0x4E714EB9,(ULONG)modrender_amigus_inthandler,0x4A804E75};



ULONG modrender_amigus_per2rate( ULONG per )
{
	/* 1073741824/192000 = 5592.40533 */
	//agus_rate = (1073741824L/192000L)*rate + ((rate*41L)/100L); /* avoids 32 bit overflow */
	// 3546895 Hz / Per = rate 
	//agus_rate = (1073741824/192000)*(3546895/Per)
	//          = 19835674514.773335/Per /* too large for 32 bit, hence 2-step calc */
	//
	//
	ULONG rate = 3546895UL/per;

//	printf("per %ld",(LONG)per);

	per  = rate*5592; /* accurate formula would be 5592.41, i.e. +rate*41/100 or +(rate*104)>>8 */
//	per += (rate*104)>>8; /* remainder: rate*0.406 */

//	printf("agrate %ld",(LONG)per);

	return per;
}

void modrender_amigus_updateregs(struct MODRender *rnd)
{
   struct MOD *mod = rnd->mod;
   LONG nchan = mod->nchannels;
   LONG i,idx;
   ULONG l,ctrl,per;
   struct MODChannel **channels,*chn;
   struct MODSample *smp;
   struct AmiGUS_Hagen_Regs *board = rnd->board_base;
   struct MODSample **ms = mod->samples;

   /* browse through channels quickly, i.e. just act when there's something new */
   channels = mod->channels;
   for( i = 0 ; i < nchan ; i++ )
   {
	chn = *channels++;
	/* skip considerations when nothing happened on this channel */
	while( chn->flags & (MODCHF_PERCHANGE|MODCHF_VOLCHANGE|MODCHF_SMPLCHANGE) )
	{
		board->VOICE_BNK = i; /* select current channel registers */

		if( chn->delaysample >= 0 )
		{
			idx  = chn->delaysample;
			smp  = ms[idx];
			ctrl = board->VOICE_CTRL|HAGEN_CTRLF_LOOP|HAGEN_CTRLF_PBSTART;
			chn->delaysample = -1;

			if( smp )
			{
#ifdef LOOP_WRKAROUND
				board->VOICE_CTRL = 0; /* FIXME: this completely destroys the idea of Delayed Samples (AGUS limitation) */
				board->VOICE_PSTRT = (ULONG)smp->SDRAM_REPPTR;

				board->VOICE_PLOOP = (ULONG)smp->SDRAM_REPPTR;
				board->VOICE_PEND  = smp->SDRAM_REPEND;
				board->VOICE_CTRL  = ctrl; /* ensure we`re playing */
#else /* LOOP_WRKAROUND */
				/* delaysample uses the repeat part */
				board->VOICE_CTRL  = ctrl; /* ensure we`re playing */	
				board->VOICE_PLOOP = smp->SDRAM_REPPTR;
				board->VOICE_PEND  = smp->SDRAM_REPEND;
				D(("V %1ld DelaySample strt %ld end %ld\n",i,smp->SDRAM_REPPTR,smp->SDRAM_REPEND));
#endif /* LOOP_WRKAROUND */
			}
		}
		else
		{
		if( chn->smp_note >= 0 )
		{
			idx = chn->smp_note;
			smp = ms[idx];
			
			D(("ctrl %1ld\n",i));
			board->VOICE_CTRL = 0; /* stop playing */
	
			if( smp )
			{
				D(("v0 %1ld strt %ld ",i,(ULONG)smp->SDRAM_LOC));
#ifdef LOOP_WRKAROUND
				board->VOICE_PSTRT = (ULONG)smp->SDRAM_LOC;
				board->VOICE_PLOOP = (ULONG)smp->SDRAM_REPPTR; /* this will be padded towards end of sample if smp->repeat == 0 */
				board->VOICE_PEND  = (ULONG)smp->SDRAM_ENDPTR;

				ctrl = HAGEN_CTRLF_PBSTART|HAGEN_CTRLF_INTERPOL;
				if( smp->replen > 2 )
					ctrl += HAGEN_CTRLF_LOOP;

				board->VOICE_VOLUMEL = (chn->volume * chn->lvolume)<<2;
				board->VOICE_VOLUMER = (chn->volume * chn->rvolume)<<2;
				board->VOICE_RATE = modrender_amigus_per2rate( chn->period );

				board->VOICE_CTRL  = ctrl;
#else /* LOOP_WRKAROUND */
				board->VOICE_PSTRT = (ULONG)smp->SDRAM_LOC;
				board->VOICE_PLOOP = (ULONG)smp->SDRAM_LOC;
				ctrl = HAGEN_CTRLF_PBSTART; // |HAGEN_CTRLF_INTERPOL

				l = smp->length;
				if( MODSAM_IS_16BIT( smp->type ) )
				{
					l+=l;
					ctrl += HAGEN_CTRLF_16BIT;
				}
				board->VOICE_PEND = (ULONG)smp->SDRAM_LOC + l;
		 		D(("end %ld",(ULONG)smp->SDRAM_LOC + l));
				
				board->VOICE_VOLUMEL = (chn->volume * chn->lvolume)<<2;
				board->VOICE_VOLUMER = (chn->volume * chn->rvolume)<<2;
				board->VOICE_RATE = modrender_amigus_per2rate( chn->period );

//		 D(("v0 %1ld %ld %ld %ld\n",i,chn->lvolume,chn->rvolume,chn->volume));

				if( chn->smp_note != chn->smp_repeat ) /* reload sample parameters if different repeat sample (rare) */
				{
					if( chn->smp_repeat >= 0 )
					{
						if( ms[chn->smp_repeat] )
							smp = ms[chn->smp_repeat];
					}
				}
				if( smp->SDRAM_REPPTR == smp->SDRAM_REPEND ) /* no loop ? */
				{
				 D(("ctrlB %1ld\n",i));
				 board->VOICE_CTRL  = ctrl;
				}
				else
				{
				 D(("ctrlC %1ld\n",i));
				 board->VOICE_CTRL  = ctrl + HAGEN_CTRLF_LOOP; /* start playback */

				 board->VOICE_PLOOP = smp->SDRAM_REPPTR;
				 board->VOICE_PEND  = smp->SDRAM_REPEND;
				}
			 D((" loop %ld pend %ld \n",(ULONG)smp->SDRAM_REPPTR,(ULONG)smp->SDRAM_REPEND));
#endif /* LOOP_WRKAROUND */
			}
		 break; /* all done */
		}
		} /* delaysample */

		/* update both volume and period */
		if( chn->flags & MODCHF_VOLCHANGE )
		{
//		 D(("v1 %1ld %ld %ld %ld\n",i,chn->lvolume,chn->rvolume,chn->volume));
		 board->VOICE_VOLUMEL = (chn->volume * chn->lvolume)<<2;
		 board->VOICE_VOLUMER = (chn->volume * chn->rvolume)<<2;
		}

		per = chn->period;
		if( (chn->flags & MODCHF_PERCHANGE) && (per) )
		{
			board->VOICE_RATE = modrender_amigus_per2rate( per );
		}
	 break;
	}

   }



}



/* init renderer for a mod 
in:  mod     - struct MOD (after mod_init() )
rate    - sampling rate of the renderer
flags   - init flags
out: 

the returned renderer is prepared for the actual number of channels
and will contain placeholders for the samples to be loaded. Please load
the Mod's samples after the init step.
 */
struct MODRender *modrender_init( struct MOD *mod, LONG rate, ULONG flags )
{
	struct Library *ExpansionBase;
	struct MODRender *mrnd = NULL;
	struct ConfigDev *board_expansionstruct;
	struct AmiGUS_Hagen_Regs *board;
	LONG   i,status;

	if( !mod )
		return mrnd;

	/* check if we actually have an AmiGUS */
	ExpansionBase = OpenLibrary( (STRPTR)"expansion.library", 34 ); /* 1.3+ */
	if( !ExpansionBase ) /* shouldn't happen */
		return mrnd;

	board_expansionstruct = FindConfigDev( NULL, AMIGUS_MANUFACTURER_ID, AMIGUS_HAGEN_PRODUCT_ID );

	CloseLibrary( ExpansionBase );

	if( !board_expansionstruct )
		return mrnd; /* card not found */
	if( board_expansionstruct->cd_Flags & (CDF_SHUTUP|CDF_BADMEMORY) )
		return mrnd; /* board unusable */

	mrnd = (struct MODRender *)mod_AllocVec( sizeof( struct MODRender ) );
	if( !mrnd )
		return mrnd;
	
	mrnd->board_sigbit = -1; /* required: -1 is checked in free() call */

	mrnd->board_expansionstruct = board_expansionstruct;
	mrnd->n.ln_Name = (char*)"AmiGUS Modplay";

	mrnd->mod   = mod;
	mrnd->rate  = rate;
	mrnd->flags = flags;
	mrnd->playing = 0;
	/* see below mrnd->time_10us = 0; */

	/* see if anybody is there already, if not then bind driver */
	/* reason why now: as soon as we start loading samples to the
	 *                 board, we can't allow others to interfere
	 */
	status = 0;
	Forbid();
	if( mrnd->board_expansionstruct->cd_Driver == NULL )
		mrnd->board_expansionstruct->cd_Driver = &mrnd->n; /* we're a driver ! */
	else
		status = -2;
	Permit();

	/* board was occupied? */
	if( status != 0 )
	{
		modrender_free( mrnd );
		return NULL;
	}

	board = (struct AmiGUS_Hagen_Regs *)board_expansionstruct->cd_BoardAddr;
	mrnd->board_base = board;
	mrnd->smpl_pad = BOARD_SMPL_PAD; 
	D(("board base 0x%lx\n",(ULONG)mrnd->board_base));

	for(i = 0 ; i < 32 ; i++ )
	{
		board->VOICE_BNK = i;
		board->VOICE_CTRL = 0; /* stop play */
	}

	/* clear out interrupts */
	Disable();
	board->INTE0 = 0xff;
	board->INTE1 = 0xff;
	board->INTE2 = 0xff;
	board->INTE3 = 0xff;
	board->INTE0 = 0x00; /* workaround for early firmware revisions: INTE was supposed to be SET/CLR but acted as R/W */
	board->INTE1 = 0x00;
	board->INTE2 = 0x00;
	board->INTE3 = 0x00;
	board->INTC0 = 0xff; /* clear out all pending interrupts */
	board->INTC1 = 0xff;
	board->INTC2 = 0xff;
	board->INTC3 = 0xff;
	Enable();



	mrnd->board_mem_pos = 0; /* remember first free location in board RAM */
	mrnd->smpl_pad = 0;      /* padding (free space) between samples */

	/* preprocess samples: compute needed bytes on board and assign locations
	                       in board memory
	*/
	{
	 struct MODSample **ms = mod->samples;
	 struct MODSample *smp;
	 LONG ns = mod->nsamples,l,rs,rl,pad_l;
	 LONG idx = -1;
	 ULONG  mempos = mrnd->smpl_pad;

	 while( ns )
	 {
		smp = *ms++;
		idx++;
		if( !(smp) )
			continue;
		ns--;

		smp->SDRAM_LOC = (APTR)mempos;
		smp->ext_ptr2  = NULL;

		/* samples are in "nsamples" units, i.e. calculate *2 for 16 bit to get nbytes */
		l = smp->length;
		rs= smp->repstart;
		rl= smp->replen;
		if( MODSAM_IS_16BIT( smp->type ) )
		{
			l += l;
			rs+= rs;
			rl+= rl;
		}
		pad_l = l;
		/* if( rl <= 2 ) rl = 0 */

		D(("s%1ld l%ld rs%ld rl%ld -> ",(LONG)idx,(LONG)l,(LONG)rs,(LONG)rl));
#ifdef LOOP_WRKAROUND
		if( rl ) 
		{
			/* PT has two cases for loops (replen>2 bytes): 
			 * a) rep_start > smpl_start
			 *    in this case, the initial part plays until rep_start, then the repeat
			 *    kicks in, regardless of the total sample bytes
			 * b) rep_start == smpl_start
			 *    in this case, the whole sample is played, followed by the specified
			 *    repeat from the beginning of the sample (typically shorter than whole len)
			 *    This case is not supported by early AmiGUS firmware versions.
			 * note: technically, PT always loops, even with replen==2 bytes
			 */
			if( rs > 0 )
			{ /* case a), regular repeats */
				l = rs+rl; /* restrict length to repeat area */
			}
			else
			{ /* case b), special repeats with repeat_start==0 -> apply padding */
			  if( (rs+rl) < l )
			  {
				pad_l = l + rl; /* reserve more space */
				rs = l; /* put repeat behind initial sample */
				l += rl;
				smp->ext_ptr2  = (APTR)rs;
			  }
			}
		}
#endif
		smp->SDRAM_ENDPTR = mempos+l-BOARD_END_OFFSET; /* end ptr */
		smp->SDRAM_REPPTR = mempos+rs;  /* repeat ptr */
		smp->SDRAM_REPEND = mempos+rs+rl-BOARD_END_OFFSET;
		smp->SDRAM_LOADBYTES = 0; /* nothing loaded yet (byte units) */

		D(("ep 0x%lx ed1 %ld ed2 %ld ed3 %ld\n",(LONG)smp->SDRAM_LOC,(LONG)smp->SDRAM_ENDPTR,(LONG)smp->SDRAM_REPPTR,(LONG)smp->SDRAM_REPEND));

		/* next sample in board memory */
		mempos += pad_l;

		mempos += mrnd->smpl_pad; /* padding */
		mempos  = (mempos+(BOARD_SMPL_ALIGN-1)) & ~(BOARD_SMPL_ALIGN-1); /* alignment */

		/* TODO: 16 bit support */
		/* right now, S8/U8 are parsed (and converted) in sample load routine */
	 }
	}

	mrnd->time_10us = 2000; /* 20ms * 100 = 50 Hz default */

	/* TODO: check if number of channels in the mod and their panning is actually supported */

	return mrnd;
}

/*
   Pass the loaded samples to the renderer

out: number of bytes left to be loaded for current sample

This call may receive partial data, i.e. you may feed the whole data set
or just a part.

SDRAM_REPEND = loaded samples in "sample" units (i.e. number of bytes or words)
 */
LONG modrender_loadsample( struct MODRender *rnd, LONG idx, UBYTE *buf, LONG bytes )
{
	struct MODSample *smp;
	struct AmiGUS_Hagen_Regs *board;
	LONG   dsamples;
	ULONG  dptr;
	ULONG  *lbuf = (ULONG*)buf;
	ULONG  eorval = 0; /* conversion signed/unsigned */

	/* TODO: proper error codes */
	if( !(rnd) || !(buf) || !(bytes) )
		return -1;
	if( !(rnd->mod) )
		return -2;
	if( !(rnd->mod->samples ) )
		return -3;
	if( (idx < 0) || (idx >= rnd->mod->maxsamples ) )
		return -4;

	smp = rnd->mod->samples[idx];
	if( !(smp) )
		return -5;

	board = rnd->board_base;

	dptr  = (ULONG)smp->SDRAM_LOC;
	dptr += smp->SDRAM_LOADBYTES;

	board->WADDR = dptr;

#ifdef LOOP_WRKAROUND
	if( bytes > (smp->length-smp->SDRAM_LOADBYTES) )
		bytes = (smp->length-smp->SDRAM_LOADBYTES);
#else
	if( bytes > (smp->SDRAM_ENDPTR + BOARD_END_OFFSET - dptr) )
		bytes = (smp->SDRAM_ENDPTR + BOARD_END_OFFSET - dptr);
#endif
	smp->SDRAM_LOADBYTES += bytes;

	dsamples = (bytes+3)>>2; /* convert to 32 bit units */

	//printf("idx %ld len %ld dsam %ld dp 0x%lx\n",(LONG)idx,(ULONG)smp->length,(ULONG)(dsamples<<2),(LONG)dptr);

	switch( smp->type )
	{
		case MODSAM_TYPE_U8:
			eorval = 0x80808080;
			// fall through
		case MODSAM_TYPE_S8:
			while(dsamples--)
				board->WDATA = *lbuf++ ^ eorval;
			break;
		default:
			return -7;
			break;
	}

#ifdef LOOP_WRKAROUND
	if( smp->ext_ptr2 != NULL )
	{
		dptr  = (ULONG)smp->SDRAM_REPPTR;
		bytes = smp->replen; 		/* FIXME: not 16 bit compatible */
		lbuf  = (ULONG*)(buf + smp->repstart);    /* repstart == 0 */

		board->WADDR = dptr;

		dsamples = (bytes+3)>>2;
		while( dsamples-- )
			board->WDATA = *lbuf++ ^ eorval;
	}
#endif

	return (LONG)(smp->length - smp->SDRAM_LOADBYTES); /* FIXME: not 16 bit compatible */
}


#if 0
{
	LONG nsamples;
	LONG i,nchannels,per,rendered_samples,suml,sumr,cur;
	ULONG stp;
	LONG  spos13,spos,vscale;
	struct MOD *mod;
	struct MODSample **samples,*smp;
	struct MODChannel *chn,**channels;

	if( !mrnd )
		return 0;
	if( !mrnd->mod )
		return 0;

	mod = mrnd->mod;

	/* number of mixed samples */
	if( time_10us >= 0 )
	{
		if( !time_10us )
			return 0;
		nsamples = (mrnd->rate * time_10us)/100000;
	}
	else
		nsamples = -time_10us;

	if( nsamples < 3 )
		return 0;

	/* we need to fit nsamples*2 for stereo into the output buffer */
	if( bufsize < (nsamples+nsamples) )
		return -1;

	samples  = mod->samples;
	nchannels= mod->nchannels;
	channels = mod->channels;
	/* preparation: period calculations (if necessary) */
	for( i = 0 ; i < nchannels ; i++ )
	{
		chn = channels[i];
		per = chn->period;
		if( (per != 0) && (per != chn->rnd_per) )
		{
			chn->rnd_per     = per;
			/* careful to stay in 32 bit range */
			stp = (3546895UL*1024UL)/(ULONG)(per*mrnd->rate);
			chn->rnd_perstep = stp<<3; /* step_per_sample<<13 */
		}
	}

	vscale = 256/(nchannels>>1);

	/* render loop */
	rendered_samples = nsamples;
	while( nsamples-- )
	{
		suml = 0;
		sumr = 0;
		for( i = 0 ; i < nchannels ; i++ )
		{
			chn    = channels[i];
			spos13 = chn->sample_pos13;
			if( spos13 >= 0 )
			{
				BYTE *dta; 

				smp  = samples[chn->sample];
				if( !smp )
				{
					chn->sample_pos13 = -1;
					continue;
				}
				if( spos13 >= smp->SDRAM_ENDPTR ) /* beyond length (either one-shot or repeat) */
				{
					if( smp->replen > 2 )
					{
						/* beyond repeat */
						spos13 -= (smp->replen<<13); 
						if( chn->delaysample >= 0 )
							chn->sample = chn->delaysample;
						chn->delaysample = -1;
					}
					else
					{ /* no replen, stop playing */
						chn->sample_pos13 = -1;
						continue;
					}
				}
				chn->sample_pos13 = spos13+chn->rnd_perstep;
				spos = spos13>>13;
				dta  = smp->data;
				cur  = (chn->volume * (dta[spos]<<8))>>6; /* TODO: 16 bit samples */

				suml += cur*chn->lvolume;
				sumr += cur*chn->rvolume;


			} /* spos13 */


		} /* for( i = 0 ; i < nchannels ; i++ ) */

		/* normalize suml, sumr */
		suml = (suml*vscale)>>16;
		sumr = (sumr*vscale)>>16;
#if 1
		/* clamping */
		if( suml > 32767 )
			suml = 32767;
		if( suml < -32768 )
		suml = -32768;
	if( sumr > 32767 )
		sumr = 32767;
	if( sumr < -32768 )
		sumr = -32768;
#endif
	*buf++ = suml;
	*buf++ = sumr;

  } /* while(nsamples--) */

  return rendered_samples;
}
#endif

/*
  deallocate, free resources
*/
LONG modrender_free( struct MODRender *rnd )
{

 if( !rnd )
 	return 0;
 if( !rnd->mod )
 	return 0;
 if( !rnd->mod->samples )
 	return 0;

 if( rnd->board_expansionstruct )
 {
	modrender_stop_play( rnd ); /* stop playing (if so) and clear out registers */

	if( rnd->board_expansionstruct->cd_Driver == &rnd->n )
	{
		struct AmiGUS_Hagen_Regs *board = rnd->board_base;
		int i;

		for(i = 0 ; i < 32 ; i++ )
		{
			board->VOICE_BNK = i;
			board->VOICE_CTRL = 0; /* stop play */
		}

		Forbid(); /* this is probably overkill */
		rnd->board_expansionstruct->cd_Driver = NULL;
		Permit();
	}
 }

 if( rnd->board_sigbit != -1 )
	FreeSignal( rnd->board_sigbit );

 mod_FreeVec( rnd );

 return 0;
}




