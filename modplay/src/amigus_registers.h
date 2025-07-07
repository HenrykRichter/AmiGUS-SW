/*
  AmiGUS register definitions
*/
#ifndef _INC_AMIGUS_H
#define _INC_AMIGUS_H

#include <exec/types.h>
#include "compiler.h" 

/* AmiGUS Zorro IDs */
#define AMIGUS_MANUFACTURER_ID	2782

#define	AMIGUS_MAIN_PRODUCT_ID	16
#define AMIGUS_HAGEN_PRODUCT_ID 17
#define AMIGUS_CODEC_PRODUCT_ID 18

#if 1
/* AmiGUS Main Register Definitions (relative to board address) */
struct AmiGUS_Main_Regs {
	UWORD INTC;
	UWORD INTE;
	UWORD SMPL_FMT;
	UWORD SMPL_RATE;
	UWORD FIFO_RES;
	UWORD FIFO_WTMK;
	ULONG FIFO_DATA;
	UWORD FIFO_USE;
	UWORD FIFO_VOL_L;
	UWORD FIFO_VOL_R;
	UWORD unused_1[5];
	//
	UWORD SPI_ADDRESS;
	UWORD SPI_WRITE_BYTE;
	UWORD SPI_WTRIG;
	UWORD SPI_RTRIG;
	UWORD SPI_READ_BYTE;
	UWORD SPI_STATUS;
	UWORD unused_2[2];
	//
	// TODO: mixer registers
}; // PACKATTR; /* disabled: the pack attribute will force undesired byte accesses with gcc6.5.0 */
#endif

#if 1
/* AmiGUS Hagen Register Definitions (relative to board address) */
struct AmiGUS_Hagen_Regs {
	volatile UWORD INTC0;
	volatile UWORD INTC1;
	volatile UWORD INTC2;
	volatile UWORD INTC3;
	volatile UWORD INTE0;
	volatile UWORD INTE1;
	volatile UWORD INTE2;
	volatile UWORD INTE3;
	/* */
	union {
		volatile UWORD WDATAHL[2];  /* 0x10 / 0x12 */
		volatile ULONG WDATA;
	};
	union {
		volatile UWORD WADDRHL[2];  /* 0x14 / 0x16 */
		volatile ULONG WADDR;
	};
	UWORD WRESET;		/* 0x18 */
	/* */
	UWORD	unused1[2]; /* 1a-1d */
	volatile UWORD	VOICE_BNK;	//0x1e
	/* */
	volatile UWORD	VOICE_CTRL;	//0x20
	union {
		volatile UWORD VOICE_PSTRTHL[2];  /* 0x22 / 0x24 */
		volatile ULONG VOICE_PSTRT;
	};
	union {
		volatile UWORD VOICE_PLOOPHL[2]; /* 0x26 / 0x28 */
		volatile ULONG VOICE_PLOOP;
	};
	union {
		volatile UWORD VOICE_PENDHL[2]; /* 0x2a / 0x2c */
		volatile ULONG VOICE_PEND;
	};
	union {
		volatile UWORD VOICE_RATEHL[2]; /* 0x2e / 0x30 */
		volatile ULONG VOICE_RATE;
	};
	volatile UWORD	VOICE_VOLUMEL;	//0x32
	volatile UWORD	VOICE_VOLUMER;	//0x34
	volatile UWORD	VOICE_EN_ATTACK; //0x36
	volatile UWORD	VOICE_EN_DECAY;  //0x38
	volatile UWORD	VOICE_EN_SUSTAIN;//0x3A
	volatile UWORD	VOICE_EN_RELEASE;//0x3C
	UWORD	unused2;         /* 3E */
	/* obsolete? */
	volatile UWORD	GLOBAL_VOLUMEL;	//0x40
	volatile UWORD	GLOBAL_VOLUMER;	//0x42
	UWORD   unused3[6];     /* 0x44-0x4F */

	UWORD   pad[0x50];	/* 0x50-0xEF */

	/* timer stuff */
	volatile UWORD	HT_CONTROL;	/* 0xF0 */
	union {
		volatile UWORD	HT_RELOADHL[2];	/* 0xF2/F4 */
		volatile ULONG  HT_RELOAD;
	      };
	union {
		volatile UWORD  HT_READHL[2]; /* 0xF6/F8 */
		volatile ULONG  HT_READ;
	};
	/* ... */
}; //PACKATTR;
#endif


#define	MAIN_INTC		0x00
#define MAIN_INTE		0x02
#define MAIN_SMPL_FMT		0x04
#define MAIN_SMPL_RATE		0x06
#define MAIN_FIFO_RES		0x08
#define MAIN_FIFO_WTMK		0x0a
#define MAIN_FIFO_DATA		0x0c
#define MAIN_FIFO_USE		0x10

/* AmiGUS Main Interrupt Flags */

#define INT_FLG_FIFO_EMPTY	0x1
#define INT_FLG_FIFO_FULL	0x2
#define INT_FLG_FIFO_WTMK	0x4
#define INT_FLG_SPI_FIN		0x8
#define INT_FLG_MASK_SET	0x8000

