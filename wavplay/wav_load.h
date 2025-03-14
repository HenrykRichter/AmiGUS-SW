/*

  wav_load.h

  (C) 2025 Henryk Richter

  parser for RIFF WAVE files, 

  returns a structure outlining general parameters
  offers format conversion into a provided buffer

  ./wavload /Applications/Emu/Work/Mods/Mod.Ani_Birth.wav

 limitations:
  - no u-LAW,A-LAW,ADPCM or MPEG support
 
*/
#ifndef WAV_LOAD_H
#define WAV_LOAD_H

#include "compiler.h"
#include "fileio.h"

#ifndef EXEC_RESIDENT_H
#include <stdint.h>
typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int16_t WORD;
typedef int32_t LONG;
typedef uint8_t UBYTE;
typedef int8_t  BYTE;
#endif

#define MAX_CHANNELS 2
#define MAX_WAVE_RATE 0x80000 /* 512 kHz */

#define SWAPL(_a_) ( (((_a_)>>24)&0xFF)|(((_a_)>>8)&0xFF00)|(((_a_)<<8)&0xFF0000)|(((_a_)&0xFF)<<24) )
#define SWAPW(_a_) ((ULONG)( (((UWORD)(_a_)>>8)&0xFF)|((UWORD)((_a_)&0xff)<<8) ))

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
#define WAV_FORMAT_ALAW        0x006    /* M$ A-LAW */
#define WAV_FORMAT_MULAW       0x007    /* M$ u-LAW */
#define WAV_FORMAT_MPEG        0x050    /* MPEG-1 Layer 1,2 */
#define WAV_FORMAT_MP3         0x055    /* MPEG-1 Layer 3 */
#define WAV_FORMAT_EXTENSIBLE  0xfffe   /* new, extensible tag */
#define IBM_FORMAT_MULAW        0x101    /* IBM mu-law format */
#define IBM_FORMAT_ALAW         0x102    /* IBM a-law format */
#define IBM_FORMAT_ADPCM        0x103    /* IBM AVC Adaptive Differential Pulse Code Modulation format */
#define MYWAV_FORMAT_U8  0x8001
#define MYWAV_FORMAT_S16 0x8002
#define MYWAV_FORMAT_S24 0x8003
#define MYWAV_FORMAT_S32 0x8004
#define MYWAV_FORMAT_ULAW 0x8005
#define MYWAV_FORMAT_ALAW 0x8006


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

 ULONG read_pos;    /* internal: position in file */
 UBYTE *tmp_buffer; /* internal: buffer for conversion */
 ULONG  tmp_buffersize;
 ULONG  tmp_bufshift; 
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

/* Mono/Separate Buffer functions */

/* buffer reading, pass size of buffer in bytes (!NOT! WORDS) for consistency across calls */
LONG GetBuffer16_Wav( struct wavload *wav, WORD *bufferL, WORD *bufferR, ULONG buffersize );
/* buffer reading in 24 bit, uses byte buffers due to alignment woes */
LONG GetBuffer24_Wav( struct wavload *wav, UBYTE *bufferL, UBYTE *bufferR, ULONG buffersize );

/* Interleaved Stereo Buffer functions */

/* buffer reading, pass size of buffer in bytes (!NOT! WORDS) for consistency across calls */
LONG GetBuffer16Stereo_Wav( struct wavload *wav, WORD *bufferL, ULONG buffersize );
/* buffer reading, pass size of buffer in bytes (!NOT! WORDS) for consistency across calls */
LONG GetBuffer24Stereo_Wav( struct wavload *wav, UBYTE *bufferL, ULONG buffersize );



#endif /* WAV_LOAD_H */
