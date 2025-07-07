/* **************************************************************************

   Somewhat generic tracker player, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   Test / Demo Application

   AGUS_Mod

   ************************************************************************** */
#include "modplay.h"
#include "modrender_common.h"
#include "fileio.h"
#include <stdio.h>

FH_T Wavwrite_Open( STRPTR fname, LONG rate, LONG nchannels );
LONG Wavwrite_Write( FH_T file, UBYTE *buf, LONG bytes );   /* write a block of bytes (assuming LE endianess) */
LONG Wavwrite_Write16LE(  FH_T wf, WORD *buf, LONG words ); /* write shorts in native endianess as LE */
void Wavwrite_Close( FH_T file );

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

	/* load whole file */
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

#if 1
   {
    struct MODSample **ms;
    struct MODSample *smp;
    LONG ns,idx,mod_samplesize;
    STRPTR ne=(STRPTR)" ";
    STRPTR nam;

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
	nam = (smp->name) ? smp->name : ne;
	printf("Sample %2ld len %6lx repstart %6lx replen %6lx name %s\n",(long)idx,(long)smp->length,(long)smp->repstart,(long)smp->replen,nam);
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

   /* TODO: check if we have initialized a renderer or a player instance */
   if( modrender_is_player( mrnd ) )
   {
	LONG time_10us=2000,ptime_ms=0;//,rtime;//,ct;

	printf("Start Playing\n");
	modrender_start_play( mrnd );

	/* TODO: end of stream method */

	//ct = 0;
	while( (ptime_ms < 30000) && !(mod->flags & MODPF_SONGEND) )// 30000 )
	{
#ifdef AMIGA
		if( SetSignal(0L,SIGBREAKF_CTRL_C)&SIGBREAKF_CTRL_C )
		{
			printf("Abort, Ptime_ms %ld\n",ptime_ms);
			break;
		}
#endif
		time_10us = modrender_render( mrnd, time_10us, NULL, 0 );
#ifdef AMIGA
		Delay(1);
#endif
		ptime_ms += time_10us/100;
	}


	modrender_stop_play( mrnd );
   }
   else
   {
	/* renderer mode */
	wavfile = Wavwrite_Open( (STRPTR)"rendermod.wav", srate, 2 );
	obs  = 2*8192;
	obuf = (unsigned char*)MALLOC( sizeof(WORD)*obs );

	if( wavfile )
		printf("have a wav file\n");
#if 1
	/* init done, now play something */
	if( (wavfile) && (obuf) )
	{
		LONG ns,time_10us,ptime_ms=0;//,rtime;//,ct;
		
		//ct = 0;
		while( (ptime_ms < 400000) && !(mod->flags & MODPF_SONGEND) )// 30000 )
		{
			//ct++;
			time_10us = mod_playinterval( mod );
			//printf("t10us %ld\n",time_10us);
#if 0
			if( time_10us != rtime )
				printf("t10us %ld\n",(long)time_10us);
			rtime = time_10us;
#endif
			ns = modrender_render( mrnd, time_10us, (WORD*)obuf, obs );
			//printf("nsam %ld\n",ns);
			
			if( ns > 0 )
				Wavwrite_Write16LE( wavfile, (WORD*)obuf, ns*2 );

			ptime_ms += time_10us/100;

			//printf(".\n");
			//ct++;
			//if( ct > 2 )
			//	break;
			//printf("%ld\n",(long)ptime_ms);
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

#ifdef __SASC
void __regargs __chkabort(void)
{}
#endif


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


