/*
**
**  $Filename: misc/EaglePlayer.h
**  $Release: 2.02 $
**  $Revision: 16$
**  $Date: 07/05/98$
**
** Definitions and Macros for creating EaglePlayer Player-/Enginesmodules
**
**	(C) Copyright 1993-2018 by DEFECT
**	    All Rights Reserved
**
**
*/
#ifndef _INC_EAGLEPLAYER_H
#define _INC_EAGLEPLAYER_H

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif

#include <misc/DeliPlayer.h>

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif


#define EAGLEVERSION	14		/* Current Version of EaglePlayer */
/* 9=V1.53/54 */
/* 10=1.55 Alpha */
/* 11=2.00 */
/* 12=2.01 */
/* 13=2.02 */
/* 14=2.07 */

/*--------------------------- New EaglePlayer Tags ----------------------------*/
#define EP_TagBase	TAG_USER+17744	/* TAG_USER+"EP" */

#define	EP_Get_ModuleInfo (EP_TagBase+0)	/* get Pointer to Moduleinfo Taglist */
#define	EP_Free_ModuleInfo (EP_TagBase+1)	/* free Taglist (e.g. Free Mem) */
#define	EP_Voices	  (EP_TagBase+2)	/* Set the Voices */
#define	EP_SampleInit	  (EP_TagBase+3)	/* create Sampleinfostructure */
#define	EP_SampleEnd	  (EP_TagBase+4)	/* ejected !!! */
#define	EP_Save		  (EP_TagBase+5)	/* */
#define	EP_ModuleChange	  (EP_TagBase+6)	/* Change Module */
#define	EP_ModuleRestore  (EP_TagBase+7)	/* Restore Module */
#define	EP_StructInit	  (EP_TagBase+8)	/* Return Pointer to Userprg.Struct (UPS_) */
#define	EP_StructEnd	  (EP_TagBase+9)	/* Free Struct (e.g. Free allocated Mem) */
#define	EP_LoadPlConfig	  (EP_TagBase+10)	/* Load Config of Player */
#define	EP_SavePlConfig	  (EP_TagBase+11)	/* Save Config of Player */
#define	EP_GetPositionNr  (EP_TagBase+12	/* return Patternnumber (in D0).l) */
#define	EP_SetSpeed	  (EP_TagBase+13)	/* Set Speed (for Players with own Timer) */
					/* Value submitted in D0 */
#define	EP_Flags	  (EP_TagBase+14)	/* see below */
#define	EP_KickVersion	  (EP_TagBase+15)	/* min Kickstartversion */
#define	EP_PlayerVersion  (EP_TagBase+16)	/* min EP-Version */
#define	EP_CheckModule	  (EP_TagBase+17)	/* Checks the module (d0)=Error or zero) */
#define	EP_EjectPlayer	  (EP_TagBase+18)	/* */
#define	EP_Date		  (EP_TagBase+19)	/* Creator-Date (e.g 11.05.1993) */
					/* 11.b, 05.b, 1993.w ) */
#define	EP_Check3	  (EP_TagBase+20)	/* */
#define	EP_SaveAsPT	  (EP_TagBase+21)	/* Save Module as Protrackermodule */

#define	EP_NewModuleInfo  (EP_TagBase+22)	/* TI_Data -> Pointer to Moduleinfo Taglist */
#define	EP_FreeExtLoad	  (EP_TagBase+23)	/* */
#define	EP_PlaySample	  (EP_TagBase+24)	/* Play Sample d0)=SampleNr */
#define	EP_PatternInit	  (EP_TagBase+25)	/* Init Patterninfostruct */
#define	EP_PatternEnd	  (EP_TagBase+26)	/* Free Patterninfostruct - optional */
#define	EP_Check4	  (EP_TagBase+27)	/* */
#define	EP_Check5	  (EP_TagBase+28)	/* */
#define	EP_Check6	  (EP_TagBase+29)	/* */

#define	EP_CreatorLNr	  (EP_TagBase+30)	/* */
#define	EP_PlayerNameLNr  (EP_TagBase+31)	/* */

#define	EP_PlayerInfo	  (EP_TagBase+32)	/* TI_Data -> APTR to TagList */
#define	EP_PlaySampleInit (EP_TagBase+33)	/* */
#define	EP_PlaySampleEnd  (EP_TagBase+34)	/* */

#define	EP_InitAmplifier  (EP_TagBase+35)	/* Input:  None */
					/* Output: d0=NUll oder Error */
					/*   Tagliste in EPGL_AmplifierTagList */

#define	EP_CheckSegment	  (EP_TagBase+36)	/* */
#define	EP_Show		  (EP_TagBase+37)	/* */
#define	EP_Hide		  (EP_TagBase+38)	/* */
#define	EP_LocaleTable	  (EP_TagBase+39)	/* */
#define	EP_Helpnodename	  (EP_TagBase+40)	/* */
#define	EP_AttnFlags	  (EP_TagBase+41)	/* */
#define	EP_EagleBase	  (EP_TagBase+42)	/* */
#define	EP_Check7	  (EP_TagBase+43)	/* for Formatloader (DTP_Check1) a0=Formattags */
#define	EP_Check8	  (EP_TagBase+44)	/* for Formatloader (DTP_Check2) a0=Formattags */
#define	EP_SetPlayFrequency (EP_TagBase+45)	/* */
#define	EP_SamplePlayer	  (EP_TagBase+46)	/* */
#define	EP_SetPosition	  (EP_TagBase+47)	/* To set Moduleprosition (Protracker,MPEG) used in EagleAMP */

