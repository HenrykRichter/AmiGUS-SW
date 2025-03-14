/*

  wav_convert16.c

  (C) 2025 Henryk Richter

  convert PCM data from WAVE file to 16 Bit signed
  in host endianess

  The current approach assumes separate buffers
  for left and right channel. Interleaved stereo
  is not on the map at the point of this writing.
 
*/
#include "wav_load.h"
#include "wav_ulawalaw.h"
//#include <proto/dos.h>

LONG divideby3( LONG x);

/* local proto */
LONG GetBuffer_16_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );
LONG GetBuffer_16_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );

LONG GetBuffer_24_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );
LONG GetBuffer_24_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );

LONG GetBuffer_8_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );
LONG GetBuffer_8_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );

LONG GetBuffer_32_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );
LONG GetBuffer_32_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );

/* interleaved stereo output (also for Mono WAV files) */
LONG GetBufferS_8_16_Mono_Wav(   struct wavload *wav, WORD *buffer, ULONG buffersize );
LONG GetBufferS_8_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize );

LONG GetBufferS_16_16_Mono_Wav(  struct wavload *wav, WORD *buffer, ULONG buffersize );
LONG GetBufferS_16_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize );

LONG GetBufferS_24_16_Mono_Wav(  struct wavload *wav, WORD *buffer, ULONG buffersize );
LONG GetBufferS_24_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize );

LONG GetBufferS_32_16_Mono_Wav(  struct wavload *wav, WORD *buffer, ULONG buffersize );
LONG GetBufferS_32_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize );


/* buffer reading, 16 bit signed interleaved stereo output
   in: wav     - from Open_Wav()
       buffer  - output buffer
       buffersize - number of bytes space 

   note: 
   - buffer loading is consecutive, no random access
*/
LONG GetBuffer16Stereo_Wav( struct wavload *wav, WORD *bufferL, ULONG buffersize )
{

	LONG sz;

	if( !wav )
		return -1;
	if( wav->read_pos >= wav->data_size )
		return 0; /* EOF */
	if( !bufferL )
		return -3;

	sz = buffersize << wav->tmp_bufshift; /* input buffer size depending on WAV file depth */

	if( wav->tmp_buffersize < sz )
	{
		//Printf((STRPTR)"Allocating temp buffer %ld\n",sz);
		
		if( wav->tmp_buffer )
			MFREE( wav->tmp_buffer );
		wav->tmp_buffersize = 0;
		wav->tmp_buffer = MALLOC( sz );
		if( !wav->tmp_buffer )
			return -2; /* error: out of memory */
		wav->tmp_buffersize = sz;
	}

	switch( wav->format )
	{
		case MYWAV_FORMAT_S16:
			if( wav->nchannels < 2 )
			  return GetBufferS_16_16_Mono_Wav( wav, bufferL, buffersize );
			else
			  return GetBufferS_16_16_Stereo_Wav( wav, bufferL, buffersize );
		case MYWAV_FORMAT_ALAW:
		case MYWAV_FORMAT_ULAW:
		case MYWAV_FORMAT_U8:
			if( wav->nchannels < 2 )
			  return GetBufferS_8_16_Mono_Wav( wav, bufferL, buffersize );
			else
			  return GetBufferS_8_16_Stereo_Wav( wav, bufferL, buffersize );
		case MYWAV_FORMAT_S24:
			if( wav->nchannels < 2 )
			  return GetBufferS_24_16_Mono_Wav( wav, bufferL, buffersize );
			else
			  return GetBufferS_24_16_Stereo_Wav( wav, bufferL, buffersize );
		case MYWAV_FORMAT_S32:
			if( wav->nchannels < 2 )
			  return GetBufferS_32_16_Mono_Wav( wav, bufferL, buffersize );
			else
			  return GetBufferS_32_16_Stereo_Wav( wav, bufferL, buffersize );
		default: 
			return 0; /* nothing to be done, unsupported format (results in EOF) */
	}

	return 0;
}