/* AmiGUS Main Sample Formats */

#define SMPL_FMT_MONO_8BIT		0x0
#define SMPL_FMT_STEREO_8BIT		0x1
#define SMPL_FMT_MONO_16BIT		0x2
#define SMPL_FMT_STEREO_16BIT		0x3
#define SMPL_FMT_MONO_24BIT		0x4
#define SMPL_FMT_STEREO_24BIT		0x5

#define SMPL_FMT_MONO_8BIT_SWP		0x8
#define SMPL_FMT_STEREO_8BIT_SWP	0x9
#define SMPL_FMT_MONO_16BIT_SWP		0xa
#define SMPL_FMT_STEREO_16BIT_SWP	0xb
#define SMPL_FMT_MONO_24BIT_SWP		0xc
#define SMPL_FMT_STEREO_24BIT_SWP	0xd

#define	SMPL_FMT_LITTLE_ENDIAN		0x10

/* AmiGUS Main Sample Rates */

#define SMPL_RATE_8000		0x0
#define SMPL_RATE_11025		0x1
#define SMPL_RATE_16000		0x2
#define SMPL_RATE_22050		0x3
#define SMPL_RATE_24000		0x4
#define SMPL_RATE_32000		0x5
#define SMPL_RATE_44100		0x6
#define SMPL_RATE_48000		0x7
#define SMPL_RATE_96000		0x8

/* Memory Buffers */
#define	FIFO_SIZE			4096			/* FIFO has 4096 * 16-bit words */
#define FIFO_WTMK			2048			/* FIFO watermark (default, the watermark is a variable) */


/* AmiGUS Hagen Register Definitions */
#define HAGEN_INTC0                     0x00
#define HAGEN_INTC1                     0x02
#define HAGEN_INTC2                     0x04
#define HAGEN_INTC3                     0x06
#define HAGEN_INTE0                     0x08
#define HAGEN_INTE1                     0x0a
#define HAGEN_INTE2                     0x0c
#define HAGEN_INTE3                     0x0e

#define HAGEN_WDATAH            0x10
#define HAGEN_WDATAL            0x12
#define HAGEN_WADDRH            0x14
#define HAGEN_WADDRL            0x16
#define HAGEN_WRESET            0x18

#define HAGEN_VOICE_BNK         0x1e
#define HAGEN_VOICE_CTRL        0x20
#define HAGEN_VOICE_PSTRTH      0x22
#define HAGEN_VOICE_PSTRTL      0x24
#define HAGEN_VOICE_PLOOPH      0x26
#define HAGEN_VOICE_PLOOPL      0x28
#define HAGEN_VOICE_PENDH       0x2a
#define HAGEN_VOICE_PENDL       0x2c
#define HAGEN_VOICE_RATEH       0x2e
#define HAGEN_VOICE_RATEL       0x30
#define HAGEN_VOICE_VOLUMEL     0x32
#define HAGEN_VOICE_VOLUMER     0x34

#define HAGEN_GLOBAL_VOLUMEL    0x40
#define HAGEN_GLOBAL_VOLUMER    0x42

/* Hagen control flags */

#define HAGEN_CTRLB_16BIT		0	/* R/W 0=8bit 1=16 Bit */
#define HAGEN_CTRLB_LOOP		1	/* R/W 0=one-shot, 1=loop */
#define HAGEN_CTRLB_INTERPOL		2	/* R/W 0=nearest neighbor, 1=linear interpol */
#define HAGEN_CTRLB_LITTLE_ENDIAN	3	/* W   0=Big Endian, 1=Little Endian */
#define HAGEN_CTRLB_ENVELOPE_MODULATE	5	/* W   0=No Envelope Modulation, 1=Use Envelope Modulation */
#define HAGEN_CTRLB_ENVELOPE_KEYON	14	/* W   0=Envelope Key off, 1=On */
#define HAGEN_CTRLB_PBSTART		15	/* R/W 0=Playback off, 1=Playback On */

#define HAGEN_CTRLF_16BIT		0x0001	/* R/W 0=8bit 1=16 Bit */
#define HAGEN_CTRLF_LOOP		0x0002	/* R/W 0=one-shot, 1=loop */
#define HAGEN_CTRLF_INTERPOL		0x0004	/* R/W 0=nearest neighbor, 1=linear interpol */
#define HAGEN_CTRLF_LITTLE_ENDIAN	0x0008	/* W   0=Big Endian, 1=Little Endian */
#define HAGEN_CTRLF_ENVELOPE_MODULATE	0x0020	/* W   0=No Envelope Modulation, 1=Use Envelope Modulation */
#define HAGEN_CTRLF_ENVELOPE_KEYON	0x4000	/* W   0=Envelope Key off, 1=On */
#define HAGEN_CTRLF_PBSTART		0x8000	/* R/W 0=Playback off, 1=Playback On */



#endif /* _INC_AMIGUS_H  */