/*********** end of player interface enumeration for EaglePlayer ***********/



/*------------------------ EaglePlayer EP_Flags --------------------------*
 *    if you use EP_Flags, please set all functions your player offers    */

#define EPF_Songend		1			/* Songend */
#define EPF_Restart		2			/* Player restartable */
#define EPF_Disable		3			/* Player disabled (1=yes) */
#define EPF_NextSong		4
#define EPF_PrevSong		5
#define EPF_NextPatt		6
#define EPF_PrevPatt		7
#define EPF_Volume		8
#define EPF_Balance		9
#define EPF_Voices		10
#define EPF_Save		11
#define EPF_Analyzer		12
#define EPF_ModuleInfo		13
#define EPF_SampleInfo		14
#define EPF_Packable		15
#define EPF_VolVoices		16
#define EPF_InternalUPSStructure 	17
#define EPF_RestartSong		18
#define EPF_LoadFast		19
#define EPF_EPAudioAlloc 	20
#define EPF_VolBalVoi		21
#define EPF_CalcDuration 	22

#define EPB_Songend		(1<<EPF_Songend)
#define EPB_Restart		(1<<EPF_Restart)
#define EPB_Disable		(1<<EPF_Disable)
#define EPB_NextSong		(1<<EPF_NextSong)
#define EPB_PrevSong		(1<<EPF_PrevSong)
#define EPB_NextPatt		(1<<EPF_NextPatt)
#define EPB_PrevPatt		(1<<EPF_PrevPatt)
#define EPB_Volume		(1<<EPF_Volume)
#define EPB_Balance		(1<<EPF_Balance	)
#define EPB_Voices		(1<<EPF_Voices)
#define EPB_Save		(1<<EPF_Save)
#define EPB_Analyzer		(1<<EPF_Analyzer)
#define EPB_ModuleInfo		(1<<EPF_ModuleInfo)
#define EPB_SampleInfo		(1<<EPF_SampleInfo)
#define EPB_Packable		(1<<EPF_Packable)
#define EPB_VolVoices		(1<<EPF_VolVoices)
#define EPB_InternalUPSStructure  (1<<EPF_InternalUPSStructure)
#define EPB_RestartSong		(1<<EPF_RestartSong)
#define EPB_LoadFast		(1<<EPF_LoadFast)
#define EPB_EPAudioAlloc 	(1<<EPF_EPAudioAlloc)
#define EPB_VolBalVoi		(1<<EPF_VolBalVoi)
#define EPB_CalcDuration 	(1<<EPF_CalcDuration)

/*---------------------------- Module-Info Tags -------------------------------*/
#define MI_TagBase	TAG_USER+19785		/* TAG_USER+"MI" */

#define	MI_SongName		 (MI_TagBase+0)	/* Pointer to Songname */
#define	MI_AuthorName		 (MI_TagBase+1)	/* Pointer to Authorname */
#define	MI_SubSongs		 (MI_TagBase+2)	/* Subsongs */
#define	MI_Pattern		 (MI_TagBase+3)	/* highest PatternNr */
#define	MI_MaxPattern		 (MI_TagBase+4)	/* Max Pattern (SoundTr. 64)) */
#define	MI_Length		 (MI_TagBase+5)	/* Length of Module */
#define	MI_MaxLength		 (MI_TagBase+6)	/* Max Length (SoundTr. 128)) */
#define	MI_Steps		 (MI_TagBase+7)	/* Steps (SoundMon) */
#define	MI_MaxSteps		 (MI_TagBase+8)	/* Max Steps */
#define	MI_Samples		 (MI_TagBase+9)	/* Number of used Samples */
#define	MI_MaxSamples		 (MI_TagBase+10	/* Max Samples (SoundTr 31)) */
#define	MI_SynthSamples		 (MI_TagBase+11)	/* Number of used SynthSamples */
#define	MI_MaxSynthSamples	 (MI_TagBase+12)	/* Max SynthSamples */
#define	MI_Songsize		 (MI_TagBase+13)	/* Songsize in Bytes */
#define	MI_SamplesSize		 (MI_TagBase+14)	/* Samplessize in Bytes */
#define	MI_ChipSize		 (MI_TagBase+15)	/* needed Chip-Size in Bytes */
#define	MI_OtherSize		 (MI_TagBase+16)	/* FastRam (MDAT-Files) in Bytes */
#define	MI_Calcsize		 (MI_TagBase+17)	/* calculated length of module in Bytes */
#define	MI_SpecialInfo		 (MI_TagBase+18)	/* Pointer to general InfoText  */
						/* Null Terminated String, may */
						/* countain linefeeds */
#define	MI_LoadSize		 (MI_TagBase+19)	/* Loadsize (TFMX,Startrekker) in Bytes */
#define	MI_Unpacked		 (MI_TagBase+20)	/* Unpacked Size (e.g. Noise- */
						/* Packer) in Bytes */
