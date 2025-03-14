/*

  wav_load.c

  (C) 2025 Henryk Richter

  parser for RIFF WAVE files, 

  returns a structure outlining general parameters
  offers format conversion into a provided buffer

  ./wavload /Applications/Emu/Work/Mods/Mod.Ani_Birth.wav

 limitations:
  - no ADPCM, Float or MPEG support
 
*/
#include "wav_load.h"
#define WAV_MAIN
#include "wav_ulawalaw.h"

#if 0
#include "compiler.h"
#include "fileio.h"


#ifndef EXEC_RESIDENT_H
#include <stdint.h>
typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int16_t WORD;
typedef int32_t LONG;
#endif

#define MAX_CHANNELS 2
#define MAX_WAVE_RATE 0x80000 /* 512 kHz */

#define SWAPL(_a_) ( (((_a_)>>24)&0xFF)|(((_a_)>>8)&0xFF00)|(((_a_)<<8)&0xFF0000)|(((_a_)&0xFF)<<24) )
#define SWAPW(_a_) ((ULONG)( (((UWORD)(_a_)>>8)&0xFF)|((UWORD)(_a_)<<8) ))

/* endianess stuff */
#ifdef LITTLE_ENDIAN
#define GETLONG_BE(_a_) SWAPL(_a_)
#define GETSHORT_BE(_a_) SWAPW(_a_)
#define GETLONG_LE(_a_) _a_
#define GETSHORT_LE(_a_) _a_
#else
#define GETLONG_LE(_a_) SWAPL(_a_)
#define GETSHORT_LE(_a_) SWAPW(_a_)
#define GETLONG_BE(_a_) _a_
#define GETSHORT_BE(_a_) _a_
#endif

/* wav structures, definitions, prototypes */

#define MAKEID(a,b,c,d) ((((ULONG)(a))<<24) | (((ULONG)(b))<<16) | (((ULONG)(c))<<8) | (ULONG)(d))

#define ID_RIFF MAKEID('R','I','F','F')
#define ID_WAVE MAKEID('W','A','V','E')
#define ID_fmt  MAKEID('f','m','t',' ')
#define ID_fact MAKEID('f','a','c','t')
#define ID_cue  MAKEID('c','u','e',' ')
#define ID_data MAKEID('d','a','t','a')

#define WAV_FORMAT_PCM         0x001    /* M$ PCM */
#define WAV_FORMAT_MULAW       0x007    /* M$ u-LAW */
#define WAV_FORMAT_MPEG        0x050    /* MPEG-1 Layer 1,2 */
#define WAV_FORMAT_MP3         0x055    /* MPEG-1 Layer 3 */
#define WAV_FORMAT_EXTENSIBLE  0xfffe   /* new, extensible tag */
#define IBM_FORMAT_MULAW        0x101    /* IBM mu-law format */
#define IBM_FORMAT_ALAW         0x102    /* IBM a-law format */
#define IBM_FORMAT_ADPCM        0x103    /* IBM AVC Adaptive Differential Pulse Code Modulation format */
#define MYWAV_FORMAT_U8  0x8001
#define MYWAV_FORMAT_S16 0x8002
#define MYWAV_FORMAT_S32 0x8003


struct wavload {
 FH_T  file;
 ULONG wavsize;     /* from RIFF header (total file length) */
 ULONG data_offset; /* data chunk position (after chunk header) */
 ULONG data_size;   /* data chunk net length (excluding chunk header) */

 ULONG rate;        /* sampling rate in Hz */
 ULONG nsamples;
 ULONG depth;
 ULONG format;
 ULONG nchannels;
 ULONG BytesPerFrame;
};

struct RIFF_chunk {
 ULONG RC_id;
 ULONG RC_size;
} PACKATTR;

struct WAV_RIFFHDR {
 struct RIFF_chunk c;
 ULONG  RIFF_format;
} PACKATTR;

struct WAV_Format {
 struct RIFF_chunk c;
 UWORD  FMT_Format;
 UWORD	FMT_Channels;
 ULONG	FMT_Samples_Per_Sec;
 ULONG	FMT_Avg_Bytes_Per_Sec;
 UWORD  FMT_BlockAlign;
} PACKATTR;

struct WAV_FormatExt {
/* same as WAV_Format */
 struct RIFF_chunk c;
 UWORD  FMT_Format;
 UWORD	FMT_Channels;
 ULONG	FMT_Samples_Per_Sec;
 ULONG	FMT_Avg_Bytes_Per_Sec;
 UWORD  FMT_BlockAlign;
/* only in ext format (see c->RC_size) */
 UWORD   PCMfmt_wBitsPerSample;  // Sample size
 UWORD   Exfmt_cbSize;
 UWORD   Exfmt_wValidBitsPerSample;
 ULONG   Exfmt_dwChannelMask;
 UWORD   Exfmt_SubFormat0;        // first two bytes are the ext. ID
 UWORD   Exfmt_SubFormat01;
 ULONG   Exfmt_SubFormat1;        // defined string, 12 bytes
 ULONG   Exfmt_SubFormat2;        //
 ULONG   Exfmt_SubFormat3;        //
} PACKATTR;

