/*

  main.c

  (C) 2025 Henryk Richter

  test application for WAV loader

*/
#include "fileio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef USE_AMIGADOS
#include "amigus_wtableout.h"
#endif
#include "stringutils.h"

#ifndef EXEC_RESIDENT_H
#include <stdint.h>
typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int16_t WORD;
typedef int32_t LONG;
#endif

#include "macros.h"
#include "wav_load.h"

/* internal */
#define PLAYMODE_16B  0
#define PLAYMODE_24B  1
#define PLAYMODE_AUTO 2


int main( int argc, char **argv )
{
	struct wavload *wav;
	LONG i,idx,started,blah=0;
#ifdef USE_AMIGADOS
	struct AmiGUS_WTableOut *wo = NULL;
	ULONG waitmsk,sigs,totlen=0;
#endif

	if( 1 )
	{
		struct AmiGUS_Hagen_Regs *rgs = NULL;
		UWORD *bla = (UWORD*)&rgs->HT_CONTROL;
		ULONG a = (ULONG)rgs,b=(ULONG)bla;

		if( (b-a) != 0xF0 )
		{
			printf("Struct problem in \"struct AmiGUS_Hagen_Regs\",\n HT_CONTROL should be 0xF0 but is 0x%lx\n",(long)(b-a));
		}

		if( 0 )
		{
			struct  AmiGUS_Hagen_Regs *rgs = NULL;
/*			UWORD   *blub1 = &rgs->VOICE_PLOOPH;
			UWORD   *blub2 = &rgs->VOICE_PLOOPL;
			ULONG   *blab  = &rgs->VOICE_PLOOP;*/
			UWORD   *blub1 = (UWORD*)&rgs->VOICE_RATEHL[0];
			UWORD   *blub2 = (UWORD*)&rgs->VOICE_RATEHL[1];
			ULONG   *blab  = (ULONG*)&rgs->VOICE_RATE;

			ULONG   a=(ULONG)blub1,b=(ULONG)blub2,c=(ULONG)blab;

			printf("0x%lx 0x%lx 0x%lx\n",a,b,c);
		}

	}

	if( argc < 2 )
	{
		printf("usage: %s file_path\n\n",argv[0]);
		return 1;
	}
	
	idx = -1;
	for( i=1 ; i < (LONG)argc ; i++ )
	{
#if 0
		if( !(my_strnicmp( argv[i], "AUTOPACK", 8 )) )
		{
			do_autopack=1;
			continue;
		}
		if( !(my_strnicmp( argv[i], "OUTPUT", 4 )) )
		{
			if( (i+1) >= argc )
			{
				printf("ERROR: OUTPUT requires an argument\n");
				break;
			}
			oname = argv[i+1];
			i++;
			continue;
		}
#endif
		idx = i;
	}

	if( idx < 0 )
	{
		printf("Error: requires file name\n");
		return 2;
	}

	do
	{
		wav = Open_Wav( argv[idx] );
		if( !wav )
		{
			printf("Error: cannot open wav file\n");
			break;
		}
#ifdef USE_AMIGADOS
		wo = agus_Init_WTableOUT( 0, 4, 32768 );
		if( (LONG)wo <= 0 ) /* CAUTION: must be >0 */
		{
			printf("Cannot find and/or obtain AmiGUS Wavetable card, status %ld\n",(LONG)wo );
			break;
		}

		started = 0;
		waitmsk = SIGBREAKF_CTRL_C|(1<<wo->board_sigbit);
		while(1)
		{
#if 1
			struct AmiGUS_WBuffer *buf = GetHead( (struct List*)&wo->board_emptybufs );
			if( buf )
			{
			  LONG sz;

			  sz = GetBuffer16_Wav( wav, buf->buf_adrL, buf->buf_adrR, buf->buf_size );

			  if( !sz )
			  {
			  	Printf( (STRPTR)"End of WAV\n");
			  	break;
			  }
			  
			  buf->buf_pos = 0;
			  buf->buf_length = sz;
			  totlen += sz;
#if 1
			  if( !	blah )
			  {
				  Printf( (STRPTR)"sz %ld\n",sz);
				  blah = 1;
			  }
#endif			  
			  Disable();
			   REMOVE(buf);
			   ADDTAIL( (struct List*)&wo->board_fullbufs, (struct Node*)buf );
			  Enable();
			  
			}
			else
#endif
			{
#if 1
				/* all buffers filled, check if we need to start */
				if( !started )
				{
//					wav->rate = 44100; /* DEBUG only */
					agus_Start_WTableOUT( wo, SMPL_FMT_STEREO_16BIT, wav->rate );
					started = 1;
				}
#endif	
				sigs = Wait( waitmsk );
				if( sigs & SIGBREAKF_CTRL_C )
					break;
			}
		}
	}
	while(0);

#if 1
	if( wo )
	{
		printf("Played %ld bytes (16 Bit)\n",totlen);

//		if( wo->board_underruns > 0 )
		{
			Printf( (STRPTR)"%lu FIFO underruns at %lu total interrupts served\n",
		       	wo->board_underruns,wo->board_ints );
		}
		agus_Exit_WTableOUT( wo );
	}
#endif

#else /* USE_AMIGADOS */
	}
	while(0);
#endif /* USE_AMIGADOS */

	Close_Wav( wav );

	return 0;
}