#define	MI_UnPackedSystem	 (MI_TagBase+21)	/* MIUS (see below) or APTR */
						/* to Soundsysname */
#define	MI_Prefix		 (MI_TagBase+22)	/* Save-Prefix (e.g. "MOD." or */
						/* "Mdat.") */
#define	MI_About		 (MI_TagBase+23)	/* TI_Data points on a string */
						/* telling something about this */
						/* Soundsystem,  */
						/* Null Terminated String, may */
						/* countain linefeeds */
#define	MI_MaxSubSongs		 (MI_TagBase+24)	/* Max SubSongs */
#define	MI_Voices		 (MI_TagBase+25)	/* used Voices */
#define	MI_MaxVoices		 (MI_TagBase+26)	/* Max Voices */
#define	MI_UnPackedSongSize	 (MI_TagBase+27)	/* for converted modules */
#define	MI_Duration		 (MI_TagBase+28)	/* Duration */
#define	MI_Soundsystem		 (MI_TagBase+29)	/* Soundsystem Name */
						/* (for multiple players) */
#define	MI_PlayFrequency	 (MI_TagBase+30)	/* Mixingfrequency */
#define	MI_Volumeboost		 (MI_TagBase+31)	/* Volume Boost */
#define	MI_Playmode		 (MI_TagBase+32)	/* Playing Mode */
						/* (mono,stereo,14 bit etc. */
						/* as STRING !!) */
#define	MI_ExtraInfo		 (MI_TagBase+33)	/* One More Field for Special */
						/* informations,TI_Data points */
						/* onto a string telling  */
						/* something more or less useful */
						/* Null Terminated String, may */
						/* countain linefeeds */
#define	MI_InfoFlags		 (MI_TagBase+34)	/* Flags */
#define	MI_AlbumName		 (MI_TagBase+35)	/* name of the Album containing this song (string) */
#define	MI_Year			 (MI_TagBase+36)	/* date this song was created (string) */
#define	MI_Comment		 (MI_TagBase+37)	/* string */
#define	MI_Genre		 (MI_TagBase+38)	/* string */
#define	MI_Bitrate		 (MI_TagBase+39)	/* LongInt, e.g. .mp3) Bitrate */

/*--------- Unpacked Soundsystems (intern), more follow --------------------*/
#define MIUS_OldSoundTracker		1
#define MIUS_SoundTracker		2
#define MIUS_NoiseTracker		3
#define MIUS_ProTracker			4
#define MIUS_SizeOF			4

/*------------------------------ MI_Flags ----------------------------------*/
#define MIF_ReplayinModule		1
#define MIB_ReplayinModule		(1<<MIF_ReplayinModule)


/*------------------------------ Sample-Info Tags --------------------------*/

struct EP_Sampletable {
	APTR	EPS_NextSample;			/*  Next SampleTag-Structure */
	APTR	EPS_SampleName;			/*  Name of Sample */
	ULONG	EPS_Adr;			/*  Adr of sample */
	ULONG	EPS_Length;			/*  Length of sample */
	ULONG	EPS_Repeat;			/*  Repeat */
	ULONG	EPS_Replen;			/*  Replen */
	ULONG	EPS_Period;			/*  default Sampleperiod */
	ULONG	EPS_Volume;			/*  default Volume */
	UWORD	EPS_Finetune;			/*  Finetune */
	UWORD	EPS_Type;			/*  SampleType */
	UWORD	EPS_Flags;			/*  Flags for this Samplestructure */
	UWORD	EPS_MaxNameLen;			/*  max Namelength */
};	/*  to be extended !!!!!! */


/*------------------------------- Sample types ------------------------------*/
#define USITY_RAW			0
#define USITY_IFF			1
#define USITY_FMSynth			2
#define USITY_AMSynth			3
#define USITY_Unknown			4

/*--------------- Sample-Flags for one SampleInfo-TagStructure --------------*/
#define USIF_Playable			0
#define USIF_Saveable			1
#define USIF_8Bit			2
#define USIF_16Bit			3
#define USIF_Interleaved		4
#define USIF_Intel			5
#define USIF_Unsigned			6

#define USIB_Playable			(1<<USIF_Playable)
#define USIB_Saveable			(1<<USIF_Saveable)
#define USIB_8Bit			(1<<USIF_8Bit)
#define USIB_16Bit			(1<<USIF_16Bit)
#define USIB_Interleaved		(1<<USIF_Interleaved)
#define USIB_Intel			(1<<USIF_Intel)
#define USIB_Unsigned			(1<<USIF_Unsigned)



/*-------------------------- Eagleplayer-PatternInfo ------------------------*/

