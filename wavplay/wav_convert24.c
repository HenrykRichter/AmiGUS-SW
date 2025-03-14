/*

  wav_convert24.c

  (C) 2025 Henryk Richter

  convert PCM data from WAVE file to 24 Bit signed
  in host endianess

  Two sets of functions are available for either interleaved
  or non-interleaved output.

 FIXME: This is unfinished, doesn't work yet for non-interleaved outputs

*/
#include "wav_load.h"
#include "wav_ulawalaw.h"
//#include <proto/dos.h>

/* local proto */
LONG GetBuffer_16_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );
LONG GetBuffer_16_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );

LONG GetBuffer_8_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );
LONG GetBuffer_8_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );

LONG GetBuffer_32_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );
LONG GetBuffer_32_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );

/* interleaved stereo output (also for Mono WAV files) */
LONG GetBufferS_8_24_Mono_Wav(   struct wavload *wav, UBYTE *buffer, ULONG buffersize );
LONG GetBufferS_8_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG buffersize );
LONG GetBufferS_32_24_Mono_Wav(  struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );
LONG GetBufferS_32_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );
LONG GetBufferS_24_24_Mono_Wav(  struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );
LONG GetBufferS_24_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );
LONG GetBufferS_16_24_Mono_Wav(  struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );
LONG GetBufferS_16_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve );

LONG divideby3( LONG x );


/* x / 3 (adjust code for systems where divl is not available */
LONG divideby3( LONG x)
{
	return x/3;
}

#if 0
/* from: https://stackoverflow.com/questions/31703174/divide-a-number-by-3-without-using-division-multiplication-or-modulus */
int oneThirdOf(int n){
    if (0<=n && n<3)
        return 0;
    if (n==3)
        return 1;
    return sum(n) + oneThirdOf(wt4(n));
}

// Compute (n>>2) + (n>>4) + (n>>6) + ... recursively.
int sum(int n){
    if (n<4)
        return 0;
    return (n>>2) + sum(n>>2);
}

// Compute the sum of the digits of n base 4 recursively.
int wt4(int n){
    if (n<4)
        return n;
    int fourth = n>>2;
    int lastDigit = n-fourth-fourth-fourth-fourth;
    return wt4(fourth) + lastDigit;
}
#endif