#if 0
struct WAV_CoefSet {
 struct RIFF_chunk c;
 WORD  WCS_coef1;
 WORD  WCS_coef2;
} PACKATTR;
#endif

/* proto */
struct wavload *Open_Wav( char *name );

struct wavload *Close_Wav( struct wavload *wav );

LONG Parse_Wav( struct wavload *wav );
#endif

/*
  in:  output from Open_Wav()
  out: NULL
*/
struct wavload *Close_Wav( struct wavload *wav )
{
	if( !wav )
		return NULL;

	if( wav->file )
	{
		FILE_CLOSE( wav->file );
	}
	if( wav->tmp_buffer )
		MFREE( wav->tmp_buffer );

	MFREE( wav );

	return NULL;
}


/*
  in:  input file path
  out: NULL in case of error, else struct describing wav file
*/
struct wavload *Open_Wav( char *name )
{
	struct wavload *ret = NULL;
	FH_T infile;

	infile = FILE_OPEN_R( name );
	if( !infile )
		return ret; /* sorry,cannot open file */

	ret = MALLOC( sizeof( struct wavload ) );
	if( !ret )
	{
		FILE_CLOSE( infile );
		return ret;
	}
	ret->file = infile;
	ret->tmp_buffer = NULL;
	ret->tmp_buffersize = 0;

	/* now parse file to get parameters */
	if( Parse_Wav(ret) < 0 )
		return Close_Wav( ret );

	return ret;
}

