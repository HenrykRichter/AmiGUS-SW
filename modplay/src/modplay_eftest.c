/* **************************************************************************

   Effect and playback tester for modplay.
   (C) 2024 Henryk Richter

   This piece of code calls the ProTracker player (PTPlay3.0B) and
   modplay simultaneously and compares the state of both player
   engines wrt. volumes, periods, played samples etc.

   The main idea is to verify and debug playback order and effect handling.

   ************************************************************************** */
#include "modplay.h"
#include "modrender_common.h"
#include "fileio.h"
#include <stdio.h>
#include "PTPlay30B.h"

FH_T Wavwrite_Open( STRPTR fname, LONG rate, LONG nchannels );
LONG Wavwrite_Write( FH_T file, UBYTE *buf, LONG bytes );   /* write a block of bytes (assuming LE endianess) */
LONG Wavwrite_Write16LE(  FH_T wf, WORD *buf, LONG words ); /* write shorts in native endianess as LE */
void Wavwrite_Close( FH_T file );

void print_pat_row( UBYTE *pt_patdta );
unsigned long per2note( unsigned long per );

int main(int argc, char **argv )
{
  FH_T ifile;
  FH_T wavfile;
  long filesize = 0;
  unsigned char *buf = NULL;
  unsigned char *obuf;// = NULL;
  struct MOD *mod;
  struct MODRender *mrnd;
  LONG srate = 44100;
  LONG obs;

  ifile = FILE_OPEN_R( argv[1] );
  if( ifile )
  {
	FILE_SEEK_END( 0, ifile );
	filesize = FILE_SEEK_TELL( ifile );
	FILE_SEEK_START(0, ifile );

	printf("file is %ld bytes long\n",filesize);
	
	buf = (unsigned char*)MALLOC( filesize );
	if( !buf )
	{
		printf("error: cannot allocate buffer\n");
		FILE_CLOSE(ifile);
		return -1;
	}
	filesize = FILE_READ( buf, filesize, ifile );
	FILE_CLOSE( ifile );
  }
  else
  {
    printf("error: cannot open %s\n",argv[1] );
  }

  if( (!filesize) || (!buf) )
  	return -2;

  do
  {
   LONG mod_songsz,mod_sampleoff=0;
   mod_songsz = mod_check( buf, filesize );
   printf("Mod song size %ld\n",(long)mod_songsz);

   if( mod_songsz <= 0 )
   	break;

   mod = mod_init( buf, filesize, 0, &mod_sampleoff );
   printf("Mod sample offset %ld\n",(long)mod_sampleoff);
  
   if( mod_sampleoff <= 0 )
   	break;

   if( mod->nchannels != 4 )
   {
	printf("%ld channels: not a 4ch Protracker mod. Quitting.\n",(long)mod->nchannels);
	break;
   }

#if 1
   {
    struct MODSample **ms;
    struct MODSample *smp;
    LONG ns,idx,mod_samplesize;

    /* debug: go through samples */
    mod_samplesize = 0;
    ns = mod->nsamples;
    ms = mod->samples;
    idx= -1;
    while( ns )
    {
   	idx++;
	smp = *ms++;
	if( !smp )
		continue;
	ns--;
	printf("Sample %2ld len %6lx repstart %6lx replen %6lx name %s\n",(long)idx,(long)smp->length,(long)smp->repstart,(long)smp->replen,smp->name);
	mod_samplesize += smp->length;
    }

    printf("song size %ld sample size %ld total %ld\n",(long)mod_sampleoff,(long)mod_samplesize,(long)(mod_sampleoff+mod_samplesize));
    printf("%ld channels\n",(long)mod->nchannels);
   }
#endif

   /* now load samples */
   mrnd = modrender_init( mod, srate, 0 );
   if( !mrnd )
   {
   	printf("cannot obtain a renderer instance, aborting...\n");
   	mod_free( mod );
	break;
   }

   {
   	LONG nbytes,i;
	ULONG foff;
	for( i = 0 ; i < mod->maxsamples ; i++ )
	{
		nbytes = mod_numbytes_of_sample( mod, i, &foff );
		if( nbytes )
		{
			/* simple here: just copy from already loaded file */
			if( 0 != modrender_loadsample( mrnd, i, buf+foff, nbytes ) )
			{
				printf("cannot load sample %ld with %ld bytes, aborting...\n",(long)i,(long)nbytes);
				modrender_free( mrnd );
				mod_free( mod );
			}
/*			else
			{
				printf("loaded sample  %ld with %ld bytes\n",(long)i,nbytes);
			}*/
		}
	}
   }

   wavfile = Wavwrite_Open( (STRPTR)"rendermod.wav", srate, 2 );
   obs  = 2*8192;
   obuf = (unsigned char*)MALLOC( sizeof(WORD)*obs );

   if( wavfile )
   	printf("have a wav file\n");
#if 1
   /* init done, now play something */
   if( (wavfile) && (obuf) )
   {
   	LONG ns,time_10us,ptime_ms=0;//,ct;
	UBYTE *pt_chnstate;
	LONG   errs = 0;
	LONG   pt_dmastate,i;
	ULONG  pt_songstate;
	LONG   pt_patpos,pt_songpos,pt_ct,pt_spd,pt_per;
	UBYTE  *pt_patdta; /* current pattern row in PT */
	struct MODPatternEntry *pdta;
	struct MODChannel *chan;

	PT_SetDryRun( 1 );   /* don't bang hardware */
	PT_InitMusic( buf ); /* init Protracker player (the real one :-) */

	//ct = 0;
	while( (ptime_ms < 300000) && !(mod->flags & MODPF_SONGEND) )// 30000 )
	{
		//ct++;
		time_10us = mod_playinterval( mod );
		//printf("t10us %ld\n",time_10us);

		PT_PlayTick( );

		/* compare playback state */
		pt_chnstate  = PT_DryChannelState();
		pt_dmastate  = PT_DryDMAState();
		pt_songstate = PT_DryCounterSpeedPposSpos(); /* Counter<<24 Speed<<16 Patpos<<8 Songpos */
		pt_patdta    = PT_DryPatternEntry(); /* return current pattern row in PT */
		pt_songpos   = (pt_songstate&0xff);
		pt_patpos    = (pt_songstate>>8)&0xff;
		pt_spd       = (pt_songstate>>16)&0xff;
		pt_ct        = (pt_songstate>>24)&0xff;
#if 0
		printf("ct %1ld sp %2ld pp %2ld sp %2ld\n",(long)(pt_songstate>>24),(long)((pt_songstate&0xff0000)>>16),(long)((pt_songstate&0xff00)>>8),(long)(pt_songstate&0xff));
#endif

		/* verify playback timing and positioning (pattern pos, song pos etc.) */
		if( (mod->counter != pt_ct) || (mod->songpos != pt_songpos) || (mod->patpos != pt_patpos) || (mod->speed != pt_spd)  )
		{
			errs++;
			printf("PT ct %1ld spd %2ld pp %2ld sp %2ld\n",(long)(pt_songstate>>24),(long)((pt_songstate&0xff0000)>>16),(long)((pt_songstate&0xff00)>>8),(long)(pt_songstate&0xff));
			printf("MP ct %1ld spd %2ld pp %2ld sp %2ld\n",(long)mod->counter,(long)mod->speed,(long)mod->patpos,(long)mod->songpos);
		}

		/* verify volume effects and note/period effects */
		pdta = mod->pdta;

#if 0
		if( (mod->songpos == 0 )
//		    || (mod->songpos < 64 ) 
		  )
		if( (pt_ct == 0) && (pdta) )
		{
			print_pat_row( pt_patdta ); /* protracker row */
//			errs++;
		}
#endif

		if( pdta )
		{
			/* compare volumes */
			for( i=0 ; i < 4 ; i++ )
			{
				chan = mod->channels[i];
				if( chan->volume != pt_chnstate[9+(i<<4)] )
				{
					errs++;
					printf("volume chan %1ld PT %2ld MP %2ld at ct %ld pp %2ld sp %2ld\n",(long)i,pt_chnstate[9+(i<<4)],chan->volume,(long)mod->counter,(long)mod->patpos,(long)mod->songpos);
				}
			}
			/* verify notes, effects, finetuning */
			for( i=0 ; i < 4 ; i++ )
			{
				chan   = mod->channels[i];
				pt_per = ( (((long)pt_chnstate[6+(i<<4)])<<8) | pt_chnstate[7+(i<<4)]);
//				if( (i==1) )
//				if( (i==1) && ( mod->songpos == 42 ) //== 1 )
				if( (chan->period != pt_per)
//				    || (mod->songpos == 17 )
				  )
				{
					errs++;
					printf("period chan %1ld PT %2ld MP %2ld at ct %ld pp %2ld sp %2ld vibidx %2ld vd %1ld vs %1ld\n",
					       (long)i,(long)pt_per,(long)chan->period,
					       (long)mod->counter,(long)mod->patpos,(long)mod->songpos,
					       (long)chan->avib_index,(long)chan->vib_depth,(long)chan->avib_speed);
				}
			}
		}

		/* stop after song position 0 */
		if( errs > 380 ) //56*6 ) //(pt_songpos > 1) || (errs > 10) )
			break;

		ns = modrender_render( mrnd, time_10us, (WORD*)obuf, obs );
		//printf("nsam %ld\n",ns);
		
		if( ns > 0 )
			Wavwrite_Write16LE( wavfile, (WORD*)obuf, ns*2 );

		ptime_ms += time_10us/100;

		//printf(".\n");
		//ct++;
		//if( ct > 2 )
		//	break;
	}
	{
	 long time_m  = (long)ptime_ms/(1000*60);
	 long time_s  = (ptime_ms/1000)-time_m*60;

	 printf("finished at %ldm:%lds\n",time_m,time_s);
	}
   }
#endif
   if( wavfile )
	Wavwrite_Close( wavfile );
   if( obuf )
   {
	MFREE( obuf );
   }

   modrender_free( mrnd );
   mod_free( mod );

  } while(0);

  if( buf )
  {
  	MFREE( buf );
  }

  return 0;
}