/* buffer reading, 16 bit signed interleaved stereo output
   in: wav     - from Open_Wav()
       buffer  - output buffer
       buffersize - number of bytes space 

   note: 
   - buffer loading is consecutive, no random access
*/
LONG GetBuffer24Stereo_Wav( struct wavload *wav, UBYTE *bufferL, ULONG buffersize )
{
	LONG nbytwelve;
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

	/* we want a multiple of 12 bytes for the outputs, reason:
           long word writes to the hardware, so everything divisible by 12
	   is implicitly divisible by 4. This multiple enforces a multiple
	   of two 24-bit stereo pairs.
	*/
	nbytwelve = divideby3( buffersize >> 2 );

	switch( wav->format )
	{
		case MYWAV_FORMAT_S16:
			if( wav->nchannels < 2 )
			  return GetBufferS_16_24_Mono_Wav( wav, bufferL, nbytwelve );
			else
			  return GetBufferS_16_24_Stereo_Wav( wav, bufferL, nbytwelve );
		case MYWAV_FORMAT_S24:
			if( wav->nchannels < 2 )
			  return GetBufferS_24_24_Mono_Wav( wav, bufferL, nbytwelve );
			else
			  return GetBufferS_24_24_Stereo_Wav( wav, bufferL, nbytwelve );
		case MYWAV_FORMAT_S32:
			if( wav->nchannels < 2 )
			  return GetBufferS_32_24_Mono_Wav( wav, bufferL, nbytwelve );
			else
			  return GetBufferS_32_24_Stereo_Wav( wav, bufferL, nbytwelve );
		case MYWAV_FORMAT_ALAW:
		case MYWAV_FORMAT_ULAW:
		case MYWAV_FORMAT_U8:
			if( wav->nchannels < 2 )
			  return GetBufferS_8_24_Mono_Wav( wav, bufferL, nbytwelve );
			else
			  return GetBufferS_8_24_Stereo_Wav( wav, bufferL, nbytwelve );
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
   - for mono WAVs, the right channel stays empty, even if a valid pointer is passed
   - buffer loading is consecutive, no random access
*/
LONG GetBuffer24_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
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
		wav->tmp_buffer = MALLOC( sz );
		if( !wav->tmp_buffer )
			return -2; /* error: out of memory */
		wav->tmp_buffersize = sz;
	}

	/* FIXME: actually implement the functions below (copies of 16 bit code, right now) */
	return 0;


	switch( wav->format )
	{
		case MYWAV_FORMAT_S16:
			if( wav->nchannels < 2 )
				return GetBuffer_16_24_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_16_24_Stereo_Wav( wav, bufferL, bufferR, buffersize );

		case MYWAV_FORMAT_U8:
			if( wav->nchannels < 2 )
				return GetBuffer_8_24_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_8_24_Stereo_Wav( wav, bufferL, bufferR, buffersize );

		case MYWAV_FORMAT_S32:
			if( wav->nchannels < 2 )
				return GetBuffer_32_24_Mono_Wav( wav, bufferL, bufferR, buffersize );
			else
				return GetBuffer_32_24_Stereo_Wav( wav, bufferL, bufferR, buffersize );


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
LONG GetBuffer_16_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,curL,curR;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nbytes > (buffersize+buffersize) )
		nbytes = (buffersize+buffersize);
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>2; /* /2 16Bit, /2 channels */
	while( i-- )
	{
		curL = GETSHORT_LE( *tmpbuf );
		tmpbuf++;
		curR = GETSHORT_LE( *tmpbuf );
		tmpbuf++;

		*bufferR++ = curR;
		*bufferL++ = curL;
	}

	return (nbytes>>1); /* nbytes is for two channels on input side */
}

LONG GetBuffer_16_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
{
	LONG nbytes = wav->data_size - wav->read_pos;
	LONG i,cur;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;

	if( nbytes > buffersize )
		nbytes = buffersize;
	
	FILE_READ( tmpbuf, nbytes, wav->file );
	wav->read_pos += nbytes;

	i = nbytes>>1;
	while( i-- )
	{
		cur = GETSHORT_LE( *tmpbuf );
		*bufferL++ = cur;
		tmpbuf++;
	}

	return nbytes;
}

/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_8_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
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

LONG GetBuffer_8_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
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
	}

	return nbytes;
}

/*
    in: wav     - from Open_Wav()
       bufferL - left channel output
       bufferR - right channel output (or NULL for mono)
       buffersize - number of bytes space in both bufferL and bufferR

    out: number of bytes written (samples = bytes/2)
*/
LONG GetBuffer_32_24_Stereo_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
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

LONG GetBuffer_32_24_Mono_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize )
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
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_24_24_Mono_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = divideby3(wav->data_size - wav->read_pos); /* number of bytes left / 3 = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UBYTE *obuf = (UBYTE*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes+nframes+nframes), wav->file ); /* 24 bit mono */
	wav->read_pos += (nframes+nframes+nframes);

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: this outputs BIG ENDIAN only */
		/* 24 bit C0 B0 A0 -> A0 B0 C0 */
		*obuf++ = tmpbuf[2]; /* A0 */
		*obuf++ = tmpbuf[1]; /* B0 */
		*obuf++ = tmpbuf[0]; /* C0 */
		*obuf++ = tmpbuf[2]; /* A0 */
		*obuf++ = tmpbuf[1]; /* B0 */
		*obuf++ = tmpbuf[0]; /* C0 */
		tmpbuf += 3;
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}