/* INTERNAL, DO NOT CALL EXTERNALLY

  in: pre-initialized wavload struct
*/
LONG Parse_Wav( struct wavload *wav )
{
	LONG  ret = 0;
	ULONG len;
	ULONG tmp,pos,tmp2,tmp3;

	/* header check */
	{
		struct WAV_RIFFHDR hdr;
		if( sizeof(hdr) != FILE_READ( &hdr, sizeof(hdr), wav->file ) )
			return -1;

		tmp = GETLONG_BE( hdr.c.RC_id );
		D(((STRPTR)"ID is 0x%lx\n",(long)tmp));
		if( tmp != ID_RIFF )
			return -2;

		tmp = GETLONG_BE( hdr.RIFF_format );
		D(((STRPTR)"FORMAT is 0x%lx\n",(long)tmp));
		if( tmp != ID_WAVE )
			return -3;

		len = GETLONG_LE( hdr.c.RC_size );
		D(((STRPTR)"Len is %ld (total %ld)\n",(long)len,(long)(len+8)));
		
		wav->wavsize = len+8; /* total file size: Length tag + 2 DWORDS (RC_id,RC_size) */
	}

	/* go through chunks, look for fmt and data */
	wav->data_offset = 0;
	wav->nchannels = 0;
	wav->nsamples = 0;
	wav->format   = 0;

	{
	 struct WAV_FormatExt fmt; /* make room for ext format but accept classic format chunk */

	 pos = FILE_SEEK_TELL( wav->file ); /* or just sizeof(struct WAV_RIFFHDR) */
	 while( pos < ( wav->wavsize - sizeof(struct RIFF_chunk) - 4 ) ) /* min 1 DWORD after chunk header */
	 {
	 	if( sizeof(struct RIFF_chunk) != FILE_READ( &fmt, sizeof(struct RIFF_chunk), wav->file ) )
		{
			D(((STRPTR)"Error: cannot read next chunk at %ld\n",(long)FILE_SEEK_TELL( wav->file ) ));
			break;
		}
		pos += sizeof(struct RIFF_chunk);

		tmp  = GETLONG_BE( fmt.c.RC_id   );
		len  = GETLONG_LE( fmt.c.RC_size );
		D(((STRPTR)"Chunk 0x%lx len %ld\n",(long)tmp,(long)len));
		if( len > wav->wavsize )
		{
			D(((STRPTR)"Illegal chunk length %lu\n",(unsigned long)len));
			break;
		}
		if( tmp == ID_fmt )
		{
			/* check if we can support the file's sample format */
			tmp2 = sizeof( struct WAV_FormatExt );
			if( tmp2 > len )
				tmp2 = len;
			len -= tmp2; /* remember that we've read at least a part of the fmt chunk */

			if( tmp2 != FILE_READ( sizeof(struct RIFF_chunk) + (char*)(&fmt), tmp2, wav->file ) )
			{
				D(((STRPTR)"Error: cannot read fmt chunk data with length %ld\n",(long)tmp2 ));
				break;
			}
			pos += tmp2;

			/* now check fmt */
			wav->rate = GETLONG_LE( fmt.FMT_Samples_Per_Sec );
			D(((STRPTR)"Sampling Rate %ld\n",(long)wav->rate));
			if( wav->rate > MAX_WAVE_RATE ) /* >512kHz or <0 */
			{
				break;
			}

			wav->depth = 8; /* default, unless we have ext fmt */
			tmp3 = GETSHORT_LE( fmt.FMT_Format );
			if( (tmp3 == WAV_FORMAT_EXTENSIBLE) && (tmp2 >= sizeof( struct WAV_FormatExt )-sizeof(struct RIFF_chunk) )  )
			{
				wav->depth = GETSHORT_LE( fmt.PCMfmt_wBitsPerSample );
			}
			else
			{
				if( tmp3 != WAV_FORMAT_PCM )
				{
					if( (tmp3 != WAV_FORMAT_MULAW) &&
					    (tmp3 != IBM_FORMAT_MULAW) &&
					    (tmp3 != IBM_FORMAT_ALAW) &&
					    (tmp3 != WAV_FORMAT_ALAW) 
					  )
					{
						/* WAV_FORMAT_MULAW,IBM_FORMAT_MULAW,IBM_FORMAT_ALAW,IBM_FORMAT_ADPCM,WAV_FORMAT_MPEG,WAV_FORMAT_MP3 */
						D(((STRPTR)"Only PCM supported, for now (format %ld)\n",(long)tmp3));
						wav->depth = 0;
						break;
					}
					else
					{
					 D(((STRPTR)"uLAW/ALAW file format %ld\n",(long)tmp3));
					}
				}
				if( tmp2 >= 16 )
				{
					tmp2 =	GETSHORT_LE( fmt.PCMfmt_wBitsPerSample );
					if( (tmp2==8)||(tmp2==16)||(tmp2==24)||(tmp2==32) )
						wav->depth = tmp2;
				}
			}

			wav->tmp_bufshift = 0;
			switch( wav->depth )
			{
				case 8:
					if( (tmp3 == WAV_FORMAT_MULAW) ||
					    (tmp3 == IBM_FORMAT_MULAW) )
					    tmp3 = MYWAV_FORMAT_ULAW;
					else
					{
					 if( (tmp3 == IBM_FORMAT_ALAW) ||
					     (tmp3 == WAV_FORMAT_ALAW) )
					    tmp3 = MYWAV_FORMAT_ALAW;
					 else
					    tmp3 = MYWAV_FORMAT_U8;
					}
					break;
				case 16:
					wav->tmp_bufshift = 1;
					tmp3 = MYWAV_FORMAT_S16;break;
				case 24:
					wav->tmp_bufshift = 2;
					tmp3 = MYWAV_FORMAT_S24;break;
				case 32:
					wav->tmp_bufshift = 2;
					tmp3 = MYWAV_FORMAT_S32;break;
				default:
					tmp3 = 0;break;
			}
			if( !tmp3 )
			{
				D(((STRPTR)"Error: only 8,16,32 Bit PCM supported here, Depth is %ld\n",(long)wav->depth));
				//wav->nchannels = 0;
				break;
			}
			D(((STRPTR)"Bits per Sample %ld\n",(long)wav->depth ));

			wav->format = tmp3;

			/* calculate number of samples (total) */
			tmp3 = GETSHORT_LE( fmt.FMT_BlockAlign );
			wav->BytesPerFrame = tmp3;
			D(((STRPTR)"Bytes per Frame %ld\n",(long)wav->BytesPerFrame ));

			/* remember number of active channels */
			wav->nchannels = GETSHORT_LE( fmt.FMT_Channels );
			D(((STRPTR)"NChannels %ld\n",(long)wav->nchannels));
			if( wav->nchannels > MAX_CHANNELS )
			{
				wav->nchannels = 0;
				break;
			}
			if( wav->nchannels > 1 )
				wav->tmp_bufshift++;

		}
		else
		{
		 if( tmp == ID_data )
		 {
			wav->data_offset = pos;
			wav->data_size = len;
			D(((STRPTR)"Data offset %ld length %ld\n",(long)pos,(long)len ));
		 }
		}
		
		if( len > 0 ) /* remaining chunk */
		{
		 FILE_SEEK_CUR( len, wav->file );
		 pos += len;
		}
	 }
	}

	/* final check: did we find useful data and fmt chunks ? */
	if( (wav->data_offset == 0) || (wav->nchannels == 0) )
		return -4;

	wav->nsamples = wav->data_size / wav->BytesPerFrame;

	wav->read_pos = 0;
	FILE_SEEK_START( wav->data_offset, wav->file );

	return ret;
}