const UWORD pt_pertab[37] = {
        856,808,762,720,678,640,604,570,538,508,480,453,
        428,404,381,360,339,320,302,285,269,254,240,226,
        214,202,190,180,170,160,151,143,135,127,120,113,0};

unsigned long per2note( unsigned long per )
{
 unsigned long ret=0;

 while( pt_pertab[ret] > per )
 {
	ret++;
 }

 return ret;
}


void print_pat_row( UBYTE *pt_patdta )
{
  int i;
  const unsigned char *notetable[13] = { "C-", "C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-","--" }; /* from FS3MDOC.TXT, courtesy of Brett Paterson */

  int oct,nidx;
  int per,smp,note,fx,fxa;

//then print it like this : notetable[noteval MOD 12]    (Modulus/Remainder)
//        followed by : noteval/12                   (octave)


  if( !pt_patdta )
  	return;

  for( i=0 ; i < 4 ; i++ )
  {
    smp = (pt_patdta[0] & 0x10) | (pt_patdta[2]>>4);
    per = (((int)(pt_patdta[0] & 0xf))<<8) | (int)(pt_patdta[1]);
    fx  = pt_patdta[2]&0xf;
    fxa = pt_patdta[3];

    oct = '-';
    nidx= 12;

    if( per )
    {
    	note = per2note( per );
	if( note < 36 )
	{
	     oct = (note/12)+'1';
	     nidx= note%12;
	}
    }

    printf("%s%c%2lx %1lx%02lx",notetable[nidx],oct,(long)smp,(long)fx,(long)fxa);
    if( i<3 )
     printf(" | ");
    else
     printf("\n");

    pt_patdta+=4;
  }
}



