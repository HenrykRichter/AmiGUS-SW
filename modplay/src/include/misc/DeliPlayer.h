/**
 **  $Filename: misc/DeliPlayer.h $
 **  $Release: 2.0 $
 **  $Revision: 2.15$
 **  $Date: 22/06/95$
 **
 **  Definitions and Macros for creating DeliTracker Playermodules
 **
 **  (C) Copyright 1991, 1992, 1993, 1994, 1995 Delirium Softdesign
 **      All Rights Reserved
 **
 **/
/*
  This file is not the original DeliPlayer.h. There have been
  changes by H. Richter to possibly enable the use of this files
  with compilers other than SAS/C. Please note that any issues
  with this file should not be reported to the Delitracker authors.
  Blame me.
*/


#ifndef	DELITRACKER_PLAYER_H
#define DELITRACKER_PLAYER_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef INTUITION_SCREENS_H
#include <intuition/screens.h>
#endif

#include <misc/compiler.h>

/* --------------------------------------------------------------------------- */

#define DELIVERSION 17			/* Current Version of DeliTracker */
#define DELIREVISION 4			/* Current Revision of DeliTracker */

/* -------------- The declarations for the interface functions --------------- */

extern ASM APTR dt_GetListDataAddr(ASMR(d0) ULONG num ASMREG(d0));
extern ASM ULONG dt_GetListDataSize(ASMR(d0) ULONG num ASMREG(d0));
extern ASM BOOL dt_LoadFile(void);
extern ASM void dt_CopyDir(void);
extern ASM void dt_CopyFile(void);
extern ASM void dt_CopyString(ASMR(a0) STRPTR string ASMREG(a0));
extern ASM BOOL dt_AudioAlloc(void);
extern ASM void dt_AudioFree(void);
extern ASM void dt_StartInt(void);
extern ASM void dt_StopInt(void);
extern ASM void dt_SongEnd(void);
extern ASM void dt_CutSuffix(void);
extern ASM void dt_SetTimer(void);
extern ASM void dt_WaitAudioDMA(void);
extern ASM struct Screen *dt_LockScreen(void);
extern ASM void dt_UnlockScreen(void);
extern ASM void dt_NotePlayer(void);
extern ASM APTR dt_AllocListData(ASMR(d0) ULONG size ASMREG(d0), ASMR(d1) ULONG memflags ASMREG(d1));
extern ASM void dt_FreeListData(ASMR(a1) APTR memblock ASMREG(a1));

/* ------------------------ Player Function Offsets -------------------------- */

struct DeliTrackerPlayer {
	ULONG RTS_code;
	UBYTE ID[8];
	struct TagItem *TagArray;
};

/* The TagItem ID's (ti_Tag values) for the player interface follow. */

#define DTP_Dummy	(TAG_USER + 0x4454)

#define DTP_InternalPlayer	(DTP_Dummy)		/* obsolete */
#define DTP_CustomPlayer	(DTP_Dummy + 1)		/* player is a customplayer */

#define DTP_RequestDTVersion	(DTP_Dummy + 2)		/* minimum DeliTracker version needed */
#define DTP_RequestKickVersion	(DTP_Dummy + 3)		/* minimum KickStart version needed */

#define DTP_PlayerVersion	(DTP_Dummy + 4)		/* actual player version & revision */
#define DTP_PlayerName		(DTP_Dummy + 5)		/* name of this player */
#define DTP_Creator		(DTP_Dummy + 6)		/* misc string */

#define DTP_Check1		(DTP_Dummy + 7)		/* Check Format before loading */
#define DTP_Check2		(DTP_Dummy + 8)		/* Check Format after file is loaded */
#define DTP_ExtLoad		(DTP_Dummy + 9)		/* Load additional files */
#define DTP_Interrupt		(DTP_Dummy + 10)	/* Interrupt routine */
#define DTP_Stop		(DTP_Dummy + 11)	/* Clear Patterncounter */
#define DTP_Config		(DTP_Dummy + 12)	/* Config Player */
#define DTP_UserConfig		(DTP_Dummy + 13)	/* User-Configroutine */
#define DTP_SubSongRange	(DTP_Dummy + 14)	/* Get min&max subsong number */