struct EP_Patterninfo {
	UWORD	PI_NumPatts;		/* Overall Number of Patterns */
	UWORD	PI_Pattern;		/* Current Pattern (from 0) */
	UWORD	PI_Pattpos;		/* Current Position in Pattern (from 0) */
	UWORD	PI_Songpos;		/* Position in Song (from 0) */
	UWORD	PI_MaxSongPos;		/* Songlengh */
	UWORD	PI_BPM;			/* Beats per Minute */
	UWORD	PI_Speed;		/* Speed */
	UWORD	PI_Pattlength;		/* Length of Actual Pattern in Rows */
	UWORD	PI_Voices;		/* Number of Voices (Patternstripes) */
	ULONG	PI_Modulo;		/* Range from one note to the next */
	APTR	PI_Convert;		/* Converts Note (a0) */
					/* to Period (D0),Samplenumber (D1), */
					/* Commandstring (D2) and Argument (D3) */
};
#define PI_Stripes sizeof(struct EP_Patterninfo)	/* Address of first Patternstripe, */
					                /* followed by the next one etc. of */
					                /* current pattern */

/*
*--------- User-Program-Structure (every player should be use this) ---------*
*--------------- no Taglist used for higher access speed --------------------*
* how to use:                                                                *
* the player sets everytime it accesses the audio-hardware these following   *
* values   the UPS_VoiceXPer is most important, is it 0 means it that no new * 
* note was played, is it different from 0 , all current Userprograms use this* 
* as a new note   the minimum to see something in the analyzers is period in *
* connection with Volume (should be easy), additionally Address and Length   *
* are recommended for the Quadra/Stereo/MonoScope -> don`t forget to set the * 
* flags, see below                                                           *
*----------------------------------------------------------------------------*
* Attention: Please, dont't use this table in the future !!!                 *
*----------------------------------------------------------------------------*
*/

struct UPS_USER {
						/* for Voice 1 */
	APTR	UPS_Voice1Adr;			/* Adr of Samplestart */
	UWORD	UPS_Voice1Len;			/* Size of Sample */
	UWORD	UPS_Voice1Per;			/* Periode of Sample */
	UWORD	UPS_Voice1Vol;			/* Volume of Sample */
	UWORD	UPS_Voice1Note;			/* Note (unused) */
	UWORD	UPS_Voice1SampleNr;		/* SampleNr */
	UWORD	UPS_Voice1SampleType;		/* SampleType */
	UWORD	UPS_Voice1Repeat;		/* Repeat on */

	/*LABEL	UPS_Modulo*/			/* MemSize for one Voice */
						/* for Voice2 */
	APTR	UPS_Voice2Adr;			/* Adr of Samplestart */
	UWORD	UPS_Voice2Len;			/* Length of Sample */
	UWORD	UPS_Voice2Per;			/* Periode of Sample */
	UWORD	UPS_Voice2Vol;			/* Volume of Sample */
	UWORD	UPS_Voice2Note;			/* Note (unused) */
	UWORD	UPS_Voice2SampleNr;		/* SampleNr */
	UWORD	UPS_Voice2SampleType;		/* SampleType */
	UWORD	UPS_Voice2Repeat;		/* Repeat on */

						/* for Voice3 */
	APTR	UPS_Voice3Adr;			/* Adr of Samplestart */
	UWORD	UPS_Voice3Len;			/* Length of Sample */
	UWORD	UPS_Voice3Per;			/* Periode of Sample */
	UWORD	UPS_Voice3Vol;			/* Volume of Sample */
	UWORD	UPS_Voice3Note;			/* Note (unused) */
	UWORD	UPS_Voice3SampleNr;		/* SampleNr */
	UWORD	UPS_Voice3SampleType;		/* SampleType */
	UWORD	UPS_Voice3Repeat;		/* Repeat on */

						/* for Voice4 */
	APTR	UPS_Voice4Adr;			/* Adr of Samplestart */
	UWORD	UPS_Voice4Len;			/* Length of Sample */
	UWORD	UPS_Voice4Per;			/* Periode of Sample */
	UWORD	UPS_Voice4Vol;			/* Volume of Sample */
	UWORD	UPS_Voice4Note;			/* Note (unused) */
	UWORD	UPS_Voice4SampleNr;		/* SampleNr */
	UWORD	UPS_Voice4SampleType;		/* SampleType */
	UWORD	UPS_Voice4Repeat;		/* Repeat on */

	UWORD	UPS_DMACon;			/* Voices on/off 15=all on */
						/* Name irritating, better was */
						/* VoicesStaus, Use like the */
						/* real Dmacon, Bit x set, */
						/* Voice x on */

	UWORD	UPS_Flags;			/* Flags , see below */
	UWORD	UPS_Enabled;			/* Zero = Read enabled */
	UWORD	UPS_Reserved;			/* unused */
}; /* UPS_SizeOF */

/*--------------------------- Flags to UPS_Flags ----------------------------*/
/* these Flags tell, which analyzerfunctions the actual player is able to do */

#define UPSFL_Adr		0		/* submits Sampleaddress */
#define UPSFL_Len		1		/* submits Samplesize */
#define UPSFL_Per		2		/* submits Period (important !) */
#define UPSFL_Vol		3		/* submits Volume */
#define UPSFL_Note		4		/* corrently not used */
#define UPSFL_SNr		5		/* corrently not used */
#define UPSFL_STy		6		/* corrently not used */
#define UPSFL_DMACon		7		/* submits which Voices are on/off */
#define UPSFL_Res		8