/* buffer reading, 16 bit signed output
   in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

   note: 
   - buffer loading is consecutive, no random access
*/
LONG GetBuffer16_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG sz;

	if( !wav )
		return -1;
	if( wav->read_pos >= wav->data_size )
		return 0; /* EOF */
	if( !bufferL )
		return -3;
	if( !bufferR )
		bufferR = bufferL;

	sz = buffersize << wav->tmp_bufshift; /* input buffer size depending on WAV file depth */

	if( wav->tmp_buffersize < sz )
	{
		if( wav->tmp_buffer )
			MFREE( wav->tmp_buffer );
		wav->tmp_buffersize = 0;
		wav->tmp_buffer = MALLOC( sz*2 );
		if( !wav->tmp_buffer )
			return -2; /* error: out of memory */
		wav->tmp_buffersize = sz;
	}

	switch( wav->format )
	{
		case MYWAV_FORMAT_S16:
			if( wav->nchannels < 2 )
				return GetBuffer_16_16_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_16_16_Stereo_Wav( wav, bufferL, bufferR, buffersize );

		case MYWAV_FORMAT_U8:
			if( wav->nchannels < 2 )
				return GetBuffer_8_16_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_8_16_Stereo_Wav( wav, bufferL, bufferR, buffersize );

		case MYWAV_FORMAT_S32:
			if( wav->nchannels < 2 )
				return GetBuffer_32_16_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_32_16_Stereo_Wav( wav, bufferL, bufferR, buffersize );
		case MYWAV_FORMAT_S24:
			if( wav->nchannels < 2 )
				return GetBuffer_24_16_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_24_16_Stereo_Wav( wav, bufferL, bufferR, buffersize );
		default: 
			return 0; /* nothing to be done, unsupported format (results in EOF) */
	}
}

/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_16_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG  nframes = (wav->data_size - wav->read_pos)>>2; /* number of frames left (input = 4 byte per frame) */
	LONG  i;
	ULONG cur;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nframes > (buffersize>>1) ) /* an output frame is 2 bytes per channel */
		nframes = (buffersize>>1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<2), wav->file );
	wav->read_pos += (nframes<<2);

	i = nframes;
	while( i-- )
	{
		cur = GETSHORT_LE( *tmpbuf ); /* A0 B0 */
		*bufferL++ = cur;
		tmpbuf++; 
		cur = GETSHORT_LE( *tmpbuf ); /* A1 B1 */
		*bufferR++ = cur;
		tmpbuf++; /* SKIP B1 A1 D2 C2 */
	}

	return (nframes<<1); /* */
}


LONG GetBuffer_16_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG  nframes = (wav->data_size - wav->read_pos)>>1; /* number of frames left (input = 2 byte per frame) */
	LONG  i;
	ULONG cur;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nframes > (buffersize>>1) ) /* an output frame is 2 bytes per channel */
		nframes = (buffersize>>1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<1), wav->file );
	wav->read_pos += (nframes<<1);

	i = nframes;
	while( i-- )
	{
		cur = GETSHORT_LE( *tmpbuf ); /* A0 B0 */
		*bufferL++ = cur;
		tmpbuf++; 
		*bufferR++ = cur;
	}

	return (nframes<<1); /* */
}


/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_8_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,curL,curR;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nbytes > buffersize ) /* (buffersize>>1) * 2 channels */
		nbytes = buffersize;
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>1; /* /2 channels */
	while( i-- )
	{
		curL = *tmpbuf++;
		curR = *tmpbuf++;
		curL ^= 0x80; /* unsigned -> signed */
		curR ^= 0x80; /* unsigned -> signed */
		curL  = (curL<<8); // + curL;
		curR  = (curR<<8); // + curR;

		*bufferR++ = curR;
		*bufferL++ = curL;
	}

	return (nbytes>>1); /* nbytes is for two channels on input side */
}


LONG GetBuffer_8_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,cur;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nbytes > (buffersize>>1) )
		nbytes = (buffersize>>1);

	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes;
	while( i-- )
	{
		cur   = 0x80 ^ *tmpbuf++; /* unsigned -> signed */
		cur <<= 8;
		*bufferL++ = cur;
		*bufferR++ = cur;
	}

	return (nbytes<<1);
}



/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_24_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG  nframes = divideby3( (wav->data_size - wav->read_pos)>>1 ); /* number of frames left (input = 6 byte per frame) */
	LONG  i;
	ULONG curL,curR;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nframes > (buffersize>>1) ) /* an output frame is 2 bytes per channel */
		nframes = (buffersize>>1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes+(nframes<<1))<<1, wav->file );
	wav->read_pos += (nframes+(nframes<<1))<<1;

	i = nframes;
	while( i-- )
	{
		/* input: 3 bytes per frame C0 B0 A0 C1 B1 A1 */
		curL = (tmpbuf[2]<<8)|(tmpbuf[1]); /* A0 B0 */
		*bufferL++ = curL;

		curR = (tmpbuf[5]<<8)|(tmpbuf[4]); /* A1 B1 */
		*bufferR++ = curR;

		tmpbuf += 6;
	}

	return (nframes<<1); /* */
}