#define DTP_InitPlayer		(DTP_Dummy + 15)	/* Initialisize the Player */
#define DTP_EndPlayer		(DTP_Dummy + 16)	/* Player clean up */
#define DTP_InitSound		(DTP_Dummy + 17)	/* Soundinitialisation routine */
#define DTP_EndSound		(DTP_Dummy + 18)	/* End sound */
#define DTP_StartInt		(DTP_Dummy + 19)	/* Start interrupt */
#define DTP_StopInt		(DTP_Dummy + 20)	/* Stop interrupt */

#define DTP_Volume		(DTP_Dummy + 21)	/* Set Volume */
#define DTP_Balance		(DTP_Dummy + 22)	/* Set Balance */
#define DTP_Faster		(DTP_Dummy + 23)	/* Incease playspeed */
#define DTP_Slower		(DTP_Dummy + 24)	/* Decrease playspeed */
#define DTP_NextPatt		(DTP_Dummy + 25)	/* Jump to next pattern */
#define DTP_PrevPatt		(DTP_Dummy + 26)	/* Jump to previous pattern */
#define DTP_NextSong		(DTP_Dummy + 27)	/* Play next subsong */
#define DTP_PrevSong		(DTP_Dummy + 28)	/* Play previous subsong */

	/*--- functions in revision 14 or higher (distributed as Release 1.35) --- */

#define DTP_SubSongTest		(DTP_Dummy + 29)	/* Test, if given subsong is vaild */

	/*--- functions in revision 16 or higher (distributed as Release 2.01) --- */

#define DTP_NewSubSongRange	(DTP_Dummy + 30)	/* enhanced replacement for DTP_SubSongRange */

#define DTP_DeliBase		(DTP_Dummy + 31)	/* the address of a pointer where DT */
												/* stores a pointer to the DeliGlobals */

#define DTP_Flags		(DTP_Dummy + 32)	/* misc Flags (see below) */

#define DTP_CheckLen		(DTP_Dummy + 33)	/* Length of the Check Code */

#define DTP_Description		(DTP_Dummy + 34)	/* misc string */

#define DTP_Decrunch		(DTP_Dummy + 35)	/* pointer to Decrunch Code */
#define DTP_Convert		(DTP_Dummy + 36)	/* pointer to Converter Code */

#define DTP_NotePlayer		(DTP_Dummy + 37)	/* pointer to a NotePlayer Structure */
#define DTP_NoteStruct		(DTP_Dummy + 38)	/* the address of a pointer to the */
												/* NoteStruct Structure */
#define DTP_NoteInfo		(DTP_Dummy + 39)	/* a pointer where DT stores a pointer */
												/* to the current NoteStruct Structure */
#define DTP_NoteSignal		(DTP_Dummy + 40)	/* pointer to NoteSignal code */

#define DTP_Process		(DTP_Dummy + 41)	/* pointer to process entry code */
#define DTP_Priority		(DTP_Dummy + 42)	/* priority of the process */
#define DTP_StackSize		(DTP_Dummy + 43)	/* stack size of the process */
#define DTP_MsgPort		(DTP_Dummy + 44)	/* a pointer where DT stores a pointer */
												/* to a port to send its messages */

#define DTP_Appear		(DTP_Dummy + 45)	/* open your window, if you can */
#define DTP_Disappear		(DTP_Dummy + 46)	/* go dormant */

#define DTP_ModuleName		(DTP_Dummy + 47)	/* get the name of the current module */
#define DTP_FormatName		(DTP_Dummy + 48)	/* get the name of the module format */
#define DTP_AuthorName		(DTP_Dummy + 49)	/* not implemented yet */

	/*--- functions in revision 17 or higher (distributed as Release 2.07) --- */

#define DTP_InitNote		(DTP_Dummy + 50)	/* NoteStruct initialization */

#define	DTP_NoteAllocMem	(DTP_Dummy + 51)	/* allocates memory for module */

#define	DTP_NoteFreeMem		(DTP_Dummy + 52)	/* frees module-memory */

#define	DTP_PlayerInfo		(DTP_Dummy + 53)	/* a pointer where DT stores a pointer */
							/* to the current Player Taglist */

#define	DTP_Patterns		(DTP_Dummy + 54)	/* FPTR to a pattern-count routine */