#define UPSB_Adr		(1<<UPSFL_Adr)
#define UPSB_Len		(1<<UPSFL_Len)
#define UPSB_Per		(1<<UPSFL_Per)
#define UPSB_Vol		(1<<UPSFL_Vol)
#define UPSB_Note		(1<<UPSFL_Note)
#define UPSB_SNr		(1<<UPSFL_SNr)
#define UPSB_STy		(1<<UPSFL_STy)
#define UPSB_DMACon		(1<<UPSFL_DMACon)
#define UPSB_Res		(1<<UPSFL_Res)

/*-------- Errornumbers for Tags with Error-Returns, for UserPrograms -------*/
/*-------- & for Subprograms (e.g. EPG_NewLoadFile)		     --------*/
#define EPR_UnknownFormat		1
#define EPR_FileNotFound		2
#define EPR_ErrorInFile			3
#define EPR_NotEnoughMem		4
#define EPR_CorruptModule		5
#define EPR_ErrorInstallPlayer		6
#define EPR_EagleRunning		7
#define EPR_CantAllocCia		8
#define EPR_CantAllocAudio		9
#define EPR_CantFindReq			10
#define EPR_NoModuleLoaded		11
#define EPR_ErrorInString		12
#define EPR_CantAllocSerial		13
#define EPR_ErrorDecrunching		14
#define EPR_ErrorExtLoad		15
#define EPR_ErrorAddPlayer		16
#define EPR_SaveError			17
#define EPR_LoadError			18
#define EPR_CantOpenWin			19
#define EPR_PlayerExists		20
#define EPR_WriteError			21
#define EPR_XPKError			22
#define EPR_XPKMasterNotFound		23
#define EPR_PPNotFound			24
#define EPR_LHNotFound			25
#define EPR_ErrorAddUserPrg		26
#define EPR_NoMoreUserPrgs		27
#define EPR_ModuleTooShort		28
#define EPR_CantDeletingPlayer		29
#define EPR_ErrorLoadingInstruments 	30
#define EPR_NoMoreEntries		31
#define EPR_ErrorLoadingDir		32
#define EPR_DirIsEmpty			33
#define EPR_BufferFull			34
#define EPR_UnknownError		35
#define EPR_FunctionNotEnabled		36
#define EPR_Passwordfailed		37
#define EPR_CommandError		38
#define EPR_ErrorInArguments		39
#define EPR_NotImplemented		40
#define EPR_FileIsNotExecutable		41
#define EPR_NotEnoughArguments		42
#define EPR_Functionaborted		43
#define EPR_InvalidNr			44
#define EPR_FileReqCancelled		45
#define EPR_ErrorDeletingFile		46
#define EPR_ErrorLoadingFont		47
#define EPR_NeedHigherKickstart		48
#define EPR_CrunchAborted		49
#define EPR_DirNotChanged		50
#define EPR_NoHelpFile			51
#define EPR_CrmNotFound			52
#define EPR_CantFindReqTools		53
#define EPR_CantFindAsl			54
#define EPR_ExtractorNotFound		55
#define EPR_UserprogramNotFound		56
#define EPR_ErrorInArchive		57
#define EPR_UnknownArchive		58
#define EPR_Archivescanned		59
#define EPR_NoAmplifier			60
#define EPR_AmplifierInitError		61
#define EPR_CantAddGadget		62
#define EPR_FatalError			63
#define EPR_LibraryNotFound		64
#define EPR_NeedhigherCPUFPU		65
#define EPR_NeednewerEP			66
#define EPR_EngineNotLoaded		67
#define EPR_AmigaNeeded			68
#define EPR_DracoNeeded			69

#define EPR_LastError			69

#define EPR_ErrorAddEngine		EPR_ErrorAddUserPrg
#define EPR_NoMoreEngines		EPR_NoMoreUserPrgs
#define EPR_ErrorLoadEngine		EPR_ErrorAddUserPrg
#define EPR_EngineNotFound		EPR_UserprogramNotFound


/*------------------------ Check-Module-Errornumbers ------------------------*/
#define EPRS_LengthTooLong		1
#define EPRS_ReplayerCorrupt		2
#define EPRS_PeriodTableDestroyed 	3
#define EPRS_SamplesTooSmall		4
#define EPRS_ModDatasTooSmall		5
#define EPRS_CorruptSamples		6
#define EPRS_InkorrectSpeed		7
#define EPRS_InkorrectTiming		8
#define EPRS_TooManySubSongs		9
#define EPRS_NotImplmentedCommand	10
#define EPRS_FilesNotFound		11
#define EPRS_AdressError		12
#define EPRS_WrongArgs			13

/* this is from DeliPlayer.h (C) DeliTracker authors - a duplication of the DeliTrackerGlobals structure,
   however not updated for Delitracker 2.x */
struct OldDelitrackerGlobals {
	/* ------ if you use dtg_AslBase, make sure that */
	/* ------ DTP_RequestDTVersion is at least 13 ! */

	struct Library *AslBase;				/* library base, don't CloseLibrary()!! */

	struct Library *DOSBase;				/* library base -"- */
	struct IntuitionBase *IntuitionBase;	/* library base -"- */
	struct GfxBase *GfxBase;				/* library base -"- */
	struct Library *GadToolsBase;			/* librarybase -"- (NULL for Kick 1.3 and below) */
	APTR ReservedLibraryBase;				/* reserved for future use */