LONG GetBuffer_24_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,cur;
	ULONG *tmpbuf = (ULONG*)wav->tmp_buffer;

	if( nbytes > (buffersize<<1) )
		nbytes = (buffersize<<1);
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>2;
	while( i-- )
	{
		cur   = GETLONG_LE( *tmpbuf );
		cur >>= 16;
		*bufferL++ = cur;
		tmpbuf++;
	}

	return (nbytes>>1);
}




/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_32_16_Stereo_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,curL,curR;
	ULONG *tmpbuf = (ULONG*)wav->tmp_buffer;

	if( nbytes > (buffersize<<2) ) /* *2 stereo, *2 Int32 */
		nbytes = (buffersize<<2);
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>3; /* /4 32Bit, /2 channels = NFRAMES */
	while( i-- )
	{
		curL = GETLONG_LE( *tmpbuf );
		tmpbuf++;
		curR = GETLONG_LE( *tmpbuf );
		tmpbuf++;
		curL >>= 16;
		curR >>= 16;

		*bufferR++ = curR;
		*bufferL++ = curL;
	}

	return (nbytes>>2); /* nbytes is for two channels on input side, 32 bit on input side */
}

LONG GetBuffer_32_16_Mono_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,cur;
	ULONG *tmpbuf = (ULONG*)wav->tmp_buffer;

	if( nbytes > (buffersize<<1) )
		nbytes = (buffersize<<1);
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>2;
	while( i-- )
	{
		cur   = GETLONG_LE( *tmpbuf );
		cur >>= 16;
		*bufferL++ = cur;
		tmpbuf++;
	}

	return (nbytes>>1);
}




/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_32_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG  nframes = (wav->data_size - wav->read_pos)>>3; /* number of frames left (input = 8 byte per frame) */
	LONG  i;
	ULONG cur;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nframes > (buffersize>>2) ) /* an output frame is 4 bytes */
		nframes = (buffersize>>2);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<3), wav->file );
	wav->read_pos += (nframes<<3);

	i = nframes;
	tmpbuf++; /* skip first two bytes D0 C0 */
	while( i-- )
	{
		/* input: 8 bytes per frame D0 C0 B0 A0 D1 C1 B1 A1 */
		cur = GETSHORT_LE( *tmpbuf ); /* A0 B0 */
		*buffer++ = cur;
		tmpbuf += 2; /* SKIP B0 A0 D1 C1 */

		cur = GETSHORT_LE( *tmpbuf ); /* A1 B1 */
		*buffer++ = cur;
		tmpbuf += 2; /* SKIP B1 A1 D2 C2 */
	}

	return (nframes<<2); /* */
}

/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_32_16_Mono_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG  nframes = (wav->data_size - wav->read_pos)>>2; /* number of frames left (input = 8 byte per frame) */
	LONG  i;
	ULONG cur;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nframes > (buffersize>>2) ) /* an output frame is 4 bytes */
		nframes = (buffersize>>2);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<2), wav->file );
	wav->read_pos += (nframes<<2);

	i = nframes;
	tmpbuf++; /* skip first two bytes D0 C0 */
	while( i-- )
	{
		/* input: 8 bytes per frame D0 C0 B0 A0 D1 C1 B1 A1 */
		cur = GETSHORT_LE( *tmpbuf ); /* A0 B0 */
		*buffer++ = cur;
		*buffer++ = cur;
		tmpbuf += 2; /* SKIP B0 A0 D1 C1 */
	}

	return (nframes<<2); /* */
}



/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_24_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG  nframes = divideby3( (wav->data_size - wav->read_pos)>>1 ); /* number of frames left (input = 6 byte per frame) */
	LONG  i;
	ULONG curL;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nframes > (buffersize>>2) ) /* an output frame is 4 bytes */
		nframes = (buffersize>>2);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes+(nframes<<1))<<1, wav->file );
	wav->read_pos += (nframes+(nframes<<1))<<1;

	i = nframes;
	while( i-- )
	{
		/* input: 3 bytes per frame C0 B0 A0 C1 B1 A1 */
		curL = (tmpbuf[2]<<8)|(tmpbuf[1]); /* A0 B0 */
		*buffer++ = curL;

		curL = (tmpbuf[5]<<8)|(tmpbuf[4]); /* A1 B1 */
		*buffer++ = curL;

		tmpbuf += 6;
	}

	return (nframes<<2); /* */
}