#define	DTP_Duration		(DTP_Dummy + 55)	/* FPTR to a duration calc routine */

#define	DTP_SampleData		(DTP_Dummy + 56)	/* FPTR to a sample-info routine */

#define	DTP_MiscText		(DTP_Dummy + 57)	/* FPTR to a misc-text routine */

/* end of player interface enumeration */


/* --- various flags --------------------------------------------------------- */

#define PLYB_CUSTOM 0		/* player is a customplayer */
#define PLYF_CUSTOM (1<<0)
#define PLYB_SONGEND 1		/* player supports songend */
#define PLYF_SONGEND (1<<1)

	/*--- flags defined in revision 17 or higher (distributed as Release 2.07) --- */

#define PLYB_ANYMEM 2		/* modules of this player don't require chipmem */
#define PLYF_ANYMEM (1<<2)

/* --- DeliTracker message --------------------------------------------------- */

struct DeliMessage {
	struct Message Message;
	ULONG (*Function)(void);
	ULONG Result;
	ULONG	DTMN_ArgumentD0;
	ULONG	DTMN_ArgumentA0;
};

/* --- DeliTracker NotePlayer ------------------------------------------------ */

struct NotePlayer {
	UWORD	npl_LeftChannels;	/* max. number of left channels this noteplayer supports */
	UWORD	npl_RightChannels;	/* max. number of right channels this noteplayer supports */
	ULONG	npl_Flags;		/* misc flags (see below) */
	ULONG	npl_MaxFrequency;	/* max. frequency this noteplayer supports (-1 if unlimited) */
	ULONG	npl_Memory;		/* memory type for samples (e.g. MEMF_CHIP) */
	UBYTE	npl_Reserved[16];	/* reserved for future use (must be 0 for now) */
};

#define	NPLB_CPUintensive 0		/* the NotePlayer needs much cpu-power */
#define	NPLF_CPUintensive (1<<0)	/* the NotePlayer needs much cpu-power */

#define	NPLB_Reverse 8			/* little endian byte ordering */
#define	NPLF_Reverse (1<<8)		/* little endian byte ordering */
#define	NPLB_Signed 9			/* signed linear (2's complement) samples supported */
#define	NPLF_Signed (1<<9)		/* signed linear (2's complement) samples supported */
#define	NPLB_Unsigned 10		/* unsigned linear samples supported */
#define	NPLF_Unsigned (1<<10)	/* unsigned linear samples supported */
#define	NPLB_Ulaw 11			/* U-law (logarithmic) samples supported */
#define	NPLF_Ulaw (1<<11)		/* U-law (logarithmic) samples supported */
#define	NPLB_Alaw 12			/* A-law (logarithmic) samples supported */
#define	NPLF_Alaw (1<<12)		/* A-law (logarithmic) samples supported */
#define	NPLB_Float 13			/* IEEE float samples supported */
#define	NPLF_Float (1<<13)		/* IEEE float samples supported */

#define	NPLB_7Bit 16			/* 7-bit samples supported */
#define	NPLF_7Bit (1<<16)		/* 7-bit samples supported */
#define	NPLB_8Bit 17			/* byte samples supported */
#define	NPLF_8Bit (1<<17)		/* byte samples supported */
#define	NPLB_16Bit 18			/* 16-bit word samples supported */
#define	NPLF_16Bit (1<<18)		/* 16-bit word samples supported */
#define	NPLB_24Bit 19			/* 24-bit data samples supported */
#define	NPLF_24Bit (1<<19)		/* 24-bit data samples supported */
#define	NPLB_32Bit 20			/* longword samples supported */
#define	NPLF_32Bit (1<<20)		/* longword samples supported */
#define	NPLB_64Bit 21			/* quadword samples supported */
#define	NPLF_64Bit (1<<21)		/* quadword samples supported */

#define	NPLD_TypeMask (NPLF_Reverse|NPLF_Signed|NPLF_Unsigned|NPLF_Ulaw|NPLF_Alaw|NPLF_Float)
#define	NPLD_SizeMask (NPLF_7Bit|NPLF_8Bit|NPLF_16Bit|NPLF_24Bit|NPLF_32Bit|NPLF_64Bit)


