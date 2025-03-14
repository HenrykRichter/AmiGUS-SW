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
#include "amigus_streamout.h"
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
	LONG i,idx,playmode=PLAYMODE_16B;
//	LONG blah;
#ifdef USE_AMIGADOS
	struct AmiGUS_Wavout *wo = NULL;
	ULONG waitmsk,sigs;
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
		if( !(my_strnicmp( argv[i], "-24", 3 )) )
		{
			playmode = PLAYMODE_24B;
			continue;
		}
		if( !(my_strnicmp( argv[i], "-auto", 5 )) )
		{
			playmode = PLAYMODE_AUTO;
			continue;
		}
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
		wo = agus_Init_WAVOUT( 0, 4, 32768 ); /* */
		if( (LONG)wo <= 0 ) /* CAUTION: must be >0 */
		{
			printf("Cannot find and/or obtain AmiGUS card, status %ld\n",(LONG)wo );
			break;
		}
		
		if( playmode == PLAYMODE_24B )
		 Printf( (STRPTR)"AmiGUS in 24 Bit mode\n");

	
		waitmsk = SIGBREAKF_CTRL_C|(1<<wo->board_sigbit);
		while(1)
		{
#if 1
			struct AmiGUS_Buffer *buf = GetHead( (struct List*)&wo->board_emptybufs );
			if( buf )
			{
			  LONG sz;

			  if( playmode == PLAYMODE_24B )
			  {
			  	sz = GetBuffer24Stereo_Wav( wav, buf->buf_adr, buf->buf_size );
			  }
			  else
			  	sz = GetBuffer16Stereo_Wav( wav, buf->buf_adr, buf->buf_size );

			  if( !sz )
			  {
			  	Printf( (STRPTR)"End of WAV\n");
			  	break;
			  }
			  
			  buf->buf_pos = 0;
			  buf->buf_length = sz;
#if 0
			  if( !	blah )
			  {
				  Printf("sz %ld\n",sz);
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
				/* all buffers filled, check if we need to start */
				if( agus_ReadReg16(wo->board_base,MAIN_FIFO_USE) == 0 )
				{
					/* now: we have a WAV, we have an AmiGUS, just start playing */
					/* formats: SMPL_FMT_LITTLE_ENDIAN|SMPL_FMT_STEREO_16BIT     */
					LONG fmt = ( playmode == PLAYMODE_24B ) ? SMPL_FMT_STEREO_24BIT : SMPL_FMT_STEREO_16BIT;
					agus_Start_WAVOUT( wo, fmt, wav->rate );
				}
			
				sigs = Wait( waitmsk );
				if( sigs & SIGBREAKF_CTRL_C )
					break;
			}
		}
	}
	while(0);

	if( wo )
	{
		if( wo->board_underruns > 0 )
		{
			Printf( (STRPTR)"%lu FIFO underruns at %lu total interrupts served, %lu waterfills\n",
		       	wo->board_underruns,wo->board_ints,wo->board_waterfills );
		}
		agus_Exit_WAVOUT( wo );
	}

#else /* USE_AMIGADOS */
	}
	while(0);
#endif /* USE_AMIGADOS */

	Close_Wav( wav );

	return 0;
}