/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_24_16_Mono_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG  nframes = divideby3(wav->data_size - wav->read_pos); /* number of frames left */
	LONG  i;
	ULONG curL;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nframes > (buffersize>>2) ) /* an output frame is 4 bytes */
		nframes = (buffersize>>2);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes+(nframes<<1)), wav->file );
	wav->read_pos += (nframes+(nframes<<1));

	i = nframes;
	while( i-- )
	{
		/* input: 3 bytes per frame C0 B0 A0 */
		curL = (tmpbuf[2]<<8)|(tmpbuf[1]); /* A0 B0 */
		tmpbuf += 3;
		*buffer++ = curL;
		*buffer++ = curL;
	}

	return (nframes<<2); /* */
}



/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_16_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos; /* number of bytes left */
	LONG i,curL;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nbytes > buffersize )
		nbytes = buffersize;

	nbytes &= ~3; /* make sure that we output an even number of samples (2 channels, 16 bit per sample = 4 bytes per pair) */
	if( !nbytes )
		return 0;

	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>1; /* /2 16Bit */
	while( i-- )
	{
		curL = GETSHORT_LE( *tmpbuf );
		tmpbuf++;
		*buffer++ = curL;
	}

	return nbytes; /* */
}



/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in both buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_16_16_Mono_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos; /* number of bytes left */
	LONG i,curL;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nbytes > (buffersize>>1) ) /* 16 bit mono input -> write stuff twice for stereo output, hence buffersize>>1 */
		nbytes = (buffersize>>1);

	nbytes &= ~3; /* make sure that we output an even number of samples (2 channels, 16 bit per sample = 4 bytes per pair) */
	if( !nbytes )
		return 0;

	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>1; /* /2 16Bit inputs */
	while( i-- )
	{
		curL = GETSHORT_LE( *tmpbuf );
		tmpbuf++;
		*buffer++ = curL;
		*buffer++ = curL;
	}

	return (nbytes<<1); /* */
}


/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_8_16_Stereo_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos; /* number of bytes left */
	LONG i,curL;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nbytes > (buffersize>>1) ) /* outputs are twice as large as inputs */
		nbytes = (buffersize>>1);

	nbytes &= ~3; /* make sure that we output an even number of samples (2 channels, 16 bit per sample = 4 bytes per pair) */
	if( !nbytes )
		return 0;

	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;


	i = nbytes; /* 8 bit inputs (stereo) */

	if( (wav->format == MYWAV_FORMAT_ALAW) ||
	    (wav->format == MYWAV_FORMAT_ULAW) )
	{
		unsigned short *convtab;
	    
		if( wav->format == MYWAV_FORMAT_ALAW )
		     convtab = ALawTable16;
		else convtab = uLawTable16;

		while( i-- )
		{
			curL  = *tmpbuf++;
			curL  = convtab[curL];
			*buffer++ = curL;
		}
	}
	else
	{
		while( i-- )
		{
			curL = *tmpbuf++;
			curL ^= 0x80;
			*buffer++ = (curL<<8);
		}
	}

	return (nbytes<<1); /* */
}


/*
    in: wav     - from Open_Wav()
       buffer   - left/right channel output
       buffersize - number of bytes space in buffer

    out: number of bytes written (samples = bytes/4)
*/
LONG GetBufferS_8_16_Mono_Wav( struct wavload *wav, WORD *buffer, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos; /* number of bytes left */
	LONG i,curL;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;

	if( nbytes > (buffersize>>2) ) /* outputs are 4x as large as inputs */
		nbytes = (buffersize>>2);

	nbytes &= ~3; /* make sure that we output an even number of samples (2 channels, 16 bit per sample = 4 bytes per pair) */
	if( !nbytes )
		return 0;

#if 0
	{
	 ULONG *bla = (ULONG*)tmpbuf;
	 Printf("nbytes %ld to 0x%lx (length %ld)\n",nbytes,(ULONG)tmpbuf,bla[-1]);
	}
#endif

	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes; /* 8 bit inputs (mono) */

	if( (wav->format == MYWAV_FORMAT_ALAW) ||
	    (wav->format == MYWAV_FORMAT_ULAW) )
	{
		unsigned short *convtab;
	    
		if( wav->format == MYWAV_FORMAT_ALAW )
		     convtab = ALawTable16;
		else convtab = uLawTable16;

		while( i-- )
		{
			curL  = *tmpbuf++;
			curL  = convtab[curL];
			*buffer++ = curL;
			*buffer++ = curL;
		}
	}
	else
	{
		while( i-- )
		{
			curL  = *tmpbuf++;
			curL ^= 0x80;
			curL <<= 8;
			*buffer++ = curL;
			*buffer++ = curL;
		}
	}

	return (nbytes<<2); /* */
}