struct NoteStruct {
	APTR	nst_Channels;		/* pointer to a list of notechannels */
	ULONG	nst_Flags;		/* misc flags (see below) */
	ULONG	nst_MaxFrequency;	/* max. frequency of this player (28,867 Hz in DMA mode) */
	UWORD	nst_MaxVolume;		/* max. volume of this player (in most cases 64) */
	UBYTE	nst_Reserved[18];	/* reserved for future use (must be 0 for now) */
};

#define	NSTB_Dummy 0			/* only a dummy-NoteStruct (no NotePlayer needed) */
#define	NSTF_Dummy (1<<0)
#define	NSTB_Period 1			/* Amiga period supplied instead of frequency */
#define	NSTF_Period (1<<1)
#define	NSTB_ExtPeriod 2		/* Extended period (period*4) supplied instead of frequency */
#define	NSTF_ExtPeriod (1<<2)
#define	NSTB_NTSCTiming 3		/* Period/ExtPeriod supplied in NTSC instead of PAL */
#define	NSTF_NTSCTiming (1<<3)
#define	NSTB_EvenLength 4		/* Samplelength supplied as WORD instead of LONG */
#define	NSTF_EvenLength (1<<4)
#define	NSTB_AllRepeats 5		/* play Repeats even if no One-Shot part was played yet */
#define	NSTF_AllRepeats (1<<5)

#define	NSTB_Reverse 8			/* little endian byte ordering */
#define	NSTF_Reverse (1<<8)
#define	NSTB_Signed 9			/* sample data is signed linear (2's complement) */
#define	NSTF_Signed (1<<9)
#define	NSTB_Unsigned 10		/*       -"-      unsigned linear */
#define	NSTF_Unsigned (1<<10)
#define	NSTB_Ulaw 11			/*       -"-      U-law (logarithmic) */
#define	NSTF_Ulaw (1<<11)
#define	NSTB_Alaw 12			/*       -"-      A-law (logarithmic) */
#define	NSTF_Alaw (1<<12)
#define	NSTB_Float 13			/*       -"-      IEEE floats */
#define	NSTF_Float (1<<13)

#define	NSTB_7Bit 16			/* sample data is in 7-bit format */
#define	NSTF_7Bit (1<<16)
#define	NSTB_8Bit 17			/*        -"-        bytes */
#define	NSTF_8Bit (1<<17)
#define	NSTB_16Bit 18			/*        -"-        16-bit words */
#define	NSTF_16Bit (1<<18)
#define	NSTB_24Bit 19			/*        -"-        24-bit data */
#define	NSTF_24Bit (1<<19)
#define	NSTB_32Bit 20			/*        -"-        longwords */
#define	NSTF_32Bit (1<<20)
#define	NSTB_64Bit 21			/*        -"-        quadwords */
#define	NSTF_64Bit (1<<21)

#define	NSTD_TypeMask (NSTF_Reverse|NSTF_Signed|NSTF_Unsigned|NSTF_Ulaw|NSTF_Alaw|NSTF_Float)
#define	NSTD_SizeMask (NSTF_7Bit|NSTF_8Bit|NSTF_16Bit|NSTF_24Bit|NSTF_32Bit|NSTF_64Bit)


struct NoteChannel {
	APTR	nch_NextChannel;	/* 0  next channel in the list (NULL if last) */
	ULONG	nch_NotePlayer;		/* 4  for use by the noteplayer (the deliplayer must ignore this) */
	WORD	nch_Reserved0;		/* 8  reserved for future use (must be 0 for now) */
	UBYTE	nch_Private;		/* 10 just what it says */
	UBYTE	nch_Changed;		/* 11 what has changed since last call */
	WORD	nch_StereoPos;		/* 12 set this field when the InitNote function is called */
	WORD	nch_Stereo;		/* 14 describes "where" this channel is supposed to play */
	APTR	nch_SampleStart;	/* 16 ^sampledata */
	ULONG	nch_SampleLength;	/* 20 size of sample */
	APTR	nch_RepeatStart;	/* 24 ^repeat part of sample */
	ULONG	nch_RepeatLength;	/* 28 size of repeat part */
	ULONG	nch_Frequency;		/* 32 frequency (or period) of sample */
	UWORD	nch_Volume;		/* 36 volume of sample */
	UBYTE	nch_Reserved1[26];	/* 38 reserved for future use (must be 0 for now) */
};