/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_24_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = divideby3( (wav->data_size - wav->read_pos)>>1 ); /* number of bytes left / 6 = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UBYTE *obuf = (UBYTE*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes+nframes+nframes)<<1, wav->file ); /* 24 bit stereo */
	wav->read_pos += (nframes+nframes+nframes)<<1;

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: this outputs BIG ENDIAN only */
		/* 32 bit C0 B0 A0 C1 B1 A1... to 2*24 bit A0 B0 C0 A1 B1 C1 */
		*obuf++ = tmpbuf[2]; /* A0 */
		*obuf++ = tmpbuf[1]; /* B0 */
		*obuf++ = tmpbuf[0]; /* C0 */
		*obuf++ = tmpbuf[5]; /* A1 */
		*obuf++ = tmpbuf[4]; /* B1 */
		*obuf++ = tmpbuf[3]; /* C1 */
		tmpbuf += 6;
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}


/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_32_24_Mono_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos)>>2; /* number of bytes left / 4 = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UBYTE *obuf = (UBYTE*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<2), wav->file ); /* 32 bit mono */
	wav->read_pos += (nframes<<2);

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: this outputs BIG ENDIAN only */
		/* 32 bit D0 C0 B0 A0 D1 C1... to 2*24 bit A0 B0 C0 A0 B0 C0 */
		*obuf++ = tmpbuf[3]; /* A0 */
		*obuf++ = tmpbuf[2]; /* B0 */
		*obuf++ = tmpbuf[1]; /* C0 */
		*obuf++ = tmpbuf[3]; /* A0 */
		*obuf++ = tmpbuf[2]; /* B0 */
		*obuf++ = tmpbuf[1]; /* C0 */
		tmpbuf += 4;
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}

/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_32_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos)>>3; /* number of bytes left / 8 = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UBYTE *obuf = (UBYTE*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<3), wav->file ); /* 32 bit stereo */
	wav->read_pos += (nframes<<3);

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: this outputs BIG ENDIAN only */
		/* 32 bit D0 C0 B0 A0 D1 C1... to 2*24 bit A0 B0 C0 A1 B1 C1 */
		*obuf++ = tmpbuf[3]; /* A0 */
		*obuf++ = tmpbuf[2]; /* B0 */
		*obuf++ = tmpbuf[1]; /* C0 */
		*obuf++ = tmpbuf[7]; /* A1 */
		*obuf++ = tmpbuf[6]; /* B1 */
		*obuf++ = tmpbuf[5]; /* C1 */
		tmpbuf += 8;
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}


/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_16_24_Mono_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos)>>1; /* number of bytes left / 2 = nsamples left */
	LONG  i;
	UWORD cura,curb;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;
	UWORD *obuf = (UWORD*)buffer;

	/* 16 bit mono input = 2 bytes per sample
	   need: nbytwelve*2 samples */
	if( nframes > (nbytwelve<<1) )
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<1), wav->file ); /* 16 bit mono */
	wav->read_pos += (nframes<<1);

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: big endian only */
		/* 16 bit xx yy to 2*24 bit xx yy 00 xx yy 00 */
		cura = GETSHORT_LE( *tmpbuf ); /* xx yy */
		tmpbuf++;
		*obuf++ = cura;    	/* xx yy */
		curb    = cura>>8; 	/* 00 xx */
		*obuf++ = curb; 	/* 00 xx */
		cura<<= 8; 		/* yy 00 */
		*obuf++ = cura;		/* yy 00 */
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}


/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_16_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos)>>2; /* number of bytes left / 4 = nframes left */
	LONG  i;
	UWORD cura,curb;
	UWORD *tmpbuf = (UWORD*)wav->tmp_buffer;
	UWORD *obuf = (UWORD*)buffer;

	/* 16 bit stereo input = 4 bytes per sample
	   need: nbytwelve*2 samples */
	if( nframes > (nbytwelve<<1) )
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<2), wav->file ); /* 16 bit stereo */
	wav->read_pos += (nframes<<2);

	i = nframes; 
	while( i-- )
	{
		/* CAUTION: big endian only */
		/* 16 bit xx yy zz ww to 24 bit xx yy 00 zz ww 00 */
		cura = GETSHORT_LE( *tmpbuf ); /* xx yy */
		tmpbuf++;
		curb = GETSHORT_LE( *tmpbuf ); /* zz ww */
		tmpbuf++;

		*obuf++ = cura;    	/* xx yy */
		cura    = curb>>8;      /* 00 zz */
		curb  <<= 8;		/* ww 00 */
		*obuf++ = cura;		/* 00 zz */
		*obuf++ = curb;		/* ww 00 */
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}