	STRPTR	DirArrayPtr;	/* Ptr to the directory of the current module */
	STRPTR	FileArrayPtr;	/* Ptr to the filename of the current module */
	STRPTR	PathArrayPtr;	/* Ptr to PathArray (e.g used in LoadFile()) */

	APTR	ChkData;		/* pointer to the module to be checked */
	ULONG	ChkSize;		/* size of the module */

	UWORD	SndNum;		/* current sound number */
	UWORD	SndVol;		/* volume (ranging from 0 to 64) */
	UWORD	SndLBal;	/* left volume (ranging from 0 to 64) */
	UWORD	SndRBal;	/* right volume (ranging from 0 to 64) */
	UWORD	LED;		/* filter (0 if the LED is off) */
	UWORD	Timer;		/* timer-value for the CIA-Timers */

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
};


/*------------------------------ Global Variables -----------------------------*/
struct EaglePlayerGlobals {

	struct	OldDelitrackerGlobals dt;

	APTR	EPG_FTPRReserved1;
	APTR	EPG_FTPRReserved2;
	APTR	EPG_FTPRReserved3;
	APTR	EPG_SaveMem;		/* Save Mem to Disk */
	APTR	EPG_FileRequest;	/* FileRequester */
	APTR	EPG_TextRequest;	/* Own Textrequester */
	APTR	EPG_LoadExecutable;	/* Load % Decrunch */
	APTR	EPG_NewLoadFile;	/* new DTG_LoadFile with Parameters */
	APTR	EPG_ScrollText;		/* Scroll Text */
	APTR	EPG_LoadPlConfig;	/* Loads a Config from Env:Eagleplayer/.. */
	APTR	EPG_SavePlConfig;	/* Saves a Config to EnvArc:Eagleplayer/.. */
	APTR	EPG_FindTag;		/* Finds a Tag in Tagliste */
	APTR	EPG_FindAuthor;		/* Find Authorname for Soundtracker- */
					/* compatibles */
	APTR	EPG_Hexdez;		/* Convert Dual to Dezimal (Ascii) */
	APTR	EPG_TypeText;		/* Print Text into the Mainwindow */
	APTR	EPG_ModuleChange;	/* Change Playroutine in Module */
	APTR	EPG_ModuleRestore;	/* Restore Playroutine in Module */

	APTR	EPG_FTPRReserved8;	/* dont't use !!! */

	APTR	EPG_XPKBase;		/* librarybase (don`t close) or zero */
	APTR	EPG_LHBase;		/* librarybase (don`t close) or zero */
	APTR	EPG_PPBase;		/* librarybase (don`t close) or zero */
	APTR	EPG_DiskFontBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_ReqToolsBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_ReqBase;		/* librarybase (don`t close) or zero */
	APTR	EPG_XFDMasterBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_WorkBenchBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_RexxSysBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_CommoditiesBase;	/* librarybase (don`t close) or zero */
	APTR	EPG_IconBase;		/* librarybase (don`t close) or zero */
	APTR	EPG_LocaleBase;		/* librarybase (don`t close) or zero */

	APTR	EPG_WinHandle;		/* MainWindow or zero (don`t close) */
	APTR	EPG_TitlePuffer;	/* Puffer for Screentitlename */
	APTR	EPG_SoundSystemname;	/* Ptr to Soundsystemname */
	APTR	EPG_Songname;		/* songname or filename */
	APTR	EPG_Reserved2;		/* zero (don`t use) */
	APTR	EPG_Reserved3;		/* zero (don`t use) */

	APTR	EPG_PubScreen;		/* Pointer to PubScreenname or zero */
	APTR	EPG_CiaBBase;		/* Ciab.Resource-Base or zero */
	APTR	EPG_UPS_Structure;	/* Private UPS_Structure, don't change */
	APTR	EPG_ModuleInfoTagList;	/* Pointer to ModuleInfo-TagList */
	APTR	EPG_Author;

	LONG	EPG_Identifier;		/* Longword="EPGL" --> Eagleplayer used */
	LONG	EPG_EagleVersion;
	WORD	EPG_Speed;

	LONG	EPG_ARGN;			/* Anz. of Parameters */

	LONG	EPG_ARG1;		/* \ */
	LONG	EPG_ARG2;		/*  \ */
	LONG	EPG_ARG3;		/*   \	Parameter-Buffer for SubProggys */
	LONG	EPG_ARG4;		/*    \_\ You must use this. You must set */
	LONG	EPG_ARG5;		/*    / /	ArgN (max. used Parameters) */
	LONG	EPG_ARG6;		/*   /	--> New SubProggys can use */
	LONG	EPG_ARG7;		/*  /	more Parameters !!!! */
	LONG	EPG_ARG8;		/* / don`t use in interrupts !!!! */

	UWORD	EPG_Voices;

	UWORD	EPG_Voice1Vol;		/* 0-64 */
	UWORD	EPG_Voice2Vol;
	UWORD	EPG_Voice3Vol;
	UWORD	EPG_Voice4Vol;

	APTR	EPG_VoiceTable;		/* Voicetable (only for Amplifier) 0-255 */
	UWORD	EPG_VoiceTableEntries;
	UWORD	EPG_Unused1;