#define	NCHB_Stereo 0			/* pan position of channel has changed (???) */
#define	NCHF_Stereo (1<<0)
#define	NCHB_Sample 1			/* one-shot part of sample has changed */
#define	NCHF_Sample (1<<1)
#define	NCHB_Repeat 2			/* repeat part of sample has changed */
#define	NCHF_Repeat (1<<2)
#define	NCHB_Frequency 3		/* frequency has changed */
#define	NCHF_Frequency (1<<3)
#define	NCHB_Volume 4			/* volume (or pan position ???) has changed */
#define	NCHF_Volume (1<<4)
#define	NCHB_Trigger 5			/* trigger sample */
#define	NCHF_Trigger (1<<5)
#define	NCHB_Restart 6			/* restart sample */
#define	NCHF_Restart (1<<6)

#define	NCHB_Loop 16			/* sample has looped at least once (set by DeliTracker) */
#define	NCHF_Loop (1<<16)

#define	NCHB_Private0 8			/* private internal use! */
#define	NCHF_Private0 (1<<8)
#define	NCHB_Private1 9			/* private internal use! */
#define	NCHF_Private1 (1<<9)


#define	NCHD_Ignore -32768		/* ignore this notechannel */
#define	NCHD_Balanced 0			/* play balanced on both sides */
#define	NCHD_FarLeft -32767		/* play only on left speaker */
#define	NCHD_FarRight +32767		/* play only on right speaker */


/* ---------------------------- Global Variables ------------------------------ */

struct DeliTrackerGlobals {

	/* ------ if you use dtg_AslBase, make sure that */
	/* ------ DTP_RequestDTVersion is at least 13 ! */

	struct Library *AslBase;		/* library base, don't CloseLibrary()!! */

	struct Library *DOSBase;		/* library base -"- */
	struct IntuitionBase *IntuitionBase;	/* library base -"- */
	struct GfxBase *GfxBase;		/* library base -"- */
	struct Library *GadToolsBase;		/* librarybase -"- (NULL for Kick 1.3 and below) */
	APTR ReservedLibraryBase;		/* reserved for future use */

	STRPTR	DirArrayPtr;		/* Ptr to the directory of the current module */
	STRPTR	FileArrayPtr;		/* Ptr to the filename of the current module */
	STRPTR	PathArrayPtr;		/* Ptr to PathArray (e.g used in LoadFile()) */

	APTR	ChkData;		/* pointer to the module to be checked */
	ULONG	ChkSize;		/* size of the module */

	UWORD	SndNum;			/* current sound number */
	UWORD	SndVol;			/* volume (ranging from 0 to 64) */
	UWORD	SndLBal;		/* left volume (ranging from 0 to 64) */
	UWORD	SndRBal;		/* right volume (ranging from 0 to 64) */
	UWORD	LED;			/* filter (0 if the LED is off) */
	UWORD	Timer;			/* timer-value for the CIA-Timers */

	APTR	dtg_GetListData;
	APTR	dtg_LoadFile;
	APTR	dtg_CopyDir;
	APTR	dtg_CopyFile;
	APTR	dtg_CopyString;
	APTR	dtg_AudioAlloc;
	APTR	dtg_AudioFree;
	APTR	dtg_StartInt;
	APTR	dtg_StopInt;
	APTR	dtg_SongEnd;		/* safe to call from interrupt code ! */
	APTR	dtg_CutSuffix;

	/* ------ extension in revision 14 */

	APTR	dtg_SetTimer;		/* safe to call from interrupt code ! */

	/* ------ extension in revision 15 */

	APTR	dtg_WaitAudioDMA;	/* safe to call from interrupt code ! */

	/* ------ extension in revision 16 */

	APTR	dtg_LockScreen;
	APTR	dtg_UnlockScreen;
	APTR	dtg_NotePlayer;		/* safe to call from interrupt code ! */
	APTR	dtg_AllocListData;
	APTR	dtg_FreeListData;

	APTR	dtg_Reserved1;		/* do not use !!! */
	APTR	dtg_Reserved2;		/* do not use !!! */
	APTR	dtg_Reserved3;		/* do not use !!! */
};

#endif