#if 1 
/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_8_24_Mono_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos); /* number of bytes left  = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UWORD t0,t1;
	UWORD *obuf = (UWORD*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, nframes, wav->file ); /* 8 bit mono */
	wav->read_pos += nframes;

	i = nframes; 

	if( (wav->format == MYWAV_FORMAT_ALAW) ||
	    (wav->format == MYWAV_FORMAT_ULAW) )
	{
		unsigned short *convtab;
	    
		if( wav->format == MYWAV_FORMAT_ALAW )
		     convtab = ALawTable16;
		else convtab = uLawTable16;

		while( i-- )
		{
			t0  = *tmpbuf++;
			t0  = convtab[t0]; /* A B */
			t1  = t0<<8;       /* B 0 */
			/* A B 0 A B 0 */
			*obuf++ = t0;      /* A B */
			t0 >>= 8;
			*obuf++ = t0;      /* 0 A */
			*obuf++ = t1;      /* B 0 */
		}
	}
	else
	{
		while( i-- )
		{
			t0   = *tmpbuf++;
			t0  ^= 0x80;  /* = 0 A */
			t1   = t0<<8; /* = A 0 */
			/* A 0 0 A 0 0 */
			*obuf++ = t1; /*-> A 0 */
			*obuf++ = t0; /*-> 0 A */
			*obuf++ = 0;  /*-> 0 0 */
		}
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}

/*
    in: wav      - from Open_Wav()
       buffer    - left/right channel output
       nbytwelve - output buffersize / 12 (unit of two output frames)

    out: number of bytes written (samples = bytes/6)
*/
LONG GetBufferS_8_24_Stereo_Wav( struct wavload *wav, UBYTE *buffer, ULONG nbytwelve )
{
	LONG nframes = (wav->data_size - wav->read_pos)>>1; /* number of bytes left /2  = frames left in file */
	LONG  i;
	UBYTE *tmpbuf = (UBYTE*)wav->tmp_buffer;
	UWORD t0,t1;
	UWORD *obuf = (UWORD*)buffer;

	if( nframes > (nbytwelve<<1) ) /* max: nbytwelve*2 frames */
		nframes = (nbytwelve<<1);

	if( !nframes )
		return 0;

	FILE_READ( tmpbuf, (nframes<<1), wav->file ); /* 8 bit stereo */
	wav->read_pos += (nframes<<1);

	i = nframes; 

	if( (wav->format == MYWAV_FORMAT_ALAW) ||
	    (wav->format == MYWAV_FORMAT_ULAW) )
	{
		unsigned short *convtab;
	    
		if( wav->format == MYWAV_FORMAT_ALAW )
		     convtab = ALawTable16;
		else convtab = uLawTable16;

		while( i-- )
		{
			t0  = *tmpbuf++;
			t0  = convtab[t0]; /* A B */
			t1  = *tmpbuf++;
			t1  = convtab[t0]; /* C D */
			/* A B 0 C D 0 */
			*obuf++ = t0;      /* A B */
			t0  = t1>>8;
			*obuf++ = t0;      /* 0 C */
			t1  <<=8;
			*obuf++ = t1;      /* D 0 */
		}
	}
	else
	{
		while( i-- )
		{
			t0   = *tmpbuf++;
			t0  ^= 0x80;  /* = 0 A */
			t0   = t0<<8; /* = A 0 */
			t1   = *tmpbuf++;
			t1  ^= 0x80;  /* = 0 B */
			/* A 0 0 B 0 0 */
			*obuf++ = t0; /*-> A 0 */
			*obuf++ = t1; /*-> 0 B */
			*obuf++ = 0;  /*-> 0 0 */
		}
	}

	return (nframes+(nframes<<1))<<1; /* inputs*6 */
}
#endif