	ULONG	EPG_SomePrefs;		/* Bits you find above the Globals */
	ULONG	EPG_Timeout;		/* Timeout in Secs ; 0 = No Timeout */

	WORD	EPG_FirstSnd;	/* first Subsong Nr. (-1 for unknown) */
	WORD	EPG_SubSongs;	/* Subsong Range (-1 for unknown) */
	ULONG	EPG_MODNr;	/* Actual Module in List (Nr) ; 0 = No Mod. */
	ULONG	EPG_MODS;	/* Number of Modules in List (Nr); 0 = No List */

	APTR	EPG_PlayerTagList;	/* Pointer to actual PlayerTaglist */
	APTR	EPG_TextFont;		/* Pointer to actual Font-Structure */

	UWORD	EPG_Volume;		/* Volumerange 0-255 */
	UWORD	EPG_Balance;		/* Balance */
	UWORD	EPG_LeftBalance;	/* Balancerange 0-255 */
	UWORD	EPG_RightBalance;	/* Balancerange 0-255 */
	UWORD	EPG_UnUsed6;
	UWORD	EPG_UnUsed7;
	UWORD	EPG_UnUsed8;
	UWORD	EPG_UnUsed9;

	WORD	EPG_DefTimer;
	WORD	EPG_CurrentPosition;
	WORD	EPG_WORDReserved3;
	WORD	EPG_WORDReserved4;
	WORD	EPG_WORDReserved5;
	WORD	EPG_WORDReserved6;
	WORD	EPG_WORDReserved7;
	WORD	EPG_WORDReserved8;

	LONG	EPG_Dirs;
	LONG	EPG_LoadedFiles;	/* loaded files by DTG/EPG_LoadFile */
	APTR	EPG_AppPort;
	APTR	EPG_SampleInfoStructure; /* call first EPNr_Sampleinit !!!! */

	LONG	EPG_MinTimeOut;
	LONG	EPG_CurrentTime;	/* Current Playtime */
	LONG	EPG_Duration;		/* Duration */
	LONG	EPG_FirstUserStruct;
	LONG	EPG_FirstFileStruct;
	LONG	EPG_Entries;
	LONG	EPG_Modulesize;
	APTR	EPG_Playerlist;
	APTR	EPG_Enginelist;
	APTR	EPG_Moduleslist;

	APTR	EPG_AmplifierList;	/* Wie Enginelist */
	APTR	EPG_ActiveAmplifier;	/* EUS_StartAdr */
	APTR	EPG_AudioStruct;	/* aktuelle AudioStruct (EP privat) */
	APTR	EPG_AmplifierTagList;
	APTR	EPG_ConfigDirArrayPtr;
	APTR	EPG_PlayerDirArrayPtr;	/* Future use */
	APTR	EPG_EngineDirArrayPtr;
	APTR	EPG_FirstPlayerStruct;
	APTR	EPG_ChkSegment;
	APTR	EPG_EagleplayerDirArrayPtr;
}; /* LABEL	EPG_SizeOf */		/* to be extended */


/*------------- Eagleplayer Globals Preferences-Flags ------------*/
#define EGPRF_Unused			0			/* FadeIn V1.0-V1.54ß */
#define EGPRF_Fadeout			1
#define EGPRF_Songend			2
#define EGPRF_Loadnext			3
#define EGPRF_Randommodule		4
#define EGPRF_Mastervolume		5
#define EGPRF_NowPlay			6
#define EGPRF_Surfacequit		7
#define EGPRF_LoadPrev			8
#define EGPRF_PausePlay			9
#define EGPRF_LoadFast			10
#define EGPRF_CalcDuration		11
#define EGPRF_AllocChannels		12
#define EGPRF_SoftInt			13
#define EGPRF_Iconify			14
#define EGPRF_Help			15
#define EGPRF_Fadein			16

#define EGPRB_Unused			(1<<EGPRF_Unused)		/* FadeIn V1.0-V1.54ß */
#define EGPRB_Fadeout			(1<<EGPRF_Fadeout)
#define EGPRB_Songend			(1<<EGPRF_Songend)
#define EGPRB_Loadnext			(1<<EGPRF_Loadnext)
#define EGPRB_Randommodule		(1<<EGPRF_Randommodule)
#define EGPRB_Mastervolume		(1<<EGPRF_Mastervolume)
#define EGPRB_NowPlay			(1<<EGPRF_NowPlay)
#define EGPRB_Surfacequit		(1<<EGPRF_Surfacit)
#define EGPRB_LoadPrev			(1<<EGPRF_LoadPrev)
#define EGPRB_PausePlay			(1<<EGPRF_PausePlay)
#define EGPRB_LoadFast			(1<<EGPRF_LoadFast)
#define EGPRB_CalcDuration		(1<<EGPRF_CalcDuration)
#define EGPRB_AllocChannels		(1<<EGPRF_AllocChannels)
#define EGPRB_SoftInt			(1<<EGPRF_SoftInt)
#define EGPRB_Iconify			(1<<EGPRF_Iconify)
#define EGPRB_Help			(1<<EGPRF_Help)
#define EGPRB_Fadein			(1<<EGPRF_Fadein)