/* simple WAV writer */
#define WAV_RIFF_LEN 0x2C

#define WAV_RIFF_TLENTAG 0x4
#define WAV_RIFF_DLENTAG 0x28

#define WAV_RIFF_CHN 22
#define WAV_RIFF_RATE 24
#define WAV_RIFF_BYTEPERSEC 28
#define WAV_RIFF_BYTEPERBLOCK 32
#define WAV_RIFF_BITPERSAMPLE 34
static UBYTE wavhead[WAV_RIFF_LEN] = 
   { 0x52,0x49,0x46,0x46, /* 0 "RIFF" */
     0xF8,0xFF,0xFF,0xFF, /* 4 total length - 8 */
     0x57,0x41,0x56,0x45, /* 8 "WAVE" */
     0x66,0x6D,0x74,0x20, /*12 "fmt " */
     0x10,0x00,0x00,0x00, /*16 fmt size */
     0x01,0x00,           /*20 1=PCM, 3=IEEE754 */
     0x02,0x00,           /*22 channels */
     0x44,0xAC,0x00,0x00, /*24 sampling rate */
     0x10,0xB1,0x02,0x00, /*28 Bytes per second */
     0x04,0x00,           /*32 bytes per block */
     0x10,0x00,           /*34 bit per sample */
     0x64,0x61,0x74,0x61, /*36 "data" */
     0xD4,0xFF,0xFF,0xFF};/*40 total length - 44 */