/*
*----------------------- Extended GlobalJumps -----------------------------*
*-- This structure is the negative buffer relative to a5 (Basisregister) --*
*--------------------------------------------------------------------------*
*/
/* outside in .fd file, mapping to proto/eagleplayer.h, inline/ clib/ etc. */
#if 0
 STRUCTURE NEWEP_GLOBALS,-6
		STRUCT	ENPP_AllocSampleStruct,-6
		STRUCT	ENPP_NewLoadFile2,-6	/* You must Freemem it !!! */
		STRUCT	ENPP_MakeDirCorrect,-6
		STRUCT	ENPP_TestAufHide,-6
		STRUCT	ENPP_ClearCache,-6
		STRUCT	ENPP_CopyMemQuick,-6
		STRUCT	ENPP_GetPassword,-6
		STRUCT	ENPP_StringCopy2,-6
		STRUCT	ENPP_ScreenToFront,-6
		STRUCT	ENPP_WindowToFront,-6

	*--------- old DeliTracker-Globals ---------*
		STRUCT	ENPP_GetListData,-6
		STRUCT	ENPP_LoadFile,-6
		STRUCT	ENPP_CopyDir,-6		/* \  new! not 100% kompatibel */
		STRUCT	ENPP_CopyFile,-6	/*  > to dtg_copyxxx */
		STRUCT	ENPP_CopyString,-6	/* / */
		STRUCT	ENPP_AllocAudio,-6
		STRUCT	ENPP_FreeAudio,-6
		STRUCT	ENPP_StartInterrupt,-6
		STRUCT	ENPP_StopInterrupt,-6
		STRUCT	ENPP_SongEnd,-6
		STRUCT	ENPP_CutSuffix,-6
		STRUCT	ENPP_SetTimer,-6
		STRUCT	ENPP_WaitAudioDMA,-6

	*--------- old EaglePlayer-Globals --------*
		STRUCT	ENPP_SaveMem,-6
		STRUCT	ENPP_FileReq,-6
		STRUCT	ENPP_TextRequest,-6
		STRUCT	ENPP_LoadExecutable,-6
		STRUCT	ENPP_NewLoadFile,-6
		STRUCT	ENPP_ScrollText,-6
		STRUCT	ENPP_LoadPlConfig,-6
		STRUCT	ENPP_SavePlConfig,-6
		STRUCT	ENPP_FindTag,-6
		STRUCT	ENPP_FindAuthor,-6

		STRUCT	ENPP_Hexdez,-6
		STRUCT	ENPP_TypeText,-6
		STRUCT	ENPP_ModuleChange,-6
		STRUCT	ENPP_ModuleRestore,-6

		STRUCT	ENPP_StringCopy,-6
		STRUCT	ENPP_CalcStringSize,-6
		STRUCT	ENPP_StringCMP,-6


		STRUCT	ENPP_DMAMask,-6
		STRUCT	ENPP_PokeAdr,-6
		STRUCT	ENPP_PokeLen,-6
		STRUCT	ENPP_PokePer,-6
		STRUCT	ENPP_PokeVol,-6
		STRUCT	ENPP_PokeCommand,-6
		STRUCT	ENPP_Amplifier,-6

		STRUCT	ENPP_TestAbortGadget,-6
		STRUCT	ENPP_GetEPNrfromMessage,-6	/* In:a1=Message */

		STRUCT	ENPP_InitDisplay,-6		/* a0=Text a1=Args */
		STRUCT	ENPP_FillDisplay,-6		/* -++- d0=per thousand */
		STRUCT	ENPP_RemoveDisplay,-6

		STRUCT	ENPP_GetLocaleString,-6

		STRUCT	ENPP_SetWaitPointer,-6
		STRUCT	ENPP_ClearWaitPointer,-6

		STRUCT	ENPP_OpenCatalog,-6
		STRUCT	ENPP_CloseCatalog,-6

		STRUCT	ENPP_AllocAmigaAudio,-6		*allocate original Amigaaudio (for Amplifiers)
		STRUCT	ENPP_FreeAmigaAudio,-6

		STRUCT	ENPP_RawToFormat,-6
		STRUCT	ENPP_FindAmplifier,-6
		STRUCT	ENPP_UserCallup5,-6
		STRUCT	ENPP_GetLoadListData,-6	/* (for ExtLoad) */
		STRUCT	ENPP_SetListData,-6	/* only for rippers !!! */
		STRUCT	ENPP_GetHardwareType,-6
		LABEL	ENPP_SizeOf		/* to be extended !!! */
#endif



/*------------------------------ Hardwaretypes --------------------------------*/
#define EPHT_Amiga	0
#define EPHT_Draco	1


/*----------------------------- Player header --------------------------------*/
/* moveq #-1,d0		; prevent crashes if someone
   rts			; tries to call the player from shell
   dc.b    'EPPLAYER'	; identifier as "Eagleplayer Player"
   dc.l \1		; pointer to Tagitem array
*/
#define EPPHEADER(_A_) \
 const ULONG EPPHEADER[4] = { 0x70ff4e75,'EPPL','AYER',(ULONG)(_A_) };

#endif /* _INC_EAGLEPLAYER_H */