#define SET32LE( ptr, off0, value ) \
 *( (ptr)+(off0)+0 ) = (value)&0xFF;\
 *( (ptr)+(off0)+1 ) = ((value)>>8)&0xFF;\
 *( (ptr)+(off0)+2 ) = ((value)>>16)&0xFF;\
 *( (ptr)+(off0)+3 ) = ((value)>>24)&0xFF;

#define SET16LE( ptr, off0, value ) \
 *( (ptr)+(off0)+0 ) = (value)&0xFF;\
 *( (ptr)+(off0)+1 ) = ((value)>>8)&0xFF;


/*
  write 16 bit PCM WAV file for given sampling rate and number of channels (1/2)

  FIXME: not reentrant, due to header re-use
*/
FH_T Wavwrite_Open( STRPTR fname, LONG rate, LONG nchannels )
{
	FH_T wf;
	UBYTE *wh;
	LONG bpb,bps;

	wf = FILE_OPEN_W( fname );
	if( !wf )
		return (0);

	wh = (UBYTE*)wavhead;

	wh[WAV_RIFF_CHN] = nchannels;
	SET32LE( wh, WAV_RIFF_RATE, rate )

	bpb = nchannels*2; /* 16 bit only, actually correct: nchannels*BitPerSample/8 */
	SET16LE( wh, WAV_RIFF_BYTEPERBLOCK, bpb )

	bps = bpb * rate;
	SET32LE( wh, WAV_RIFF_BYTEPERSEC, bps )

	FILE_WRITE( wh, WAV_RIFF_LEN, wf );

	return wf;
}

/* 
  write a block of audio data (assuming it is in correct =LE endianess)

*/
LONG Wavwrite_Write( FH_T wf, UBYTE *buf, LONG bytes )
{
	if( !(wf) || !(buf) )
		return 0;

	return FILE_WRITE( buf, bytes, wf );
}

/*
  write a number of 16 bit words

  CAUTION: in-place modify of buffer contents 
*/
LONG Wavwrite_Write16LE(  FH_T wf, WORD *buf, LONG words )
{
	LONG n,sz,t;
	UBYTE *b1;
	WORD  *b2;

	if( !(wf) || !(buf) )
		return 0;

	/* byteswap */
	n = words;
	b1= (UBYTE*)buf;
	b2= buf;
	while( n-- )
	{
		t = *b2++;
		*b1++ = t&0xff;
		*b1++ = (t>>8)&0xff;
	}

	sz = FILE_WRITE( buf, (words<<1), wf );

	return sz;
}



void Wavwrite_Close( FH_T wf )
{
	LONG totlen;
	UBYTE *wh;

	wh = (UBYTE*)wavhead;

	if( !wf )
		return;

	FILE_SEEK_END( 0, wf );
	totlen = FILE_SEEK_TELL( wf );

	FILE_SEEK_START( 0, wf );
	
	/* update header */
	SET32LE( wh, WAV_RIFF_TLENTAG, totlen-8 );
	SET32LE( wh, WAV_RIFF_DLENTAG, totlen-44 );

	/* write header */
	FILE_WRITE( wh, WAV_RIFF_LEN, wf );

	FILE_CLOSE( wf );
}


