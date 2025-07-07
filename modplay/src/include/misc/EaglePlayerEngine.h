/*
**
**	$Filename: misc/EaglePlayerEngine.h
**	$Release: 2.01 $
**	$Revision: 16$
**	$Date: 03/16/98$
**
** Definitions and Macros for creating EaglePlayer Engines & Players
**
**	(C) Copyright 1994-2018 by DEFECT
**		All Rights Reserved
**
**
*/
#ifndef _INC_EAGLEPLAYERENGINE_H
#define _INC_EAGLEPLAYERENGINE_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef _INC_EAGLEPLAYER_H
#include <misc/EaglePlayer.h>
#endif

/*---------------------------- macros --------------------------------------*/
#if 0
RS_RESET	MACRO
RSOFFSET	SET	0
		ENDM

RS_BYTE		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+\2
		ENDM

RS_WORD		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+2*\2
		ENDM

RS_LONG		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+4*\2
		ENDM
#endif

/*---------------------------- PlayerInfo Tags -----------------------------*/
#define EPPI_TagBase	TAG_USER+20553		/* TAG_USER+"PI" */

#define EPPI_Creator		(EPPI_TagBase+0)	/*Ptr to Creator*/
#define EPPI_CreatorYearPrg	(EPPI_TagBase+1)	/*Date for Program*/
#define EPPI_CreatorYearPlayer	(EPPI_TagBase+2)	/*Date for Eagleplayer*/
#define EPPI_Adapted		(EPPI_TagBase+3)	/*Ptr to Adaptor*/
#define EPPI_About		(EPPI_TagBase+4)	/*TI_Data points on a string*/
       				/*telling something about this*/
       				/*Soundsystem*/
#define EPPI_MaxPattern		(EPPI_TagBase+5)	/*Max Pattern (SoundTr. 64)*/
#define EPPI_MaxPatternLength	(EPPI_TagBase+6)	/*Max PatternLen (SoundTr. 64)*/
#define EPPI_MaxLength		(EPPI_TagBase+7)	/*Max Length (SoundTr. 128)*/
#define EPPI_MaxTracks		(EPPI_TagBase+8)	/*Max Tracks (SonicA.)*/
#define EPPI_MaxVoices		(EPPI_TagBase+9)	/*Max Voices*/
#define EPPI_MaxSubSongs	(EPPI_TagBase+10)	/*Max SubSongs*/
#define EPPI_MaxSteps		(EPPI_TagBase+11)	/*Max Steps*/
#define EPPI_MaxSamples		(EPPI_TagBase+12)	/*Max Samples*/
#define EPPI_MaxSynthSamples	(EPPI_TagBase+13)	/*Max SynthSamples*/
#define EPPI_SpecialInfo	(EPPI_TagBase+14)	/*Pointer to InfoText*/
#define EPPI_UnPackedSystem	(EPPI_TagBase+15)	/*MIUS (see below) or APTR*/
       						/*to Soundsysname*/
#define EPPI_Prefix		(EPPI_TagBase+16)	/*Save-Prefix (e.g. "MOD." or*/
       						/*"Mdat.")*/
#define EPPI_Prefix2		(EPPI_TagBase+17)	/*for SampleData ("Smpl.")*/
#define EPPI_NeededProzessor	(EPPI_TagBase+18)
#define EPPI_NeededKickStart	(EPPI_TagBase+19)
#define EPPI_ReplayerSizeChip	(EPPI_TagBase+20)
#define EPPI_ReplayerSizeFast	(EPPI_TagBase+21)
#define EPPI_Flags		(EPPI_TagBase+22)
#define EPPI_AssessmentFileFormat	(EPPI_TagBase+23)	/*Note from 0 to 6*/
#define EPPI_AssessmentMultitaskingSure	(EPPI_TagBase+24)
#define EPPI_AssessmentBugs	(EPPI_TagBase+25)
#define EPPI_AssessmentTempo	(EPPI_TagBase+26)
#define EPPI_AssessmentReplayer	(EPPI_TagBase+27)


/**--------------------------- PlayerInfo-Flags ----------------------------*/
#define EPPIF_SelfModify	 	1
#define EPPIF_ReplayerInFront	 	2
#define EPPIF_FilesSpitted	 	3
#define EPPIF_CiaTiming		 	4
#define EPPIF_RealPattern	 	5
#define EPPIF_MidiSupport	 	6
#define EPPIF_AudioInterrupt	 	7
#define EPPIF_SoftInt		 	8


/*
**----------------------- Engine -Start-Structure ----------------------**
**	This Structure must be initialized in the executable file	**
**	EUS_Next,EUS_UserNr don`t use, important !!!			**
**	EUS_Flags,EUS_WinX,EUS_WinY,EUS_Special can be initialized	**
**	(will be changed if prefs are loaded :)-=-=	)		**
*/

/*struct EUS_Identifier;*/

struct Engine_START {
	ULONG	EUS_Jmp;		/*bra.s (bra.b) to the real start of the*/
					/*userprogram (usually direct after this*/
					/*struct	(MUST be a BRA.W !!!!!)*/
	UBYTE	EUS_Identifier[8];	/*Identifier "EPEngine"*/
					/*Eagleplayer can use this (Ampifiers)*/
	APTR	EUS_Next;		/*Don`t Use !!! only for Eagleplayer*/
	UWORD	EUS_UserNr;		/*don`t change, only for Eagleplayer*/

	APTR	EUS_EPBase;		/*Pointer to the Eagleplayerglobals*/
	APTR	EUS_FreeTable;
	APTR	EUS_TaskAdr;		/*if zero=kein Task*/
	APTR	EUS_Unused1;		/*Future use (don't change)*/
	APTR	EUS_Unused2;		/*Future use (don't change)*/
	APTR	EUS_SpecialJumpTab;	/*private Jumptab (e.g Amplifiers)*/
	APTR	EUS_TagList;
	UWORD	EUS_Ticks;		/*Ticks for Interrupt (0=no Interrupt)*/
	UWORD	EUS_TickCounter;
	ULONG	EUS_TickFlags;
	ULONG	EUS_MsgFlags;

	APTR	EUS_PName;		/*Processname of this Engine*/
					/*e.g. 'Analyzer.Task'*/
	APTR	EUS_Creator;		/*Creatorname e.g. "BUGGS" or "Eagleeye"*/
	APTR	EUS_AboutEngine;	/*tells something about this Engine.*/
	UWORD	EUS_KickStart;		/*min Kickversion 0=All , 37 = OS 2.0*/
	ULONG	EUS_EPVersion;		/*min. EagleplayerVersion*/
	UWORD	EUS_Version;		/*Engine. Version	eg. 37 \not very*/
	UWORD	EUS_Revision;		/*Engine. Revision eg.175 /important*/
	APTR	EUS_UserName;		/*Name for PullDownMenu (max.14 Chars)*/
	UWORD	EUS_WinX;		/*User-Window X-Position*/
	UWORD	EUS_WinY;		/*User-Window Y-Position*/
	UWORD	EUS_Flags;		/*Flags (Iconify/ZipWindow,Window Open),*/
					/*see below*/
	UWORD	EUS_AMPriority;		/* was: ULONG EUS_Special */
	UWORD	EUS_AMFlags;
	ULONG	EUS_Special2;		/*SpecialInfos (e.g. Windowsize)*/
	ULONG	EUS_Special3;		/*more SpecialInfos*/

	ULONG	EUS_CreatorDate;	/*Creatordate (e.g 14.b 6. 1993.w)*/
	UBYTE	EUS_Priority;		/*TaskPriority of Engine*/
	UBYTE	EUS_Type;		/*Userprogramtype (0=unknown)*/

	UWORD	EUS_Reserved2;		/*set zero, don`t change*/
	ULONG	EUS_AMIDNr;		/*ID Kennung des Amplifiers,darf nur einmal vorkommen*/
	APTR	EUS_AMTiming;           /* was EUS_Reserved4, Amplifier supplied */
                                        /* playback timing (i.e. no separate timer/interrupt needed) */
	ULONG	EUS_AMUPSStruct;	/*set zero, don`t change*/
	ULONG	EUS_Reserved6;		/*set zero, don`t change*/
}; /* EUS_SizeOF */

struct Engine_Header {
 struct Engine_START e; /* EUS structure    */
 UWORD  jmp;            /* 0x4EF9 goes here */
 APTR   main;           /* entry point for main() */
};
#define ENGINE_HEADER_JMP 0x4EF9
#define ENGINE_HEADER_BRA (0x5FFFFFFE + sizeof(struct Engine_START))

#define EUS_EngineNr	 	EUS_UserNr
#define EUS_EngineName	 	EUS_UserName
#define EUS_AboutUPrg	 	EUS_AboutEngine
//#define EUS_AMPriority	 	EUS_Special	/*Priorität des Amplifiers*/
					/*(zum Einsortieren)*/
//#define EUS_AMFlags	 	EUS_Special+2	/*Flags des Amplifiers (Enable/Disable)*/

/*--------------------------------- EUS-Flags --------------------------------*/
#define EUSF_Openwin	 	0	/*Window to be opened when started ?*/
				/*Bit set = yes*/
#define EUSF_Zipwin	 	1	/*Zipwindow/Iconify when started ?*/
				/*Bit set = yes*/
#define EUSF_Disable	 	2	/*Amplifier disabled ?*/
				/*Bit set = yes*/
#define EUSF_Show	 	3	/*Show (e.g. Extractor)*/

#define EUSF_Activate	 	EUSF_Openwin

#define EUSB_Openwin	 	(1<<EUSF_Openwin)
#define EUSB_Zipwin	 	(1<<EUSF_Zipwin)
#define EUSB_Disable	 	(1<<EUSF_Disable)
#define EUSB_Show	 	(1<<EUSF_Show)
#define EUSB_Activate	 	(1<<EUSF_Activate)

/*--------------------------------- EUS-NPFlags ------------------------------*/

#define ENPF_NoWindow	 	0		/*no Window to show/hide ?*/
#define ENPB_NoWindow	 	(1<<ENPF_NoWindow)


/*----------------------------- EUS-InterruptFlags ----------------------------*/
#define EUIF_Always	 	1
#define EUIF_OnlyPlay	 	2
#define EUIF_OnlyActive	 	3
#define EUIF_TickCounter	4

#define EUIB_Always	 	(1<<EUIF_Always)
#define EUIB_OnlyPlay	 	(1<<EUIF_OnlyPlay)
#define EUIB_OnlyActive	 	(1<<EUIF_OnlyActive)
#define EUIB_TickCounter	(1<<EUIF_TickCounter)

/*------------------------ Amplifiers timing provisions -----------------------*/
struct Amplifier_Timing {
        ULONG   APT_Version;            /* extension, keep 0 for now */
        APTR    APT_Init;
        APTR    APT_SetTime;
        APTR    APT_End;
        APTR    APT_Call;               /* set by EP: struct Interrupt */
};

/*------------------------ Amplifiers direct Jumps ---------------------------*/

struct Amplifier_Jumps {
	APTR	AMJ_CheckFeatures;	/*Test, ob der Amplifier Spezialfeatures wie z.B. 16 Bit Samples*/
					/*unterstützt, optional*/

	APTR	AMJ_Init;		/*Übergabe der Amplifiertagliste*/
	APTR	AMJ_StartInt;		/*Alloc Audio & Start Int*/
	APTR	AMJ_StopInt;		/*Free Audio & Stop Int*/
	APTR	AMJ_End;		/*mem freigeben, etc.*/
	APTR	AMJ_Amplifier;		/*Übergabe einer neu gefüllten AS_-Struktur an den Amplifier durch das Replay*/

	APTR	AMJ_PokeAdr;		/*nur für Amplifier initialisieren,*/
	APTR	AMJ_PokeLen;		/*die auf die Hardware poken, wie*/
	APTR	AMJ_PokePer;		/*z.B. der Chipram Player, sonst*/
	APTR	AMJ_PokeVol;		/*auf 0 setzen, diese Jumps werden mit*/
	APTR	AMJ_DMAMask;		/*den selben Parametern aufgerufen, wie die ENPP_Pokes*/

	APTR	AMJ_Command;		/*spezielle Kommandos ,z.B Filter*/
					/*Aufruf: D0 - Command*/
					/*        D1 - Argument*/
};
#define AMCMD_Filter	1	/*D1: 0=off;1=on;-1=toggle*/

/*------------------------------ EUS-Enginetypes -----------------------------*/
#define EUTY_Unknown		 	0
#define EUTY_Manager		 	1
#define EUTY_SampleInfo		 	2
#define EUTY_EngineInfo		 	3
#define EUTY_PlayerInfo		 	4
#define EUTY_Decruncher		 	5
#define EUTY_Ripper		 	6
#define EUTY_MainWindow		 	7
#define EUTY_ScreenManager	 	8
#define EUTY_DirViewer		 	9
#define EUTY_Analyzer		 	10
#define EUTY_Amplifier		 	11
#define EUTY_PatternScroll	 	12
#define EUTY_MessageWindow	 	13
#define EUTY_ListAdministration	 	14
#define EUTY_AmplifierManager	 	15
#define EUTY_PlayerLoader	 	16
#define EUTY_Extractor		 	17
#define EUTY_Scope		 	18
#define EUTY_ModuleInfo		 	19
#define EUTY_Converter		 	20
#define EUTY_FormatLoader	 	21
#define EUTY_SampleSaver	 	22


/*------------------------------- Engine MsgFlags ----------------------------*/
/*------- If you want to get special Messages you must set this flags --------*/
#define USMF_Zipwin		 	0
#define USMF_NewEngineLoaded	 	1
#define USMF_ChangeConfig	 	2	/* for WinPos,ZipPos */
#define USMF_Command		 	3
#define USMF_NewModule		 	4
#define USMF_NewSong		 	5
#define USMF_NewPreference	 	6
#define USMF_ChangeInterrupt	 	7
#define USMF_PlaySample		 	8
#define USMF_NewScrollText	 	9
#define USMF_NewDirectory	 	10
#define USMF_NewPlayer		 	11
#define USMF_KillModule		 	12
#define USMF_Surface		 	13
#define USMF_ActiveSurface	 	14
#define USMF_AmplifiersChanged	 	15	/*Änderung in Amplifierlist*/
#define USMF_RemPlayerList	 	17
#define USMF_RemEnginesList	 	18
#define USMF_RemModulesList	 	19
#define USMF_SaveConfig		 	20	/*for Amplifierconfig*/
#define USMF_Configuration	 	21	/*Configwindow (e.g. Extractor)*/
#define USMF_Show		 	22
#define USMF_LoadConfig		 	23
#define USMF_WaitPointer	 	24

#define USMB_Zipwin		 	(1<<USMF_Zipwin)
#define USMB_NewEngineLoaded	 	(1<<USMF_NewEngineLoaded)
#define USMB_ChangeConfig	 	(1<<USMF_ChangeConfig)
#define USMB_Command		 	(1<<USMF_Command)
#define USMB_NewModule		 	(1<<USMF_NewModule)
#define USMB_NewSong		 	(1<<USMF_NewSong)
#define USMB_NewPreference	 	(1<<USMF_NewPreference)
#define USMB_ChangeInterrupt	 	(1<<USMF_ChangeInterrupt)
#define USMB_PlaySample		 	(1<<USMF_PlaySample)
#define USMB_NewScrollText	 	(1<<USMF_NewScrollText)
#define USMB_NewDirectory	 	(1<<USMF_NewDirectory)
#define USMB_NewPlayer		 	(1<<USMF_NewPlayer)
#define USMB_KillModule		 	(1<<USMF_KillModule)
#define USMB_Surface		 	(1<<USMF_Surface)
#define USMB_ActiveSurface	 	(1<<USMF_ActiveSurface)
#define USMB_AmplifersChanged	 	(1<<USMF_AmplifiersChanged)
#define USMB_RemPlayerList	 	(1<<USMF_RemPlayerList)
#define USMB_RemEnginesList	 	(1<<USMF_RemEnginesList)
#define USMB_RemModulesList	 	(1<<USMF_RemModulesList)
#define USMB_SaveConfig		 	(1<<USMF_SaveConfig)
#define USMB_Configuration	 	(1<<USMF_Configuration)
#define USMB_Show		 	(1<<USMF_Show)
#define USMB_LoadConfig		 	(1<<USMF_LoadConfig)
#define USMB_WaitPointer	 	(1<<USMF_WaitPointer)

/*----------------------------- EP-Userprogram-Tags ---------------------------*/
#define EUT_TagBase	TAG_USER+21843	/*TAG_USER+"US"		;EagleplayerUserTags*/

#define EUT_EngineFlags	(EUT_TagBase+0)	 /* Userflags */

#define EUT_InitEngine	(EUT_TagBase+1)	 /* Wird aufgerufen, wenn ein Engine ge- */
				 /* laden wird und kein Task ist. */
				 /* Die richtige Konfiguration ist bereits */
				 /* in der EUS-Strukture enthalten ! */
				 /* Bei einem Error wird das Engine */
				 /* entfernt !!!!!!!!!!!!!!!!!! */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_ExitEngine	(EUT_TagBase+2)	 /* Das Engine wird beendet */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_Activate	(EUT_TagBase+3)	 /* Das Engine startet seine Arbeit */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_DeActivate	(EUT_TagBase+4)	 /* Das Engine beendet seine Arbeit */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_Show	(EUT_TagBase+5)	 /* Open Window */
				 /* output:d0=Null oder Error */
#define EUT_Hide	(EUT_TagBase+6)	 /* Open Window */
#define EUT_Config	(EUT_TagBase+7)	 /* Das Engine startet eine Configuration */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_GetConfig	(EUT_TagBase+8)	 /* Das UserProgramm soll seine aktuelle */
				 /* Configuration in den Merkzellen der */
				 /* EUS-Strukture schreiben */
				 /* INPUT : - */
				 /* OUTPUT: d0=Error oder NULL */
				 /* 	EUS-Strukture gefüllt */
#define EUT_NewConfig	(EUT_TagBase+9)	 /* Eine neue Config für dieses Engine */
				 /* wurde festgelegt (->EUS-Strukture) */
				 /* INPUT : EUS-Strukture gefüllt */
				 /* OUTPUT: d0=Error oder NULL */

#define EUT_OpenAWindow	(EUT_TagBase+10)	 /* Öffnet ein Window vom angegebenen Type */
				 /* z.B. Surface, Playerwindow,TextrEQUest */
				 /* INPUT : d0=Type des Windows */
				 /* 	d1=Pos X oder -1 für Unknown */
				 /* 	d2=Pos Y oder -1 für Unknown */
				 /* 	a0=Title des Windows */
				 /* 	d3-a3=SpecialInfo */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_CloseAWindow	(EUT_TagBase+11) /* Öffnet ein Window vom angegebenen Type */
				 /* z.B. Surface, Playerwindow,TextrEQUest */
				 /* INPUT : d0=Type des Windows */
				 /* 	d1=Pos X oder -1 für Unknown */
				 /* 	d2=Pos Y oder -1 für Unknown */
				 /* 	d3-a3=SpecialInfo */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_GetWindowHandle	(EUT_TagBase+12) /* es handelt sich um ein normales Int. */
				 /* Window. AppWindow,WaitPointer,Menues */
				 /* werden automatisch bearbeitet. */
				 /* Input:	d0=Windowtype */
				 /* Output: d0=WinHandle oder Error */
#define EUT_SetWaitPointer	(EUT_TagBase+13) /* Input:	- */
				 /* Output: d0=Error oder NULL */
#define EUT_ClearWaitPointer	(EUT_TagBase+14) /* Input:	- */
				 /* Output: d0=Error oder NULL */
#define EUT_ChangeAWindowPosition	(EUT_TagBase+15) /*  Das Window wird in seiner Position */
				 /*  im Screen verschoben */
				 /* INPUT : d0=Type des Windows */
				 /* 	d1=abs. X-Position */
				 /* 	d2=abs. Y-Position */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_Iconify	(EUT_TagBase+16)	 /* Die Engine soll in den Iconify-Mode */
				 /* usefull für EP-Surfaces */
				 /* INPUT : d0=interne EP-IconifyMode */
				 /* OUTPUT: d0=NULL oder Error */
				 /* 	d1=NULL (EP-Mode) */
				 /* 		<> NULL (SurfaceMode) */
#define EUT_UnIconify	(EUT_TagBase+17)	 /* Die Engine soll zurück in Normal- */
				 /* Mode */
				 /* usefull für EP-Surfaces */
				 /* INPUT : d0=IconifyMode */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_ZipAWin	(EUT_TagBase+18)	 /* Das Window soll in den ZipMode */
				 /* INPUT : d0=Type des Windows */
				 /* 	d1=Pos X oder -1 für Unknown */
				 /* 	d2=Pos Y oder -1 für Unknown */
				 /* 	d3-a3=SpecialInfo */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_UnZipAWin	(EUT_TagBase+19)	 /* Das Window soll in den UnZipMode */
				 /* INPUT : d0=Type des Windows */
				 /* 	d1=Pos X oder -1 für Unknown */
				 /* 	d2=Pos Y oder -1 für Unknown */
				 /* 	d3-a3=SpecialInfo */
				 /* OUTPUT: d0=NULL oder Error */

#define EUT_Changelocale	(EUT_TagBase+20) /* Die Localisation wird geändert */
				 /* INPUT : ? */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_UpdateVER	(EUT_TagBase+21)	 /* Updated alle Versionsnummern des EP */
				 /* in den Texten des Userprogramms */
				 /* INPUT : d0=0 -> nicht registriert */
				 /* 		1 -> registriert */
				 /* 	a0=Zeiger auf Versionstring */
				 /* 		4 Zeichen ("1.60") */
				 /* OUTPUT: d0=Error oder NULL */

#define EUT_DisableAll	(EUT_TagBase+22)	 /* Alle Gadgets und Menustrips werden */
				 /* disabled. Vorbereitung für ChangeGui ! */
				 /* INPUT : - */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_EnableAll	(EUT_TagBase+23)	 /* Alle Gadgets und Menustrips werden */
				 /* enabled. Vorbereitung für ChangeGui ! */
				 /* INPUT : - */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_SignalReceived	(EUT_TagBase+24) /* INPUT:	d0=SignalSet */
				 /* Output: d0=ActionNr oder NULL */
				 /* Es wurden Signale empfangen. Die zur */
				 /* angegebenen WaitMask gehören */
				 /* Hinweis, EUT_GetMSG wird danach vom */
				 /* EP aufgerufen */
#define EUT_GetWaitMask	(EUT_TagBase+25)	 /* Gibt dem EP die Signalbits auf die */
				 /* zusätzlich zu den EP-internen gewartet */
				 /* werden soll */
				 /* INPUT : - */
				 /* OUTPUT: d0=SignalSet oder -1 */
#define EUT_GetInterruptMask	(EUT_TagBase+26) /* Diese SignalMaske wird beim Ablauf des */
				 /* Interrupts (EUS_Count) gesendet. */
				 /* Input: -- */
				 /* Output: d0=SignalNr */
#define EUT_GetMsg	(EUT_TagBase+27)	 /* Testen im Engine, ob ein Gadget ge- */
				 /* drückt wurde (für Surface) */
				 /* INPUT : d0=Signal oder -1 */
				 /* OUTPUT: d0=EPNr oder NULL */
				 /* 	 a0=Hook */
				 /* 	 d1=? */
				 /* 	 d2/a2 = Arg1 */
				 /* 	 d3/a3 = Arg2 */
#define EUT_ClearMenuStrip	(EUT_TagBase+28) /* Löscht die entsprechenden MenuStrips */
				 /* in allen Engines */
				 /* INPUT : a1=MenuStrip (Test auf Zero) */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_SetMenuStrip	(EUT_TagBase+29) /* Setzt die MenuStrips in allen */
				 /* offenen Engines */
				 /* INPUT : a1=MenuStrip (Test auf Zero) */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_Unused5	(EUT_TagBase+30)
#define EUT_Unused4	(EUT_TagBase+31)
#define EUT_Command	(EUT_TagBase+32)	 /* Ein Commando aus der UCM_Tabelle wird */
				 /* oder soll ausgeführt werden */
				 /* INPUT : d0=Nr der Aktion */
				 /* 	d1-d7=Parameter */
				 /* 	a0-a3=Parameter */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_NewModule	(EUT_TagBase+33)	 /* Es wurde ein neues Module geladen */
				 /* INPUT : d0=Size des Modules */
				 /* 	a0=Name des Modules */
				 /* 	a1=SoundSystem */
				 /* 	a2=PlayerTagListe */
				 /* 	a3=ModListeStructure */
				 /* OUTPUT: - */
#define EUT_NewSong	(EUT_TagBase+34)	 /* Der angegebene Song wird abgespielt */
				 /* INPUT : d0=Nr des Songs */
				 /* OUTPUT: - */
#define EUT_NewPreference	(EUT_TagBase+35) /* In der Eagleplayer-Preference wurde */
				 /* eine Einstellung geändert. Die Para */
				 /* meter werden je nach Aktion genutzt */
				 /* INPUT : d0=Nr der Aktion */
				 /* 	d1-d7=Parameter */
				 /* 	a0-a3=Parameter */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_StartInt	(EUT_TagBase+36)	 /* Der EP-Interrupt wurde gestartet */
				 /* INPUT : - */
				 /* OUTPUT: - */
#define EUT_StopInt	(EUT_TagBase+37)	 /* Der EP-Interrupt wurde gestoppt */
				 /* INPUT : - */
				 /* OUTPUT: - */
#define EUT_PlaySample	(EUT_TagBase+38)	 /* Das angegebene Sample wird abgespielt */
				 /* Audiokanäle sind allokiert ! */
				 /* INPUT : d0=Nr des Samples */
				 /* 	d1=PlayMode (Loop,NoLoop) */
				 /* 	d2=Offset vom Start */
				 /* 	d3=Offset vom End */
				 /* 	a0=SampleInfostrukture */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_ClearTextWindow	(EUT_TagBase+39) /* Das TextWindow wird gelöscht. */
#define EUT_NewScrollText	(EUT_TagBase+40) /* Der angegebene ScrollText soll ge- */
				 /* scrollt werden. Er kann unendlich lang */
				 /* sein */
				 /* INPUT : a0=ScrollText */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_PrintText	(EUT_TagBase+41)	 /* Printe Text ins Statusdisplay */
				 /* INPUT : a0=Text für ScrollWindow */
				 /* OUTPUT: d0=NULL oder Error */
#define EUT_NewDirectory	(EUT_TagBase+42) /* Es wurde ein neues Directory gelesen */
				 /* INPUT : d0=Dirs */
				 /* 	d1=Files */
				 /* OUTPUT: - */
#define EUT_NewPlayer	(EUT_TagBase+43)	 /* Es wurde ein Player hinzugeladen */
				 /* INPUT : d0=Anz der Player im EP */
				 /* OUTPUT: - */
#define EUT_NewEngineLoaded	(EUT_TagBase+44) /* Es wurde ein neues Engine geladen */
				 /* INPUT : - */
				 /* OUTPUT: - */
#define EUT_KillModule	(EUT_TagBase+45)	 /* Das geladene Module wurde entfernt */
				 /* INPUT : - */
				 /* OUTPUT: - */
#define EUT_NewAmplifierList	(EUT_TagBase+46) /* Die Amplifierliste ist geändert */
				 /* worden */
				 /* INPUT:	a0=AmplifierList */
				 /* OUTPUT: - */
#define EUT_CheckForPlayer	(EUT_TagBase+47) /* Testet den angegebenen Speicherbe- */
				 /* reich nach möglichen Playern und gibt */
				 /* die Namen in einer EPT-Textstructure */
				 /* zurück. */
				 /* INPUT : d0=Size des Speicherbereichs */
				 /* 	d1=Size des Files */
				 /* 	a0=Adresse des Files */
				 /* 	a1=Name des Files */
				 /* 	a2=PlayerList */
				 /* OUTPUT: d0=Error oder NULL */
				 /* 	a0=EPT-Textstucture */
#define EUT_Ripp	(EUT_TagBase+48)	 /* Input: a0=Memstart */
				 /* 	d0=MemSize */
				 /* 	a1=Filename */
				 /* Output:d0=Error oder NULL */
				 /* 	d1=ModuleSize */
				 /* 	a0=Modulestart */
				 /* 	a1=Ripperstruct (read only) */
#define EUT_RippSegment	(EUT_TagBase+49)	 /* Such in einem Segment nach Modulen. */
				 /* (unbenutzt in V2.0) */
				 /* Input: d0=Segment */
				 /* Output:d0=Error oder NULL */
				 /* 	d1=ModuleSize */
				 /* 	a0=Modulestart */
				 /* 	a1=Ripperstruct (read only) */
#define EUT_RippCont	(EUT_TagBase+50)	 /* Führt die Suche nach Modulen fort. */
				 /* Input: a0=Ripperstruct */
				 /* Output:d0=Error oder NULL */
				 /* 	d1=ModuleSize */
				 /* 	a0=Modulestart */
				 /* 	a1=Ripperstruct (read only) */
#define EUT_RippExt	(EUT_TagBase+51)	 /* Dient zum Suchen der Samples von */
				 /* Modulen (z.B. Smpl.TRSI) */
				 /* Input: a0=Ripperstruct */
				 /* Output:d0=Error oder NULL */



				 /* ------------ stored !!!! -----------* */
				 /* 	d1=Samplesize */
				 /* 	a0=Samplestart */
				 /* 	a1=Ripperstruct (read only) */
#define EUT_Unused6	(EUT_TagBase+52)
#define EUT_ConvertModule	(EUT_TagBase+53) /* Convertiert ein Module in das angege- */
				 /* bene Format. */
				 /* Der alte Speicherbereich wird NICHT!!! */
				 /* freigegeben !!!!!!!!!!! */
				 /* INPUT : d0=Size des Modules */
				 /* 	d1=DefaultMemory */
				 /* 	d2=Offset zur Size */
				 /* 	a0=DataPtr */
				 /* OUTPUT: d0=NULL oder Error */
				 /* 	d1=Targetfiles */
				 /* 	d2=Flags */
				 /* 		0=Module nun gesplittet */
				 /* 		a0=SongAdr a1=Sample */
				 /* 		1=Kein OriginalMod mehr */
				 /* 		2=Sample im OriginalMod */
				 /* 		3=Sample im FastRAM */
				 /* 	a0=EPT_TestSt. Playerloading */
				 /* 	a1=PlayerTaglist */
				 /* 	a2=LFPuffer (max 100 Files) */
#define EUT_Interrupt	(EUT_TagBase+54)	 /* (APTR) Intereinsprung (nach Angaben von */
				 /* EUS_Ticks/EUS_Counter ... */
				 /* d0=0	Scroller ist fertig */
				 /* sonst d0<>0 */

#define EUT_InitDisplay	(EUT_TagBase+55)	 /* Bar für's Packen & Entpacken */
				 /* Input: a0=HintergrundText */
				 /* Output:d0?Null oder Error */
#define EUT_FillDisplay	(EUT_TagBase+56)	 /* Input: d0=Füllen in Prozent */
				 /* Output -- */
#define EUT_RemoveDisplay	(EUT_TagBase+57) /* Input: -- */
				 /* Output: -- */
#define EUT_StringGadget	(EUT_TagBase+58) /* Input:	d0=Mode 0=Ziffern */
				 /* 		1=Text */
				 /* 		!2=Invisible */
				 /* 	d1=MinValue */
				 /* 	d2=MaxValue od Maxlen */
				 /* 	a0=Puffer oder Adr */
				 /* 	a1=TextFmt */
				 /* 	a2=TextFmtArgs */
				 /* 	a3=REQUestertitle */
#define EUT_AddAbortGadget	(EUT_TagBase+59) /* Output:d0=Error oder NULL */
#define EUT_RemoveAbortGadget	(EUT_TagBase+60) /* Input :-- */
#define EUT_GetAbortMsg	(EUT_TagBase+61)	 /* Input: -- */
				 /* Output:d0=Error oder NULL */
				 /* 	d1=0 keine Reaktion */
				 /* 	d1=1 Abort gedrückt */
#define EUT_ChangeGui	(EUT_TagBase+62)	 /* Das Gadgets des Gui's werden erlaubt */
				 /* bzw erlaubt. Die Gadgets sind removed. */
				 /* kein Refresh (wird über AddGList ge- */
				 /* macht) */
				 /* Input:	d0= Aktionnr. */
				 /* 	d1=Zustand 0=enable 1=disable */
				 /* Output:-- */
#define EUT_ChangePrefs	(EUT_TagBase+63)	 /* Es wurde im EP irgendetwas geändert. */
				 /* z.B Volume über Arexx oder Zustand */
				 /* Es muß refresht werden. */
				 /* INPUT : d0=Nr der Aktion */
				 /* 		d1=Zustand 0=Enable 1=Disable */
				 /* 			-1=keine Änderung */
				 /* 		d2= * Wert z.B. Volumewert */
				 /* 		* 0=Aus 1=Ein */
				 /* 	a0-a3=Parameter */
				 /* OUTPUT: d0=Error oder NULL */
#define EUT_GetGuiMenuStrip	(EUT_TagBase+64) /* Es wird ein Menustrip erzeugt, der */
				 /* im Eagleplayermenu mit aufgenommen */
				 /* wird. Der Menustrip wird initialisiert */
				 /* Das Layout erledigt der EP */
				 /* Input: -- */
				 /* Ouput: d0=Error oder NULL */
				 /* 	a0=MenuStrip */
#define EUT_FreeGuiMenuStrip	(EUT_TagBase+65) /* Der benutzte Menuzstrip kann freige- */
				 /* geben werden. */
				 /* Input: a0=MenuStrip */
				 /* Ouput: d0=Error oder NULL */
#define EUT_GetPrefs	(EUT_TagBase+66)	 /* Der Eagleplayer fragt etwas vom Gui */
				 /* ab */
				 /* Input: d0=EPNr ??? */
				 /* Output: */
#define EUT_SaveConfig	(EUT_TagBase+67)	 /* Hier wird hingesprungen, wenn der EP */
				 /* Saveconfig aufruft. */

#define EUT_TestValidArchive	(EUT_TagBase+68) /* Teste, on File ein Archive ist */
				 /* Input:	a0=Pfad des Archives */
				 /* Output:d0=Error oder NULL=Ok */
#define EUT_OpenArchive	(EUT_TagBase+69)	 /* Öffne ein Archive zum Dirscannen */
				 /* Input: a0=Pfad des Archives */
				 /* 	d0=Error oder NULL */
				 /* 	a0=Archiveinfo */
#define EUT_CloseArchive	(EUT_TagBase+70) /* Schließe Archive wieder */
				 /* Input:	a0=Archiveinfo */
#define EUT_ArchiveExNext	(EUT_TagBase+71) /* Gibt den nächsten Eintrag aus dem */
				 /* Archive im WorkPuffer mit WSize */
				 /* zurück */
				 /* Input:	a0=ArchiveInfo */
				 /* 	a1=ZielPuffer */
				 /* 	d1=ZielPufferSize */
				 /* Output:d0=Error oder NULL */
#define EUT_ExtractArchiveEntry	(EUT_TagBase+72) /* Entpackt einen Eintrag aus dem Archive */
				 /* nach T: + Pfad(!!!) des Entries */
				 /* Input:	a0=Pfad des Archives */
				 /* 	a1=Pfad des Namens im Archive */
				 /* 	a2=WorkPuffer für Befehlspfad */
				 /* 	d2=WorkPufferSize */

#define EUT_RemPlayerList	(EUT_TagBase+73)
#define EUT_RemEnginesList	(EUT_TagBase+74)
#define EUT_RemModulesList	(EUT_TagBase+75)
	
#define EUT_Unused1	(EUT_TagBase+76)
#define EUT_Unused2	(EUT_TagBase+77)
#define EUT_Unused3	(EUT_TagBase+78)

#define EUT_EPSubItems	(EUT_TagBase+79)	 /* TI_Data= Ptr to Menustrip for SubItem */
				 /* 	  in Enginemenu in EPFormat */

#define EUT_LocaleTable	(EUT_TagBase+80)	 /* TI_Data=Ptr to Table of */
				 /* .w Localenr + .l Ptr to orig. Text */
				 /* + .l to localeText */
				 /* EP will check the locale-catalogs and */
				 /* wil store a Pointer to .l */
				 /* End with ZERO */

#define EUT_LoadConfig	(EUT_TagBase+81)	 /* Hier wird hingesprungen, wenn der EP */
				 /* Loadconfig macht. */
#define EUT_UpdateEngineMenu	(EUT_TagBase+82) /* Hier wird hingesprungen, wenn der EP */
				 /* im Helpmodus ist und die Hilfe eines */
				 /* Schalters angewählt wird. In dieser */
				 /* Routine wird der Haken wieder richtig */
				 /* gesetzt.  */
#define EUT_HelpNodeName	(EUT_TagBase+83) /* Name der Helpnode für das Engine */

#define EUT_SpecialinfoLNr	(EUT_TagBase+84) /* Helpnummer für die Specialinfos der Engines */
#define EUT_SpecialinfoArgs	(EUT_TagBase+85) /* Argumente für die Specialinfos */

#define EUT_AttnFlags	(EUT_TagBase+86)	 /* rEQUired CPU/FPU type, Exec style */

#define EUT_EngineCommand	(EUT_TagBase+87)
#define EUT_FreeRipperstruct	(EUT_TagBase+88) /* a0=Ripperstruct */

#if 0
/*----------------------------------------------------------------------------*/
#define EUT_AddModulesMenu	(EUT_TagBase+89) /* Das intern geladen Directory wird in */
				 /* das Menu eingebaut */
				 /* INPUT : d0=Anzahl der Einträge */
				 /* 	a0=1. Modulesstrukture */
				 /* OUTPUT: d0=NULL oder Error */
				 /* 	d1=Anzahl der angezeigten */
				 /* 		Eintrge */
#define EUT_RemoveModulesMenu	(EUT_TagBase+90) /* Das intern geladene Directory wird */
				 /* wieder aus dem Menu ausgebaut. */
				 /* INPUT : - */
				 /* OUTPUT: d0=NULL oder Error */
#endif

/*------- end of player interface enumeration for EaglePlayer-Engine ------*/

/*---------------------------- Windowtypes - Types ---------------------------*/
#define EPST_AllWindows		 	0
#define EPST_MainWindow		 	1
#define EPST_PlayerWindow	 	2
#define EPST_UserprogramWindow	 	3
#define EPST_PreferenceWindow	 	4
#define EPST_ModuleInfoWindow	 	5
#define EPST_SampleInfoWindow	 	6
#define EPST_TextREQUest	 	7		/*intern !!!*/

/*------------------------------- Gadgettypes --------------------------------*/
#define EUTGTY_AbortGadget	 	1
#define EUTGTY_StringGadget	 	2
#define EUTGTY_YesNoGadget	 	3
#define EUTGTY_RetryCancelGadget	4

/*----------------------- EagleUserProgram-Identifier ------------------------*/
#define EUSN_Identifier {'E','P','E','n','g','i','n','e'}

/*------------------------- Userprogramm FreeTabelle -------------------------*/
/*- Here now a Table for all libraries that the userprograms use.	     -*/
/*- If an userprogram crashes the eagleplayer will close these windows/screens*/
/*- libraries/Interrupts/subtasks and will free the Memory at MemAdr	     -*/
#define EUSM_ListEnd	 	0
#define EUSM_Ignore	 	1
#define EUSM_Arg1	 	2
#define EUSM_Arg2	 	3
#define EUSM_Arg3	 	4
#define EUSM_Arg4	 	5
#define EUSM_Interrupt	 	10
#define EUSM_SubTask	 	11
#define EUSM_Segment	 	12
#define EUSM_Port13	 	13		/*Remport (no freeMem)*/
#define EUSM_MsgPort	 	14		/*deleteMsgPort*/
#define EUSM_Msg	 	15		/*will replyed*/
#define EUSM_Lock	 	16
#define EUSM_Device	 	17
#define EUSM_LibraryBase 	18
#define EUSM_Window	 	19
#define EUSM_Screen	 	20
#define EUSM_Raster	 	21		/*FreeRaster*/
#define EUSM_Font	 	22
#define EUSM_MemAdr	 	23
#define EUSM_MemListAdr	 	24
#define EUSM_MenuStrip	 	25
#define EUSM_GadgetAdr	 	26
#define EUSM_VisualInfo	 	27
#define EUSM_PubScreenAdr	28
#define EUSM_MaxNummer	 	28


/*----------------- Analyzer and Eagleplayer Message-Structure ---------------*/
struct UM_Message {
	struct Message UM_Message;	/*Exec-Message*/
	APTR	UM_TaskAdr;		/*Engine-TaskAdr*/
	ULONG	UM_Signal;		/*Engine-Signal for Interrupt-Signal*/
	ULONG	UM_Type;		/*MessageType*/
	UWORD	UM_Class;		/*Message-Class*/
	APTR	UM_StructAdr;		/*UPS_StructAdr*/
	APTR	UM_UserPort;		/*Pointer to UserProgram-Port*/
	UWORD	UM_UserNr;		/*Engine-Nr*/
	UWORD	UM_Enabled;		/*1=Enable (Window close->no Signal)*/
	ULONG	UM_Command;		/*Command from Userprog to EP*/
	ULONG	UM_Argstring;		/*Ptr to Argstruct*/
	ULONG	UM_Result;		/*Result*/
	APTR	UM_UserWindow;		/*Pointer to Window of Userprogram*/
}; /*	UM_SizeOf */


#define UM_EnginePort	 	UM_UserPort	/*Pointer to UserProgram-Port*/
#define UM_EngineNr	 	UM_UserNr
#define UM_EngineWindow	 	UM_UserWindow
#define UM_Arglist	 	UM_Argstring


/*------------------------------- Messagetypes -----------------------------*/
#define USM_UserPrg		 	1	/*Message from UserPrg to EP*/
#define USM_Eagleplayer		 	2	/*Message from Eagleplayer to ...*/
#define USM_Externalprg		 	3	/*Message from an External Program to EP*/

#define USM_Engine		 	USM_UserPrg	/*Message from Engine to EP*/

/*----------------------------- Message-Classes ----------------------------*/
#define USClass_Dummy		 	0	/*no class (maybe new StructAdr)*/
#define USClass_Activate	 	1	/*Start Work e.g. Open Window*/
#define USClass_DeActivate	 	2	/*e.g. Close Window (just Pause,NO Exit!)*/
#define USClass_Exit		 	3	/*Exit Engine (only to Engine)*/
#define USClass_Zipwin		 	4
#define USClass_Unzipwin	 	5
#define USClass_NewEngine	 	6	/*Return-Message bei Engine-Start*/
#define USClass_GetConfig	 	7	/*EUS_Flags,EUS_Winx,EUS_Winy,EUS_special have*/
					/*to be initialized by Userprogram	before*/
					/*replying the Message*/
#define USClass_NewConfig	 	8	/*EUS_Flags,EUS_Winx,EUS_Winy,EUS_special*/
					/*countain new Values (the Userprogram has to*/
					/*utilize this new datas)*/
#define USClass_LockEP		 	9	/*Eagleplayer sets Waiting Pointer and*/
					/*does nothing until USCLASS_UNLOCKEP*/
					/*comes, be shure to unlock the EP*/
					/*again (be carefully, call UnLock only*/
					/*as often as you called Lock)*/
#define USClass_UnLockEP	 	10	/*Free Eagleplayer to work on*/
#define USClass_Command		 	11	/*Command to Eagleplayer, future*/
					/*use (e.g. NextPattern,Stop)*/
#define USClass_Answer		 	12	/*Command came back, in UM_Arg and*/
					/*UM_Arg2 results*/
#define USClass_QuitEagle	 	13	/*Makes the Eagleplayer quit*/


#define USClass_NewModule	 	14
#define USClass_NewSong		 	15
#define USClass_NewPreference	 	16
#define USClass_StartInt	 	17
#define USClass_StopInt		 	18
#define USClass_PlaySample	 	19
#define USClass_NewScrollText	 	20
#define USClass_NewDirectory	 	21
#define USClass_NewPlayer	 	22
#define USClass_NewEngineLoaded  	23	/*for Engine-Manager*/
#define USClass_KillModule	 	24
#define USClass_Surface		 	25
#define USClass_ActiveSurface	 	26
#define USClass_NewAmplifierlist  	27
#define USClass_Show		 	28	/*only show Window*/
#define USClass_Hide		 	29	/*only hide Window*/
#define USClass_SaveConfig	 	30
#define USClass_RemPlayerList	 	31
#define USClass_RemEnginesList	 	32
#define USClass_RemModulesList	 	33
#define USClass_LoadConfig	 	34
#define USClass_SetWaitPointer	 	35
#define USClass_ClearWaitPointer  	36
#define USClass_EngineCommand	 	37
#define USClass_SizeOf		 	37

#define USClass_NewUserPrg	 	USClass_NewEngine
#define USClass_NewUserPrgLoaded  	USClass_NewEngineLoaded

/*----------------------------------------------------------------------------*/
struct EaglePlayerText {
	APTR	EPT_Next;		/*Pointer to Next EPT-Struct*/
	LONG	EPT_Result1;		/*Arg or Returnvalue*/
	LONG	EPT_Result2;		/*Arg or Returnvalue*/
	LONG	EPT_StringSize;		/*Size of allocated Mem (Struct+String)*/
};

struct EngineArgs {
	APTR	ENA_Next;		/*Pointer to Next EngineArgs*/
	LONG	ENA_Arg1;		/*Argument 1*/
	LONG	ENA_Arg2;		/*Argument 2*/
	LONG	ENA_Size;		/*Size of EngineArg-Struct*/
};

/*----------------------------- Nummer der Aktionen ---------------------------*/

enum {
			EPNr_Dummy,		/*unused*/

	/*-------------------- Eagleplayer-Windows -------------------*/
			EPNr_MainWindow,

	/*--------------------- General-Funktionen -------------------*/
			EPNr_LoadConfig,
			EPNr_SaveConfig,
			EPNr_Help,
			EPNr_DeleteFile,
			EPNr_AboutEP,
			EPNr_Iconify,
			EPNr_ToggleFilter,
			EPNr_Hide,
			EPNr_Quit,
			EPNr_Voices,		/*d0=Voice d1=state*/
			EPNr_Module,		/*\*/
			EPNr_Player,		/* > d0=EPNr d1=Mod/Pl/UPrgNr*/
			EPNr_Engine,		/* */
			EPNr_PlayMem,
			EPNr_PublicScreen,
			EPNr_Abort,
			EPNr_Clock,
			EPNr_HotKey,
			EPNr_CXPriority,
			EPNr_Status,
			EPNr_Status2,		/* for Engines*/

	/*---------------------- Module Funktionen -------------------*/
			EPNr_LoadModule,	/*d0=0->select in FilerEQUester*/
			EPNr_SaveModule,
			EPNr_SaveModulePrefs,
			EPNr_PrevModule,
			EPNr_PrevSong,
			EPNr_PrevPattern,
			EPNr_ReplaySong,
			EPNr_Play,
			EPNr_PlayFaster,	/*nur Pause*/
			EPNr_NextPattern,
			EPNr_NextSong,
			EPNr_NextModule,
			EPNr_StopPlay,
			EPNr_EjectModule,
			EPNr_AboutModule,
			EPNr_SubSong,
			EPNr_GetSampleInfo,
			EPNr_FreeSampleInfo,

	/*---------------------- Player Funktionen -------------------*/
			EPNr_LoadPlayer,
			EPNr_LoadPlayerDir,
			EPNr_DeletePlayer,
			EPNr_DeleteAllPlayer,
			EPNr_PlayerState,		/*oder disable*/
			EPNr_PlayerConfig,
			EPNr_LoadPlayerConfig,
			EPNr_SavePlayerConfig,

	/*--------------------- Engine-Funktionen -------------------*/
			EPNr_LoadEngine,
			EPNr_LoadEngineDir,
			EPNr_DeleteEngine,
			EPNr_DeleteAllEngines,
			EPNr_LoadEngineConfig,
			EPNr_SaveEngineConfig,

	/*---------------------- Listen-Funktionen -------------------*/
			EPNr_InsertModulesList,

	/*---------------------- Font-Funktionen ---------------------*/
			EPNr_MenuFont,
			EPNr_ModulesFont,
			EPNr_ScrollFont,
			EPNr_FontReserved,
			EPNr_FontReserved2,

	/*---------------------- Einstellungssachen ------------------*/

			/*------------ Allgemein --------------*/
			EPNr_QuickStart,
			EPNr_ScrollInfos,
			EPNr_SongName,
			EPNr_Filter,
			EPNr_MasterVolume,
			EPNr_SaveT,
			EPNr_FadeIn,
			EPNr_Fadeout,
			EPNr_Quiteagle,
			EPNr_EjectPlayers,
			EPNr_EjectEngines,
			EPNr_PlayerBatch,
			EPNr_Prefix,
			EPNr_LoadFast,
			/*----------- Dir/ModulesPrefs ---------*/
			EPNr_LoadDir,
			EPNr_EPDir,
			EPNr_HideAll,
			EPNr_RescanDir,
			EPNr_Notify,
			EPNr_ScanAlways,
			EPNr_Reserved,
			EPNr_LoadParentDir,
			/*------------ Programm ---------------*/
			EPNr_ProgramMode,
			EPNr_LoadBefore,
			EPNr_RandomSong,
			EPNr_AutoSubSong,
			EPNr_LoadAlways,
			EPNr_SongEnd,
			EPNr_CalcDuration,
			EPNr_DirJump,
			EPNr_RandomStart,
			EPNr_TimeMode,
			EPNr_PlayTime,
			EPNr_MinPlayTime,
			/*--------------- SaveMode ------------*/
			EPNr_CrunchMode,
			EPNr_SaveDir,
			EPNr_SSDir,
			EPNr_SafeSave,
			EPNr_SampleMode,
			EPNr_SaveAsProTracker,
			EPNr_Protect,
			EPNr_ProtectionBits,
			EPNr_XPKCrunchMode,
			EPNr_Overwrite,
			EPNr_AutomaticSave,
			EPNr_Password,
			/*-------------- Timing ---------------*/
			EPNr_TimingMode,
			EPNr_WaitingMode,
			EPNr_RasterLines,
			EPNr_DBFLoops,
			EPNr_AllocChannels,
			EPNr_SoftInt,

			EPNr_AutoPassword,
			EPNr_XFDDecrunch,
			EPNr_XFDLoadSeg,

			EPNr_Volume,
			EPNr_VolumeDefault,
			EPNr_VolumeLower,
			EPNr_VolumeHigher,

			EPNr_Balance,
			EPNr_BalanceDefault,
			EPNr_BalanceLeft,
			EPNr_BalanceRight,

			EPNr_Speed,
			EPNr_SpeedDefault,
			EPNr_SpeedFaster,
			EPNr_SpeedSlower,

			EPNr_SmallModule,
			EPNr_BigModule,
			EPNr_ModuleFault,

			EPNr_FileREQUesterMode,
			EPNr_IconifyMode,

			EPNr_ChipRamAmplifier,
			EPNr_FastRamAmplifier,
			EPNr_PrintText,
			EPNr_ScrollText,
			EPNr_LockModule,

			EPNr_EngineShowState,	/* Konfiguration Show/Hide */
			EPNr_EngineItemCommand,	/* Item aus dem Menu */
			EPNr_EngineConfig,
			EPNr_Documentation,
			EPNr_NoGui,
			EPNr_ExtractFirstModule,
			EPNr_HelpLink,
			EPNr_Instrumentsdir,
			EPNr_NoEngine,
			EPNr_AscEngine,
			EPNr_LoadList,
			EPNr_SaveList,
			EPNr_SetAuthor,
			EPNr_InsertPysionList,
			EPNr_FormatLoad,
			EPNr_PlaySample,
			EPNr_Config,
			EPNr_AddEntry,		/* Rev 12*/
			EPNr_FreeDirPuffer,	/* Rev 12*/
			EPNr_ScreenMode,	/* Rev 13*/
			EPNr_SetPosition,	/* Rev 13*/
			EPNr_LastInternal 
 }; /* enum */

#define EPNr_FirstExternal	 	$6000
#define EPNr_LastExternal	 	$6fff
#define EPNr_FirstEngine	 	$7000
#define EPNr_LastEngine		 	$77ff
#define EPNr_FirstPlayer	 	$7800
#define EPNr_LastPlayer		 	$7fff
#define EPNr_FirstModule	 	$8000
#define EPNr_LastModule		 	$FFFF

#define EPNrI_AppIcon		 	0		* IconifyModi
#define EPNrI_AppItem		 	1
#define EPNrI_MaxId		 	1

#define EPNrF_Req		 	0
#define EPNrF_ReqTools		 	1
#define EPNrF_ASL		 	2
#define EPNrF_MaxId		 	2

#define EPNrC_Uncrunched	 	0		* CrunchModi
#define EPNrC_PPCrunched	 	1
#define EPNrC_LHCrunched	 	2
#define EPNrC_XPKCrunched	 	3
#define EPNrC_CrMCrunched	 	4
#define EPNrC_MaxId		 	4

#define EPNrT_CiaTiming		 	0
#define EPNrT_VBlankTiming	 	1
#define EPNrT_TimerDevice	 	2
#define EPNrT_Amplifier			3
#define EPNrT_MaxId			3

#define EPNrW_Automaticwait	 	0
#define EPNrW_RasterWait	 	1
#define EPNrW_DBFWait		 	2
#define EPNrW_MaxId		 	2

#define EPNrP_NoNextModule	 	0
#define EPNrP_PrevModule	 	1
#define EPNrP_NextModule	 	2
#define EPNrP_RandomModule	 	3
#define EPNrP_MaxId		 	3

#define EP_MaxVolume		 	255		/*v2.0*/
#define EP_DefVolume		 	255		/*v2.0*/
#define EP_MaxVoices		 	32		/*v2.0*/
#define EP_MaxSpeed		 	25		/*v2.0 -25 to 25*/

#define EPFF_Proportional	 	1		/*proportinal-font*/
#define EPFB_Proportional	 	(1<<EPFF_Proportional)

#define EPRMF_DownGadget	 	1		/* z.B Volumegadget gedrückt */
#define EPRMF_NoGuiInfo		 	2
#define EPRMB_DownGadget	 	(1<<EPRMF_DownGadget)
#define EPRMB_NoGuiInfo		 	(1<<EPRMF_NoGuiInfo)

/*-------------------------------------------------------------------------*
 * Nummern für Moduleseigenschaften (Pysion)
		RS_RESET
		RS_BYTE	EPMENr_Dummy,1
		RS_BYTE	EPMENr_Songname,1
		RS_BYTE	EPMENr_Specialinfo,1
		RS_BYTE	EPMENr_Group,1
 *--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Locale IDs: these allow to retrieve localized strings for common texts  */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
#define LNr_FirstInternal	 	-1
#define LNr_FirstExternal	 	1
#define LNr_FirstEngine		 	$3000
#define LNr_FirstPlayer		 	$6000

/* FIXME: not converted to C yet */
#if 0
		RS_RESET
		RS_BYTE	LNr_Dummy,LNr_FirstInternal
		RS_BYTE	LNr_StringError,-1
		RS_BYTE	LNr_EPVersion,-1
		RS_BYTE	LNr_Dosname,-1
		RS_BYTE	LNr_Intuiname,-1
		RS_BYTE	LNr_IconName,-1
		RS_BYTE	LNr_Graphname,-1
		RS_BYTE	LNr_GadToolsName,-1
		RS_BYTE	LNr_LocaleName,-1
		RS_BYTE	LNr_Reqname,-1
		RS_BYTE	LNr_ASLName,-1
		RS_BYTE	LNr_ReqToolsName,-1
		RS_BYTE	LNr_LhName,-1
		RS_BYTE	LNr_PPName,-1
		RS_BYTE	LNr_WBName,-1
		RS_BYTE	LNr_DiskFontName,-1
		RS_BYTE	LNr_ScreenNotifyName,-1
		RS_BYTE	LNr_AmigaGuideName,-1
		RS_BYTE	LNr_CommodityName,-1
		RS_BYTE	LNr_XPKName,-1
		RS_BYTE	LNr_XFDMasterName,-1
		RS_BYTE	LNr_CrMName,-1
		RS_BYTE	LNr_Audioname,-1
		RS_BYTE	LNr_CiaBname,-1
		RS_BYTE	LNr_TopazName,-1
		RS_BYTE	LNr_RexxSysName,-1
		RS_BYTE	LNr_Prozessname,-1
		RS_BYTE	LNr_EPEnginePort,-1
		RS_BYTE	LNr_EPReplayEnginePort,-1
		RS_BYTE	LNr_EPAppPort,-1
		RS_BYTE	LNr_EPArexxPort,-1
		RS_BYTE	LNr_EPScreenNotifyPort,-1
		RS_BYTE	LNr_InterruptName,-1
		RS_BYTE	LNr_TimerName,-1
		RS_BYTE	LNr_SoftIntName,-1
		RS_BYTE	LNr_EPAudioName,-1
		RS_BYTE	LNr_EPNotifyName,-1
		RS_BYTE	LNr_EPCommodityName,-1
		RS_BYTE	LNr_CommoTitle,-1
		RS_BYTE	LNr_CommoText,-1
		RS_BYTE	LNr_SaveKopf,-1
		RS_BYTE	LNr_EngineDir,-1
		RS_BYTE	LNr_EaglePlayerDir,-1
		RS_BYTE	LNr_MainGuideName,-1
		RS_BYTE	LNr_EngineGuideName,-1
		RS_BYTE	LNr_PlayerGuideName,-1
		RS_BYTE	LNr_GuideALink,-1
		RS_BYTE	LNr_MainLink,-1
		RS_BYTE	LNr_ShowLink,-1
		RS_BYTE	LNr_InstrumentsDir,-1
		RS_BYTE	LNr_TName,-1
		RS_BYTE	LNr_EPTName,-1
		RS_BYTE	LNr_EnvName,-1
		RS_BYTE	LNr_EnvarcName,-1
		RS_BYTE	LNr_SName,-1
		RS_BYTE	LNr_DevsName,-1
		RS_BYTE	LNr_LocaleAssign,-1
		RS_BYTE	LNr_EPAssign,-1
		RS_BYTE	LNr_ConfigDir,-1
		RS_BYTE	LNr_EPDirName,-1
		RS_BYTE	LNr_ConfigDirName,-1
		RS_BYTE	LNr_ConfigName,-1
		RS_BYTE	LNr_KeyName,-1
		RS_BYTE	LNr_PlayerBatchName,-1
		RS_BYTE	LNr_MainCatalog,-1
		RS_BYTE	LNr_EngineCatalog,-1
		RS_BYTE	LNr_PlayerCatalog,-1
		RS_BYTE	LNr_DefaultGui,-1
		RS_BYTE	LNr_WindowTitleRegistered,-1
		RS_BYTE	LNr_WindowTitleGeneric,-1
		RS_BYTE	LNr_EPTitle,-1
		RS_BYTE	LNr_CommentText,-1
		RS_BYTE	LNr_Yes,-1
		RS_BYTE	LNr_On,-1
		RS_BYTE	LNr_No,-1
		RS_BYTE	LNr_Off,-1
		RS_BYTE	LNr_1,-1
		RS_BYTE	LNr_0,-1
		RS_BYTE	LNr_Toggle,-1
		RS_BYTE	LNr_Toggle2,-1
		RS_BYTE	LNr_DefaultHotkey,-1
		RS_BYTE	LNr_DefPubScreen,-1
		RS_BYTE	LNr_Str_Status,-1
		RS_BYTE	LNr_Str_Popup,-1
		RS_BYTE	LNr_Str_PlayFaster,-1
		RS_BYTE	LNr_Str_HotKey,-1
		RS_BYTE	LNr_Str_Priority,-1
		RS_BYTE	LNr_Str_ChipRamAmplifier,-1
		RS_BYTE	LNr_Str_FastRamAmplifier,-1
		RS_BYTE	LNr_Str_PrintText,-1
		RS_BYTE	LNr_Str_ScrollText,-1
		RS_BYTE	LNr_Str_MenuFont,-1
		RS_BYTE	LNr_Str_ModulesFont,-1
		RS_BYTE	LNr_Str_ScrollFont,-1
		RS_BYTE	LNr_Str_Help,-1
		RS_BYTE	LNr_Str_DeleteFile,-1
		RS_BYTE	LNr_Str_Iconify,-1
		RS_BYTE	LNr_Str_LoadConfig,-1
		RS_BYTE	LNr_Str_SaveConfig,-1
		RS_BYTE	LNr_Str_Config,-1
		RS_BYTE	LNr_Str_AboutEP,-1
		RS_BYTE	LNr_Str_Hide,-1
		RS_BYTE	LNr_Str_Quit,-1
		RS_BYTE	LNr_Str_LoadGui,-1
		RS_BYTE	LNr_Str_DeleteGui,-1
		RS_BYTE	LNr_Str_LoadPlayer,-1
		RS_BYTE	LNr_Str_LoadPlayerDir,-1
		RS_BYTE	LNr_Str_DeletePlayer,-1
		RS_BYTE	LNr_Str_DeleteAllPlayer,-1
		RS_BYTE	LNr_Str_Enable,-1
		RS_BYTE	LNr_Str_LoadPlayerConfig,-1
		RS_BYTE	LNr_Str_SavePlayerConfig,-1
		RS_BYTE	LNr_Str_PlayerConfig,-1
		RS_BYTE	LNr_Str_LoadEngine,-1
		RS_BYTE	LNr_Str_LoadEngineDir,-1
		RS_BYTE	LNr_Str_DeleteEngine,-1
		RS_BYTE	LNr_Str_DeleteAllEngines,-1
		RS_BYTE	LNr_Str_XPKPackmethod,-1
		RS_BYTE	LNr_Str_Password,-1
		RS_BYTE	LNr_Str_InstrumentsDir,-1
		RS_BYTE	LNr_Str_Module,-1
		RS_BYTE	LNr_Str_LoadModule,-1
		RS_BYTE	LNr_Str_SaveModule,-1
		RS_BYTE	LNr_Str_SaveModulePrefs,-1
		RS_BYTE	LNr_Str_SaveDir,-1
		RS_BYTE	LNr_Str_Eject,-1
		RS_BYTE	LNr_Str_PrevSong,-1
		RS_BYTE	LNr_Str_NextSong,-1
		RS_BYTE	LNr_Str_SubSong,-1
		RS_BYTE	LNr_Str_PrevPattern,-1
		RS_BYTE	LNr_Str_NextPattern,-1
		RS_BYTE	LNr_Str_PrevModule,-1
		RS_BYTE	LNr_Str_NextModule,-1
		RS_BYTE	LNr_Str_Pause,-1
		RS_BYTE	LNr_Str_Play,-1
		RS_BYTE	LNr_Str_Stop,-1
		RS_BYTE	LNr_Str_ReplaySong,-1
		RS_BYTE	LNr_Str_AboutModule,-1
		RS_BYTE	LNr_Str_PubScreen,-1
		RS_BYTE	LNr_Str_Filter,-1
		RS_BYTE	LNr_Str_ToggleFilter,-1
		RS_BYTE	LNr_Str_FadeIn,-1
		RS_BYTE	LNr_Str_FadeOut,-1
		RS_BYTE	LNr_Str_QuitEagle,-1
		RS_BYTE	LNr_Str_SaveT,-1
		RS_BYTE	LNr_Str_ScrollInfos,-1
		RS_BYTE	LNr_Str_LoadFast,-1
		RS_BYTE	LNr_Str_SongName,-1
		RS_BYTE	LNr_Str_Prefix,-1
		RS_BYTE	LNr_Str_EjectPlayer,-1
		RS_BYTE	LNr_Str_EjectEngine,-1
		RS_BYTE	LNr_Str_PlayerBatch,-1
		RS_BYTE	LNr_Str_AddListEntry,-1
		RS_BYTE	LNr_Str_AddList,-1
		RS_BYTE	LNr_Str_RescanDir,-1
		RS_BYTE	LNr_Str_LoadDir,-1
		RS_BYTE	LNr_Str_ParentDir,-1
		RS_BYTE	LNr_Str_EPDir,-1
		RS_BYTE	LNr_Str_HideAll,-1
		RS_BYTE	LNr_Str_Notify,-1
		RS_BYTE	LNr_Str_ScanAlways,-1
		RS_BYTE	LNr_Str_Protect,-1
		RS_BYTE	LNr_Str_ProtectionBits,-1
		RS_BYTE	LNr_Str_SampleMode,-1
		RS_BYTE	LNr_Str_SaveAsProTracker,-1
		RS_BYTE	LNr_Str_SSDir,-1
		RS_BYTE	LNr_Str_SafeSave,-1
		RS_BYTE	LNr_Str_AutomaticSave,-1
		RS_BYTE	LNr_Str_MasterVolume,-1
		RS_BYTE	LNr_Str_Overwrite,-1
		RS_BYTE	LNr_Str_AutoPassword,-1
		RS_BYTE	LNr_Str_XFDDecrunch,-1
		RS_BYTE	LNr_Str_XFDLoadSeg,-1
		RS_BYTE	LNr_Str_SmallModule,-1
		RS_BYTE	LNr_Str_BigModule,-1
		RS_BYTE	LNr_Str_ModuleFault,-1
		RS_BYTE	LNr_Str_ProgramMode,-1
		RS_BYTE	LNr_Str_SongEnd,-1
		RS_BYTE	LNr_Str_QuickStart,-1
		RS_BYTE	LNr_Str_RandomSong,-1
		RS_BYTE	LNr_Str_LoadBefore,-1
		RS_BYTE	LNr_Str_LoadAlways,-1
		RS_BYTE	LNr_Str_DirJump,-1
		RS_BYTE	LNr_Str_TimeMode,-1
		RS_BYTE	LNr_Str_CalcDuration,-1
		RS_BYTE	LNr_Str_Randomstart,-1
		RS_BYTE	LNr_Str_AutoSubSong,-1
		RS_BYTE	LNr_Str_TimeOut,-1
		RS_BYTE	LNr_Str_MinTimeOut,-1
		RS_BYTE	LNr_Str_TimingMode,-1
		RS_BYTE	LNr_Str_WaitingMode,-1
		RS_BYTE	LNr_Str_RasterLines,-1
		RS_BYTE	LNr_Str_DBFLoops,-1
		RS_BYTE	LNr_Str_AllocChannels,-1
		RS_BYTE	LNr_Str_SoftInt,-1
		RS_BYTE	LNr_Str_IconifyMode,-1
		RS_BYTE	LNr_Str_CrunchMode,-1
		RS_BYTE	LNr_Str_FileREQUesterMode,-1
		RS_BYTE	LNr_Str_Volume,-1
		RS_BYTE	LNr_Str_Balance,-1
		RS_BYTE	LNr_Str_Voice,-1
		RS_BYTE	LNr_Str_Speed,-1
		RS_BYTE	LNr_Str_DefaultSpeed,-1
		RS_BYTE	LNr_Str_SlowerSpeed,-1
		RS_BYTE	LNr_Str_FasterSpeed,-1
		RS_BYTE	LNr_Str_LockModule,-1
		RS_BYTE	LNr_Str_NoGui,-1
		RS_BYTE	LNr_Str_ExtractFirstModule,-1
		RS_BYTE	LNr_Str_Helplink,-1
		RS_BYTE	LNr_Err_Exists,-1
		RS_BYTE	LNr_Err_Signals,-1
		RS_BYTE	LNr_Err_Dos,-1
		RS_BYTE	LNr_Err_GFX,-1
		RS_BYTE	LNr_Err_Intuition,-1
		RS_BYTE	LNr_ErrorConWin,-1
		RS_BYTE	LNr_OldSoundTracker,-1
		RS_BYTE	LNr_SoundTracker,-1
		RS_BYTE	LNr_NoiseTracker,-1
		RS_BYTE	LNr_ProTracker,-1
		RS_BYTE	LNr_Registration,-1
		RS_BYTE	LNr_Str_NoEngine,-1
		RS_BYTE	LNr_Str_ASCEngine,-1
		RS_BYTE	LNr_Str_LoadList,-1
		RS_BYTE	LNr_Str_SaveList,-1
		RS_BYTE	LNr_Str_SetAuthor,-1
		RS_BYTE	LNr_EPTName2,-1
		RS_BYTE	LNr_DracoResource,-1
		RS_BYTE	LNr_Str_FreeDirPuffer,-1	/* Rev ,12*/
		RS_BYTE	LNr_Str_ScreenMode,-1		/* Rev 13*/
		RS_BYTE	LNr_AppIconName,-1		/* Rev 13*/
		RS_BYTE	LNr_Str_SetPosition,-1		/* Rev 13*/
		RS_BYTE	LNr_LastInternal,0

	*---------- Nun die externen,localisierbaren Strings ----------*
		RS_RESET
		RS_BYTE	LNr_FirstExtDummy,LNr_FirstExternal
		RS_BYTE	LNr_Ok,1
		RS_BYTE	LNr_Yes_Extern,1
		RS_BYTE	LNr_No_Extern,1
		RS_BYTE	LNr_On_Extern,1
		RS_BYTE	LNr_Off_Extern,1
		RS_BYTE	LNr_Continue,1
		RS_BYTE	LNr_Abort,1
		RS_BYTE	LNr_Fr_LoadModule,1
		RS_BYTE	LNr_Fr_SaveModule,1
		RS_BYTE	LNr_SW_SaveModule,1
		RS_BYTE	LNr_Fr_SaveDir,1
		RS_BYTE	LNr_SW_SaveDir,1
		RS_BYTE	LNr_Fr_AddEngine,1
		RS_BYTE	LNr_SW_AddEngine,1
/*		RS_BYTE	LNr_Fr_AddEngineDir,1*/
		RS_BYTE	LNr_Fr_DeleteFile,1
		RS_BYTE	LNr_SW_DeleteFile,1
		RS_BYTE	LNr_Fr_AddPlayer,1
		RS_BYTE	LNr_SW_AddPlayer,1
*		RS_BYTE	LNr_Fr_AddPlayerDir,1
		RS_BYTE	LNr_Fr_LoadConfig,1
		RS_BYTE	LNr_SW_LoadConfig,1
		RS_BYTE	LNr_Fr_SaveConfig,1
		RS_BYTE	LNr_SW_SaveConfig,1
		RS_BYTE	LNr_Fr_InstrumentDir,1
		RS_BYTE	LNr_SW_InstrumentDir,1
		RS_BYTE	LNr_Fr_SelectFile,1
		RS_BYTE	LNr_SW_SelectFile,1
		RS_BYTE	LNr_Welcome,1
		RS_BYTE	LNr_PlayersAdded,1
		RS_BYTE	LNr_EnginesAdded,1
		RS_BYTE	LNr_EngineCommandDone,1
		RS_BYTE	LNr_Show,1
		RS_BYTE	LNr_Hide,1
		RS_BYTE	LNr_DelPlayer,1
		RS_BYTE	LNr_Loaded,1
		RS_BYTE	LNr_Reloaded,1
		RS_BYTE	LNr_DeleteAllPlayer,1
		RS_BYTE	LNr_DeleteFile,1
		RS_BYTE	LNr_LoadConfig,1
		RS_BYTE	LNr_SaveConfig,1
		RS_BYTE	LNr_Unknown,1
		RS_BYTE	LNr_LoadingInstruments,1
		RS_BYTE	LNr_Enabled,1
		RS_BYTE	LNr_Disabled,1
		RS_BYTE	LNr_SelectFont,1
		RS_BYTE	LNr_NewFontset,1
		RS_BYTE	LNr_Deleted,1
		RS_BYTE	LNr_OperationSuccesful,1
		RS_BYTE	LNr_Configurating,1
		RS_BYTE	LNr_NowLoaded,1
		RS_BYTE	LNr_Loading,1
		RS_BYTE	LNr_Decrunching,1
		RS_BYTE	LNr_Exploding,1
		RS_BYTE	LNr_Extracting,1
		RS_BYTE	LNr_Playing,1
		RS_BYTE	LNr_Pause,1
		RS_BYTE	LNr_Song,1
		RS_BYTE	LNr_CurrentPosition,1
		RS_BYTE	LNr_ReplaySong,1
		RS_BYTE	LNr_StopPlay,1
		RS_BYTE	LNr_EjectModule,1
		RS_BYTE	LNr_Crunching,1
		RS_BYTE	LNr_Saving,1
		RS_BYTE	LNr_SaveOk,1
		RS_BYTE	LNr_Voicex,1
		RS_BYTE	LNr_VoiceOff,1
		RS_BYTE	LNr_ResetVoices,1
		RS_BYTE	LNr_Quit,1
		RS_BYTE	LNr_Bytes,1
		RS_BYTE	LNr_NoNewDirLoaded,1
		RS_BYTE	LNr_Active,1
		RS_BYTE	LNr_Show2,1
		RS_BYTE	LNr_SmallPlay,1
		RS_BYTE	LNr_SmallEject,1
		RS_BYTE	LNr_Author,1
		RS_BYTE	LNr_SubSongs,1
		RS_BYTE	LNr_Duration,1
		RS_BYTE	LNr_Minuts,1
		RS_BYTE	LNr_Seconds,1
		RS_BYTE	LNr_Length,1
		RS_BYTE	LNr_Steps,1
		RS_BYTE	LNr_Pattern,1
		RS_BYTE	LNr_Voices,1
		RS_BYTE	LNr_Samples,1
		RS_BYTE	LNr_SynthSamples,1
		RS_BYTE	LNr_SongSize,1
		RS_BYTE	LNr_SamplesSize,1
		RS_BYTE	LNr_ChipSize,1
		RS_BYTE	LNr_EQUalSize,1
		RS_BYTE	LNr_Creator,1
		RS_BYTE	LNr_Help,1
		RS_BYTE	LNr_Documentationloaded,1
		RS_BYTE	LNr_QuickStart,1
		RS_BYTE	LNr_ScrollInfos,1
		RS_BYTE	LNr_MasterVolume,1
		RS_BYTE	LNr_Filter,1
		RS_BYTE	LNr_FadeIn,1
		RS_BYTE	LNr_FadeOut,1
		RS_BYTE	LNr_QuitEagle,1
		RS_BYTE	LNr_SaveT,1
		RS_BYTE	LNr_Songname,1
		RS_BYTE	LNr_Prefix,1
		RS_BYTE	LNr_LoadFast,1
		RS_BYTE	LNr_EjectPlayers,1
		RS_BYTE	LNr_EjectEngines,1
		RS_BYTE	LNr_PlayerBatch,1
		RS_BYTE	LNr_LoadDir,1
		RS_BYTE	LNr_NewDirLoaded,1
		RS_BYTE	LNr_EPDir,1
		RS_BYTE	LNr_HideAll,1
		RS_BYTE	LNr_RescanDir,1
		RS_BYTE	LNr_Notify,1
		RS_BYTE	LNr_ScanAlways,1
		RS_BYTE	LNr_UnCrunched,1
		RS_BYTE	LNr_PPCrunched,1
		RS_BYTE	LNr_LHCrunched,1
		RS_BYTE	LNr_XPKCrunched,1
		RS_BYTE	LNr_CrMCrunched,1
		RS_BYTE	LNr_SetSaveDir,1
		RS_BYTE	LNr_SaveDirOk,1
		RS_BYTE	LNr_AutomaticSave,1
		RS_BYTE	LNr_SSDir,1
		RS_BYTE	LNr_SafeSave,1
		RS_BYTE	LNr_OverWrite,1
		RS_BYTE	LNr_SampleMode,1
		RS_BYTE	LNr_SaveAsProTracker,1
		RS_BYTE	LNr_Protect,1
		RS_BYTE	LNr_PBits,1
		RS_BYTE	LNr_NoNewModule,1
		RS_BYTE	LNr_LoadPrev,1
		RS_BYTE	LNr_LoadNext,1
		RS_BYTE	LNr_RandomModule,1
		RS_BYTE	LNr_RandomSong,1
		RS_BYTE	LNr_AutoSubSong,1
		RS_BYTE	LNr_LoadAlways,1
		RS_BYTE	LNr_Dirjump,1
		RS_BYTE	LNr_Songend,1
		RS_BYTE	LNr_LoadBefore,1
		RS_BYTE	LNr_CalcDuration,1
		RS_BYTE	LNr_RandomStart,1
		RS_BYTE	LNr_TimeMode,1
		RS_BYTE	LNr_CiaTiming,1
		RS_BYTE	LNr_VBlankTiming,1
		RS_BYTE	LNr_TimerDevice,1
		RS_BYTE	LNr_AutomaticWait,1
		RS_BYTE	LNr_RasterWait,1
		RS_BYTE	LNr_DBFWait,1
		RS_BYTE	LNr_AllocChannels,1
		RS_BYTE	LNr_SoftInt,1
		RS_BYTE	LNr_AutoPassword,1
		RS_BYTE	LNr_XFDDecrunch,1
		RS_BYTE	LNr_XFDLoadSeg,1
		RS_BYTE	LNr_SmallModule,1
		RS_BYTE	LNr_BigModule,1
		RS_BYTE	LNr_ModuleFault,1
		RS_BYTE	LNr_ReqLib,1
		RS_BYTE	LNr_ReqToolsLib,1
		RS_BYTE	LNr_AslLib,1
		RS_BYTE	LNr_AppIcon,1
		RS_BYTE	LNr_Appitem,1
		RS_BYTE	LNr_Volume,1
		RS_BYTE	LNr_Balance,1
		RS_BYTE	LNr_Speed,1
		RS_BYTE	LNr_ModuleLocked,1
		RS_BYTE	LNr_ModuleUnlocked,1
		RS_BYTE	LNr_NoGui,1
		RS_BYTE	LNr_ExtractFirstModule,1
		RS_BYTE	LNr_PleaseEnter,1
		RS_BYTE	LNr_SG_Publicscreen,1
		RS_BYTE	LNr_Publicscreen,1
		RS_BYTE	LNr_SG_PlayTime,1
		RS_BYTE	LNr_PlaytimeNow,1
		RS_BYTE	LNr_SG_MinTimeOut,1
		RS_BYTE	LNr_MinTimeOut,1
		RS_BYTE	LNr_SG_RasterLines,1
		RS_BYTE	LNr_RasterLines,1
		RS_BYTE	LNr_SG_DBFLoops,1
		RS_BYTE	LNr_DBFLoops,1
		RS_BYTE	LNr_SG_Password,1
		RS_BYTE	LNr_Password,1
		RS_BYTE	LNr_SG_XPKCrunchMode,1
		RS_BYTE	LNr_XPKCrunchMode,1

		RS_BYTE	LNr_PM_ProjectTitle,1
		RS_BYTE	LNr_PM_LoadModule,1
		RS_BYTE	LNr_PM_SaveModule,1
		RS_BYTE	LNr_PM_SaveModulePrefs,1
		RS_BYTE	LNr_PM_AboutModule,1
		RS_BYTE	LNr_PM_LoadPlayer,1
		RS_BYTE	LNr_PM_LoadConfig,1
		RS_BYTE	LNr_PM_SaveConfig,1
		RS_BYTE	LNr_PM_DeleteFile,1
		RS_BYTE	LNr_PM_Help,1
		RS_BYTE	LNr_PM_Documentation,1
		RS_BYTE	LNr_PM_Iconify,1
		RS_BYTE	LNr_PM_About,1
		RS_BYTE	LNr_PM_Hide,1
		RS_BYTE	LNr_PM_Quit,1

		RS_BYTE	LNr_PM_CommandTitle,1
		RS_BYTE	LNr_PM_PrevModule,1
		RS_BYTE	LNr_PM_PrevSong,1
		RS_BYTE	LNr_PM_PrevPattern,1
		RS_BYTE	LNr_PM_ReplaySong,1
		RS_BYTE	LNr_PM_Play,1
		RS_BYTE	LNr_PM_FastPlay,1
		RS_BYTE	LNr_PM_NextPattern,1
		RS_BYTE	LNr_PM_NextSong,1
		RS_BYTE	LNr_PM_NextModule,1
		RS_BYTE	LNr_PM_StopPlay,1
		RS_BYTE	LNr_PM_Eject,1

		RS_BYTE	LNr_PM_EngineTitle,1
		RS_BYTE	LNr_PM_LoadEngine,1
		RS_BYTE	LNr_PM_SpecialTitle,1
		RS_BYTE	LNr_PM_Preferences,1
		RS_BYTE	LNr_PM_ModulesMenu,1
		RS_BYTE	LNr_PM_SaveMode,1
		RS_BYTE	LNr_PM_Program,1
		RS_BYTE	LNr_PM_Timing,1
		RS_BYTE	LNr_PM_Warning,1
		RS_BYTE	LNr_PM_FileREQUester,1
		RS_BYTE	LNr_PM_IconifyMode,1
		RS_BYTE	LNr_PM_Layout,1
		RS_BYTE	LNr_PM_Cruncher,1
		RS_BYTE	LNr_PM_Voices,1
		RS_BYTE	LNr_PM_Volume,1
		RS_BYTE	LNr_PM_Balance,1
		RS_BYTE	LNr_PM_Speed,1
		RS_BYTE	LNr_PM_ToggleFilter,1
		RS_BYTE	LNr_PM_Publicscreen,1

		RS_BYTE	LNr_PM_SmallModule,1
		RS_BYTE	LNr_PM_BigModule,1
		RS_BYTE	LNr_PM_ModuleFault,1
		RS_BYTE	LNr_PM_ScrollInfos,1
		RS_BYTE	LNr_PM_FadeIn,1
		RS_BYTE	LNr_PM_FadeOut,1
		RS_BYTE	LNr_PM_Filter,1
		RS_BYTE	LNr_PM_MasterVolume,1
		RS_BYTE	LNr_PM_SongName,1
		RS_BYTE	LNr_PM_Prefix,1
		RS_BYTE	LNr_PM_SaveT,1
		RS_BYTE	LNr_PM_LoadFast,1
		RS_BYTE	LNr_PM_EjectPlayers,1
		RS_BYTE	LNr_PM_EjectEngines,1
		RS_BYTE	LNr_PM_PlayerBatch,1

		RS_BYTE	LNr_PM_RescanDir,1
		RS_BYTE	LNr_PM_LoadDir,1
		RS_BYTE	LNr_PM_EPDir,1
		RS_BYTE	LNr_PM_Notify,1
		RS_BYTE	LNr_PM_ScanAlways,1
		RS_BYTE	LNr_PM_HideAll,1

		RS_BYTE	LNr_PM_Uncrunched,1
		RS_BYTE	LNr_PM_PPCrunched,1
		RS_BYTE	LNr_PM_LHCrunched,1
		RS_BYTE	LNr_PM_XPKCrunched,1
		RS_BYTE	LNr_PM_CrMCrunched,1
		RS_BYTE	LNr_PM_AutomaticSave,1
		RS_BYTE	LNr_PM_Overwrite,1
		RS_BYTE	LNr_PM_SafeSave,1
		RS_BYTE	LNr_PM_SSDir,1
		RS_BYTE	LNr_PM_SampleMode,1
		RS_BYTE	LNr_PM_SaveAsProTracker,1
		RS_BYTE	LNr_PM_Protect,1
		RS_BYTE	LNr_PM_ProtectionBits,1
		RS_BYTE	LNr_PM_SaveDir,1
		RS_BYTE	LNr_PM_XPKCrunchmode,1
		RS_BYTE	LNr_PM_Password,1

		RS_BYTE	LNr_PM_NoNewModule,1
		RS_BYTE	LNr_PM_LoadPrev,1
		RS_BYTE	LNr_PM_LoadNext,1
		RS_BYTE	LNr_PM_RandomModule,1
		RS_BYTE	LNr_PM_Songend,1
		RS_BYTE	LNr_PM_LoadBefore,1
		RS_BYTE	LNr_PM_RandomSong,1
		RS_BYTE	LNr_PM_AutoSubSong,1
		RS_BYTE	LNr_PM_LoadAlways,1
		RS_BYTE	LNr_PM_Dirjump,1
		RS_BYTE	LNr_PM_TimeMode,1
		RS_BYTE	LNr_PM_CalcDuration,1
		RS_BYTE	LNr_PM_RandomStart,1
		RS_BYTE	LNr_PM_Quickstart,1
		RS_BYTE	LNr_PM_PlayTime,1
		RS_BYTE	LNr_PM_MinPlayTime,1


		RS_BYTE	LNr_PM_AutoPassword,1
		RS_BYTE	LNr_PM_ExtractFirstModule,1
		RS_BYTE	LNr_PM_XFDDecrunch,1
		RS_BYTE	LNr_PM_XFDLoadSeg,1
		RS_BYTE	LNr_PM_Req,1
		RS_BYTE	LNr_PM_ReqTools,1
		RS_BYTE	LNr_PM_Asl,1
		RS_BYTE	LNr_PM_AppIcon,1
		RS_BYTE	LNr_PM_AppItem,1
		RS_BYTE	LNr_PM_Menufont,1
		RS_BYTE	LNr_PM_Modulesfont,1
		RS_BYTE	LNr_PM_ScrollFont,1
		RS_BYTE	LNr_PM_CiaTiming,1
		RS_BYTE	LNr_PM_VBlankTiming,1
		RS_BYTE	LNr_PM_TimerDevice,1
		RS_BYTE	LNr_PM_AutomaticWait,1
		RS_BYTE	LNr_PM_RasterWait,1
		RS_BYTE	LNr_PM_DBFWait,1
		RS_BYTE	LNr_PM_RasterLines,1
		RS_BYTE	LNr_PM_DBFLoops,1
		RS_BYTE	LNr_PM_AllocChannels,1
		RS_BYTE	LNr_PM_SoftInt,1

		RS_BYTE	LNr_PM_Voice1,1
		RS_BYTE	LNr_PM_Voice2,1
		RS_BYTE	LNr_PM_Voice3,1
		RS_BYTE	LNr_PM_Voice4,1
		RS_BYTE	LNr_PM_ResetVoices,1
		RS_BYTE	LNr_PM_VolumeDefault,1
		RS_BYTE	LNr_PM_VolumeHigher,1
		RS_BYTE	LNr_PM_VolumeLower,1
		RS_BYTE	LNr_PM_BalanceDefault,1
		RS_BYTE	LNr_PM_BalanceLeft,1
		RS_BYTE	LNr_PM_BalanceRight,1
		RS_BYTE	LNr_PM_SpeedDefault,1
		RS_BYTE	LNr_PM_SpeedFaster,1
		RS_BYTE	LNr_PM_SpeedSlower,1
		RS_BYTE	LNr_PM_LastMenuItem,0

		RS_BYTE	LNr_ModuleWarning,1
		RS_BYTE	LNr_ModuleShorter,1
		RS_BYTE	LNr_ModuleHey,1
		RS_BYTE	LNr_ModuleLonger,1
		RS_BYTE	LNr_ModuleAs,1
		RS_BYTE	LNr_ThisIsA,1
		RS_BYTE	LNr_OriginalSize,1
		RS_BYTE	LNr_PackedSize,1
		RS_BYTE	LNr_BytesWon,1
		RS_BYTE	LNr_Module,1
		RS_BYTE	LNr_SongIsOver,1
		RS_BYTE	LNr_TimeIsOver,1
		RS_BYTE	LNr_LoadingList,1
		RS_BYTE	LNr_Scanning,1
		RS_BYTE	LNr_Parent,1
		RS_BYTE	LNr_Entries,1
		RS_BYTE	LNr_Entry,1
		RS_BYTE	LNr_ModulesTitle,1
		RS_BYTE	LNr_FT1,1
		RS_BYTE	LNr_FT2,1
		RS_BYTE	LNr_FT3,1
		RS_BYTE	LNr_FT4,1
		RS_BYTE	LNr_FT5,1
		RS_BYTE	LNr_FT6,1
		RS_BYTE	LNr_FT7,1
		RS_BYTE	LNr_FT8,1
		RS_BYTE	LNr_FT9,1
		RS_BYTE	LNr_FT10,1
		RS_BYTE	LNr_FT11,1
		RS_BYTE	LNr_FT12,1
		RS_BYTE	LNr_FT13,1
		RS_BYTE	LNr_FT14,1
		RS_BYTE	LNr_FT15,1
		RS_BYTE	LNr_FT16,1
		RS_BYTE	LNr_FT17,1
		RS_BYTE	LNr_FT18,1
		RS_BYTE	LNr_FT19,1
		RS_BYTE	LNr_FT20,1
		RS_BYTE	LNr_FT21,1
		RS_BYTE	LNr_FT22,1
		RS_BYTE	LNr_FT23,1
		RS_BYTE	LNr_FT24,1
		RS_BYTE	LNr_FT25,1
		RS_BYTE	LNr_FT26,1
		RS_BYTE	LNr_FT27,1
		RS_BYTE	LNr_FT28,1
		RS_BYTE	LNr_FT29,1
		RS_BYTE	LNr_FT30,1
		RS_BYTE	LNr_FT31,1
		RS_BYTE	LNr_FT32,1
		RS_BYTE	LNr_FT33,1
		RS_BYTE	LNr_FT34,1
		RS_BYTE	LNr_FT35,1
		RS_BYTE	LNr_FT36,1
		RS_BYTE	LNr_FT37,1
		RS_BYTE	LNr_FT38,1
		RS_BYTE	LNr_FT39,1
		RS_BYTE	LNr_FT40,1
		RS_BYTE	LNr_FT41,1
		RS_BYTE	LNr_FT42,1
		RS_BYTE	LNr_FT43,1
		RS_BYTE	LNr_FT44,1
		RS_BYTE	LNr_FT45,1
		RS_BYTE	LNr_FT46,1
		RS_BYTE	LNr_FT47,1
		RS_BYTE	LNr_FT48,1
		RS_BYTE	LNr_FT49,1
		RS_BYTE	LNr_FT50,1
		RS_BYTE	LNr_FT51,1
		RS_BYTE	LNr_FT52,1
		RS_BYTE	LNr_FT53,1
		RS_BYTE	LNr_FT54,1
		RS_BYTE	LNr_FT55,1
		RS_BYTE	LNr_FT56,1
		RS_BYTE	LNr_FT57,1
		RS_BYTE	LNr_FT58,1
		RS_BYTE	LNr_FT59,1
		RS_BYTE	LNr_FT60,1
		RS_BYTE	LNr_FT61,1
		RS_BYTE	LNr_FT62,1
		RS_BYTE	LNr_FT63,1
		RS_BYTE	LNr_FT64,1
		RS_BYTE	LNr_FT65,1
		RS_BYTE	LNr_FT66,1
		RS_BYTE	LNr_FT67,1
		RS_BYTE	LNr_FT68,1
		RS_BYTE	LNr_FT69,1
		RS_BYTE	LNr_FT70,1
		RS_BYTE	LNr_CreditsTitle,1
		RS_BYTE	LNr_DistributionTitle,1
		RS_BYTE	LNr_LookingTitle,1
		RS_BYTE	LNr_GlobalInfosTitle,1
		RS_BYTE	LNr_RegisterTitle,1
		RS_BYTE	LNr_AboutEagleplayer,1
		RS_BYTE	LNr_EagleplayerREQUest,1
		RS_BYTE	LNr_AboutGlobal,1
		RS_BYTE	LNr_AboutCredits,1
		RS_BYTE	LNr_AboutDistribution,1
		RS_BYTE	LNr_AboutLooking,1
		RS_BYTE	LNr_AboutRegister,1
		RS_BYTE	LNr_AboutRRegister,1
		RS_BYTE	LNr_TR_KeyHasAFault,1
		RS_BYTE	LNr_TR_KeyIsDisabled,1
		RS_BYTE	LNr_TR_Config,1
		RS_BYTE	LNr_TR_ToolTypes,1
		RS_BYTE	LNr_TR_Overwrite,1
		RS_BYTE	LNr_TR_SaveAsPT,1
		RS_BYTE	LNr_TR_Protection,1
		RS_BYTE	LNr_TR_SmallModule,1
		RS_BYTE	LNr_TR_ModuleFault,1
		RS_BYTE	LNr_TR_AudioAlloc,1
		RS_BYTE	LNr_TR_RippError,1
		RS_BYTE	LNr_TR_EmulatorError,1
		RS_BYTE	LNr_TR_IllegalFunction,1
		RS_BYTE	LNr_TR_ErrorLoadingInstruments,1
		RS_BYTE	LNr_EPBy,1
		RS_BYTE	LNr_GA_Ok,1
		RS_BYTE	LNr_GA_Yes,1
		RS_BYTE	LNr_GA_Config,1
		RS_BYTE	LNr_GA_RetrySkipCancel,1
		RS_BYTE	LNr_GA_SmallModule,1
		RS_BYTE	LNr_GA_Global,1
		RS_BYTE	LNr_GA_Credits,1
		RS_BYTE	LNr_GA_Distribution,1
		RS_BYTE	LNr_GA_Looking,1
		RS_BYTE	LNr_GA_Register,1
		RS_BYTE	LNr_AboutKey,1
		RS_BYTE	LNr_HideKey,1

		RS_BYTE	LNr_TR_RegisteredNormal,1
		RS_BYTE	LNr_TR_RegisteredSaveModule,1
		RS_BYTE	LNr_GA_Registered,1
		RS_BYTE	LNr_NervREQUest,1
		RS_BYTE	LNr_TR_RegisteredStart,1

		RS_BYTE	LNr_Fr_LoadList,1
		RS_BYTE	LNr_SW_LoadList,1
		RS_BYTE	LNr_Fr_SaveList,1
		RS_BYTE	LNr_SW_SaveList,1
		RS_BYTE	LNr_PM_LoadList,1
		RS_BYTE	LNr_PM_SaveList,1
		RS_BYTE	LNr_NewListloaded,1
		RS_BYTE	LNr_By,1
		RS_BYTE	LNr_ModulImFastMem,1
		RS_BYTE	LNr_ChipRAMAmplifier,1
		RS_BYTE	LNr_FastRAMAmplifier,1

		RS_BYTE	LNr_LastExternal,0


		*--- Nun die externen Enginestrings ---*

		RS_RESET
		RS_BYTE	LNr_E_Dummy,LNr_FirstEngine
		RS_BYTE	LNrE_SelectExoticPath,1
		RS_BYTE	LNrE_SelectingExoticPath,1
		RS_BYTE	LNrE_SelectExoticPathTitle,1
		RS_BYTE	LNrE_CantRunSecondExoticGui,1
		RS_BYTE	LNrE_CantQuitEagleexotic,1
		RS_BYTE	LNrE_CantQuitExoticripper,1
		RS_BYTE	LNrE_CantLoadExoticripper,1
		RS_BYTE	LNrE_ExoticKeinKey,1

		RS_BYTE	LNrE_SpecialInfo_8BitAmplifier,1
		RS_BYTE	LNrE_SpecialInfo_14BitAmplifier,1
		RS_BYTE	LNrE_SpecialInfo_AmplifierManager,1
		RS_BYTE	LNrE_SpecialInfo_BifatGUI,1
		RS_BYTE	LNrE_SpecialInfo_DirListViewer,1
		RS_BYTE	LNrE_SpecialInfo_Eagleexotic,1
		RS_BYTE	LNrE_SpecialInfo_Eagleplayer1GUI,1
		RS_BYTE	LNrE_SpecialInfo_Extractor,1
		RS_BYTE	LNrE_SpecialInfo_FFTAnalyzer,1
		RS_BYTE	LNrE_SpecialInfo_Levelgraph,1
		RS_BYTE	LNrE_SpecialInfo_Levelmeter,1
		RS_BYTE	LNrE_SpecialInfo_Manager,1
		RS_BYTE	LNrE_SpecialInfo_Messagewindow,1
		RS_BYTE	LNrE_SpecialInfo_Moduleinfo,1
		RS_BYTE	LNrE_SpecialInfo_Patternscroll,1
		RS_BYTE	LNrE_SpecialInfo_Playerloader,1
		RS_BYTE	LNrE_SpecialInfo_PublicscreenSelector,1
		RS_BYTE	LNrE_SpecialInfo_Quadrascope,1
		RS_BYTE	LNrE_SpecialInfo_SpaceScope,1
		RS_BYTE	LNrE_SpecialInfo_Stereoscope,1
		RS_BYTE	LNrE_SpecialInfo_Time,1


		RS_BYTE	LNrE_RefreshList,1		/* früher RefreshList*/
		RS_BYTE	LNrE_RefreshListKey,1		/* früher RefreshList*/
		RS_BYTE	LNrE_DLV_EasyTitle,1
		RS_BYTE	LNrE_DLV_About,1

		RS_BYTE	LNrE_PSS_EasyTitle,1
		RS_BYTE	LNrE_PSS_About,1
		RS_BYTE	LNrE_ChooseBackPic,1
		RS_BYTE	LNrE_BackPic,1

		RS_BYTE	LNrE_SpecialInfo_Pysion,1
		RS_BYTE	LNrE_SpecialInfo_SampleSaver,1
		RS_BYTE	LNrE_SpecialInfo_Noiseconverter,1
		RS_BYTE	LNrE_SpecialInfo_Formatloader,1

		RS_BYTE	LNrE_Exoticerror,1

		RS_BYTE	LNrE_SpecialInfo_Eagleripper,1
		RS_BYTE	LNrE_Pass1,1
		RS_BYTE	LNrE_Pass2,1
		RS_BYTE	LNrE_Pass3,1
		RS_BYTE	LNrE_SecurityMode,1
		RS_BYTE	LNrE_Ripping,1

		RS_BYTE	LNrE_Security1,1
		RS_BYTE	LNrE_Security2,1
		RS_BYTE	LNrE_Unknown,1
		RS_BYTE	LNrE_ER_SecurityGadgets,1

*LNrE_DefectSoftworks		RS_BYTE	1
*LNrE_RescanDirKey		RS_BYTE	1
*LNrE_AboutKey			RS_BYTE	1
*LNrE_HideKey			RS_BYTE	1
*LNrE_QuitKey			RS_BYTE	1


		RS_BYTE	LNrE_LastEngine,0

		*--- Nun die externen Playerstrings ---*
		RS_RESET
		RS_BYTE	LNrP_Dummy,LNr_FirstPlayer
		RS_BYTE	LNrP_AdaptedByDefect,1
		RS_BYTE LNrP_PleaseSelectYourInstrumentsDir,1
		RS_BYTE	LNrP_LastPlayer,0
#endif


/*-----------------------------------------------------------------------------*/

struct UPrgS_EngineInternal {
	struct	UPrgS_EngineInternal* UPrgS_NextUPrg;	/*Next Userprogramm*/

	/*------------ Userprogramm-Item-Struktur ------------*/
	APTR    UPrgS_NextItem;		/* next */
	UWORD   UPrgS_Left;   /*SpecialItemBreite-30	;Left Edge*/
	UWORD	UPrgS_Verti;  /*PrEngineVerti1	;Verti Pos.*/
	UWORD	UPrgS_Width;  /*UserItemBreite	;Width-Size (Negationbreite)*/
	UWORD	UPrgS_Height; /*10			;Hight-Size*/
	UWORD	UPrgS_Flags;  /*$4b*/
	ULONG	UPrgS_Dummy1;	/*Mutalexclude*/
	APTR	UPrgS_IAdr;	/*ItemFill*/
	ULONG	UPrgS_Dummy2;	/*SelectFill*/
	ULONG	UPrgS_Dummy3;	/*Command/KludgeFill*/
	APTR	UPrgS_SubItem;	/*SubItem*/
	UWORD	UPrgS_Dummy4;	/*NextSelect*/
	ULONG	UPrgS_Dummy5;	/*ab Kick2.0+*/
	APTR	UPrgS_IText;	/*dc.w	3,0,20,1*/
	APTR	UPrgS_Font;
	APTR	UPrgS_Name;     /*Menuname des Engines*/
	APTR	UPrgS_Wackel;
	UWORD	UPrgS_EPNr;	/*AktivationsNr =EPNr_Engine*/

	struct  Node UPrgS_Node; /*,LN_SIZE */

	/*----------- Zusätzliche Vereinbarungen -------------*/
	struct UM_Message UPrgS_Message; 	/*Message-Struktur+Sicherheit*/
	APTR	UPrgS_EUSAdr;			/*EUS_Adresse*/
	ULONG	UPrgS_EngineNr;			/*EngineNr*/
	UWORD	UPrgS_WinX;			/*WinX*/
	UWORD	UPrgS_WinY;			/*WinY*/
	UWORD	UPrgS_EUSFlags;		/*Flags*/
	ULONG	UPrgS_Special1;		/*Special1*/
	ULONG	UPrgS_Special2;		/*Special2*/
	ULONG	UPrgS_Special3;		/*Special3*/
	UWORD	UPrgS_CFlags;			/*Welche Zellen sind belegt*/
	ULONG	UPrgS_UFlags;			/*Flags (1=Loaded)*/
	ULONG	UPrgS_Size;
	/* RS_BYTE UPrgS_NamePuffer,0 */
};

#define UPrgF_Engine		 	0
#define UPrgF_BatchName		 	1
#define UPrgF_Debugger		 	2
#define UPrgF_TagUser		 	3
#define UPrgF_Active		 	4
#define UPrgF_Disabled		 	5
#define UPrgF_Gui		 	6
#define UPrgF_Show		 	7

#define UPrgB_Engine		 	(1<<UPrgF_Engine)
#define UPrgB_BatchName		 	(1<<UPrgF_BatchName)
#define UPrgB_Debugger		 	(1<<UPrgF_Debugger)
#define UPrgB_TagUser		 	(1<<UPrgF_TagUser)
#define UPrgB_Active		 	(1<<UPrgF_Active)
#define UPrgB_Disabled		 	(1<<UPrgF_Disabled)
#define UPrgB_Gui		 	(1<<UPrgF_Gui)
#define UPrgB_Show		 	(1<<UPrgF_Show)

#define MaxFilenameSize		 	108
#define MaxDirectorynameSize	 	500

/*--------- Struktur für einen Eintrag im Eagleplayer-FilePuffer --------------*
 * wird vom Eagleplayer, Dirlistviewer, Pysion benutzt.
 *-----------------------------------------------------------------------------*/
struct EPFile {
	APTR	EFP_NextAdr;			/*MenuItem*/
	APTR	EFP_MI_NextItem;
	UWORD	EFP_MI_LeftEdge;
	UWORD	EFP_MI_TopEdge;
	UWORD	EFP_MI_Width;
	UWORD	EFP_MI_Height;
	UWORD	EFP_MI_Flags;
	APTR	EFP_MI_MutualExclude;
	APTR	EFP_MI_ItemFill;
	APTR	EFP_MI_SelectFill;
	UBYTE	EFP_MI_Command;
	UBYTE	EFP_MI_KludgeFill00;
	APTR	EFP_MI_SubItem;
	UWORD	EFP_MI_NextSelect;
	APTR	EFP_Dummy1;				/*ab Kick2.0+*/
	UBYTE	EFP_IT_FrontPen;			/*I-TextStrukture*/
	UBYTE	EFP_IT_BackPen;
	UBYTE	EFP_IT_DrawMode;
	UBYTE	EFP_IT_KludgeFill00;
	UWORD	EFP_IT_LeftEdge;
	UWORD	EFP_IT_TopEdge;
	APTR	EFP_IT_ITextFont;
	APTR	EFP_IT_IText;
	APTR	EFP_IT_NextText;
	UWORD	EFP_ModuleNr;
	struct Node EFP_Node;			/* [LN_SIZE]; */
	ULONG	EFP_StructSize;
	APTR	EFP_PathArrayPtr;		/*kompletter Pfad (für Listen)*/
	APTR	EFP_DirArrayPtr;
	APTR	EFP_FileArrayPtr;
	APTR	EFP_NameArrayPtr;		/*without Präfix*/
	APTR	EFP_NameDirLock;
	UWORD	EFP_ArchiveEngineNr;		/*Nummer des Extractors*/
	UBYTE	EFP_Flags;			/*Achtung,wegen LHA kann*/
	UBYTE	EFP_NameRKennung;		/*EFP_NamePuffer länger sein !!!*/
	UBYTE	EFP_NamePuffer[MaxFilenameSize+2];	/*komplette Size in EFP_StructSize*/
	/* UBYTE	EFP_NamePufferend,0 */
};

#define EFPF_FileName		 	0
#define EFPF_DirName		 	1
#define EFPF_Parent		 	2
#define EFPF_ArchiveDir		 	3
#define EFPF_ArchiveFile	 	4
#define EFPF_ParentList		 	5
#define EFPF_All		 	-1

#define EFPB_FileName		 	(1<<EFPF_FileName)
#define EFPB_DirName		 	(1<<EFPF_DirName)
#define EFPB_Parent		 	(1<<EFPF_Parent)
#define EFPB_ArchiveDir		 	(1<<EFPF_ArchiveDir)
#define EFPB_ArchiveFile	 	(1<<EFPF_ArchiveFile)
#define EFPB_ParentList		 	(1<<EFPF_ParentList)


/*-------------------------------Audio Struct---------------------------------*/
struct AudioSource {
	APTR	AS_CurrentAdr;	/*wenn AS_CurrentAdr gesetzt wird, muß*/
	ULONG	AS_CurrentPos;	/*AS_Currentpos auf 0 zurückgesetzt werden !!!*/
				/*(Currentpos = aktuelles Offset von Currentadr aus gesehen,*/
				/*wird vom Amplifier erhöht)*/
	ULONG	AS_CurrentFPos; /*genauso AS_CurrentFPos (Fraction Position),*/
				/*muss auch 0 gesetzt werden bei neuem Sample vom Replay,*/
				/*Inhalt undefiniert.*/
					
	ULONG	AS_SampleSize;	/*Länge in Bytes*/
	UWORD	AS_Period;	/*Periodenwert, kompatibel zu Amiga-Standardwerten*/
	APTR	AS_RepeatAdr;	/*Repeatadresse (wenn kein Repeat, AS_Loopflag=0 setzen !!)*/
	APTR	AS_RepeatSize;	/*Repeatlänge in Bytes*/
	UWORD	AS_LeftVolume;	/*Lautstärke 0..64   -> wenn kein Panning, BEIDE !*/
	UWORD	AS_RightVolume; /*Lautstärke 0..64      gleich setzen !!!!!!!!!!*/
	UWORD	AS_DMABit;	/* 0 -> Kanal aus, sonst an*/
	UWORD	AS_LeftRight;	/* 0 = gor nix auf diesem Kanal*/
				/* 1 = Kanal nur links*/
				/*-1 = Kanal nur rechts*/
				/* 2 = Rechts & Links (Panning über AS_LeftVolume & -RightVolume)*/
	UBYTE	AS_LoopFlag;	/*see Definititions below*/
	UBYTE	AS_NoLoop;	/*rücksetzen auf 0 durch Player, wenn neue  Note gespielt wird!!*/
	ULONG	AS_Changeflags; /*Flags für geänderte Variablen*/
	ULONG	AS_SampleFlags; /*Typ des Samples usw. (unsupported yet)*/

	APTR	AS_Int;		/*Softint Struktur, wird aufgerufen, wenn auf*/
				/*diesem Kanal ein neues Sample angespielt wird,*/
				/*genau wie beim "Paula"-Chip*/

	ULONG	AS_AmplifierPrivate1;	/*können beliebig durch den Amplifier genutzt*/
	ULONG	AS_AmplifierPrivate2;	/*werden, dürfen nach der Übergabe der Struktur an den*/
	ULONG	AS_AmplifierPrivate3;	/*Amplifier keinesfalls durch das Replay verändert werden,*/
	ULONG	AS_AmplifierPrivate4;	/*am besten mit 0 initialisieren, dann an den Amplifier*/
	ULONG	AS_AmplifierPrivate5;	/*übergeben und nicht weiter drum kümmern*/
	ULONG	AS_AmplifierPrivate6;	/**/
	ULONG	AS_AmplifierPrivate7;	/**/
	ULONG	AS_AmplifierPrivate8;	/**/

	ULONG	AS_Reserved1;
	ULONG	AS_Reserved2;
	ULONG	AS_Reserved3;
	ULONG	AS_Reserved4;
	ULONG	AS_Reserved5;
};

/*----------------------------- Flags for Looping ------------------------------*/
#define ASLoop_None		 	0		/*no Loop (whole Byte = 0)*/
#define ASLoopF_Forwards	 	0		/*1 = Loop nur vorwärts*/
#define ASLoopF_Backwards	 	1		/*2 = Loop nur rückwärts (not supported yet)*/
#define ASLoopF_PingPong	 	2		/*3 = Ping-Pong Loop*/

#define ASLoopB_Forwards	 	(1<<ASLoopF_Forwards)
#define ASLoopB_Backwards	 	(1<<ASLoopF_Backwards)
#define ASLoopB_PingPong	 	(1<<ASLoopF_PingPong)

/*----------------------------- ChangeFlags -----------------------------------*/

#define ASChF_Adr		 	0
#define ASChF_Len		 	1
#define ASChF_RepAdr		 	2
#define ASChF_RepLen		 	3
#define ASChF_Per		 	4
#define ASChF_Vol		 	5
#define ASChF_DMA		 	6
#define ASChF_SamFlags		 	7

#define ASChB_Adr		 	(1<<ASChF_Adr)
#define ASChB_Len		 	(1<<ASChF_Len)
#define ASChB_RepAdr		 	(1<<ASChF_RepAdr)
#define ASChB_RepLen		 	(1<<ASChF_RepLen)
#define ASChB_Per		 	(1<<ASChF_Per)
#define ASChB_Vol		 	(1<<ASChF_Vol)
#define ASChB_DMA		 	(1<<ASChF_DMA)
#define ASChB_SamFlags		 	(1<<ASChF_SamFlags)


/*---------------------- Tagliste für Amplifierstrukturen/Infos ---------------*/
#define	EPAMT_TagBase	TAG_USER+16717		/*TAG_USER+"AM"*/

#define EPAMT_NumStructs	(EPAMT_TagBase+0)	/*Number of supplied Audio*/
							/*structures*/
#define EPAMT_AudioStructs	(EPAMT_TagBase+1) 	/*Address of the first Audio*/
							/*structure*/
#define EPAMT_MinEPVersion	(EPAMT_TagBase+2) 	/*Minimum EP Version needed*/

#define EPAMT_Flags		(EPAMT_TagBase+3) 	/*Flags (see below)*/

/*--------------------------- Flags für Amplifier ----------------------------*/

#define EPAMF_Direct		 	0 /*for Chipram/Fastram Amplifier, Replayer supports "ENPP_PokeXXX" */
#define EPAMF_8Bit		 	1 /*8 Bit SampleData, signed */
#define EPAMF_8BitUnsigned	 	2 /*8 Bit SampleData, unsigned */
#define EPAMF_16Bit		 	3 /*16 Bit SampleData, signed */
#define EPAMF_ChipRam		 	4 /*Samples located in ChipRam */
#define EPAMF_PingPongLoops	 	5 /*Replayer supports Ping-Pong-Loops (e.g. FT-II) */
#define EPAMF_WaitForStruct	 	6 /*Amplifier has to wait for a complete AS-Structure */
#define EPAMF_AudioInts		 	7 /*Replayer needs Audio Interrupt support */
#define EPAMF_DirectInts	 	8 /* Replayer Audio Interrupt should be called directly instead of signaling for after mixing (per channel in AS_SampleFlags) */


#define EPAMB_Direct		 	(1<<EPAMF_Direct)
#define EPAMB_8Bit		 	(1<<EPAMF_8Bit)
#define EPAMB_8BitUnsigned	 	(1<<EPAMF_8BitUnsigned)
#define EPAMB_16Bit		 	(1<<EPAMF_16Bit)
#define EPAMB_ChipRam		 	(1<<EPAMF_ChipRam)
#define EPAMB_PingPongLoops	 	(1<<EPAMF_PingPongLoops)
#define EPAMB_WaitForStruct	 	(1<<EPAMF_WaitForStruct)
#define EPAMB_AudioInts		 	(1<<EPAMF_AudioInts)
#define EPAMB_DirectInts		(1<<EPAMF_DirectInts)

/*---------------------- Playerstruct (for Playerloader) ----------------------*/
struct EPLPlayer {
	struct EPLPlayer *EPPl_Next;
	struct Node EPPl_Node;
	ULONG	EPPl_Size;	/* StructSize */
	APTR	EPPl_Segment;
	APTR	EPPl_TagList;
	APTR	EPPl_ExtraMem;
	ULONG	EPPl_ExtraMemSize;
	APTR	EPPl_Taskadr;
	APTR	EPPl_MsgPort;
	UWORD	EPPl_Flags;
};
/*	RS_BYTE	EPPl_PlayerName,0 */
/*	RS_BYTE	EPPl_Sizeof,0 */

#define	EPPLF_Enabled	0
#define	EPPLB_Enabled	(1<<EPPLF_Enabled)


#endif /* _INC_EAGLEPLAYERENGINE_H */

#if 0
*******************************************************************************
* Buggs:
*
* Jan:
*	Menus
*	Userprograms
*
* Amplifiersystem:
* -----------------
*
* Konfiguration der Amplifiers
*	EPConfig		= Win...,[Amplifier_Priorität.w,Flags.w] (EUS_Special.l)
*	eigene Config	= ???
*
*EUS_AMPriority	EQU	EUS_Special	/*Priorität des Amplifiers*/
*					/*(zum Einsortieren)*/
*EUS_AMFlags	EQU	EUS_Special+2	/*Flags des Amplifers (NoWin)*/
*
*--------------------------------- EUS-Flags ------------------------------*
*EUSF_Disable	EQU	0	/*Amplifier Disabled ?*/
*				/*Bit set = yes*/
*EUSB_Disable	EQU	1<<EUSF_Disable
*
*
* 2 interne User ChipRAMPlayer,FastRAMPlayer (4 Voices), nur in Liste der
* Amplifier, werden in Userprogrammliste nicht mit aufgenommen, da hier
* keine Einstellungen wie Mixrate usw. vorgenommen werden müssen, einziges
* Problem sind die Priorität und Enable/Disable, welche ja mit in der Konfig
* gespeichert werden sollen, mal sehen was sich da machen läßt
*
*
* Beim Entfernen der Engines Amplifierliste überarbeiten und gegebenenfalls
* das aktuelle Modul rauswerfen, falls der aktuelle Amplifier davon betroffen
* ist, selbiges gilt für das Laden/Einfügen von Userprogrammen
*
*	EUT_AmplifierManager <- An diesen bei jeder Änderung der Amplifierliste
*				eine Message "USClass_NewAmplifierlist" schicken
*
*
*
* jeder Amplifier wird höchstens einmal geladen
*		(EUS_AMIDNr		/* wird von uns bestimmt)*/
* eigene Liste in
*
*
*			EPG_AmplifierList
*			EPG_ActiveAmplifier /*EUS_StartAdr*/
*			EPG_AudioStruct	/*aktuelle AudioStruct (EP privat)*/
*			EPG_AmplifierTagList
*
*			LONG	EPAMT_NumStructs
*			APTR	EPAMT_AudioStructs
*			LONG	EPAMT_MinEPVersion
*			LONG	EPAMT_Flags
*				EPAMF_Direct
*				EPAMF_8Bit
*				EPAMF_8BitUnsigned
*				EPAMF_16Bit
*				EPAMF_ChipRam		/*Samples im Chip*/
*
*
*
*	EUS_Type = EUT_Amplifier		/*Userprogramtype (0=unknown)*/
*
* ENPP_PokeAdr
* ENPP_PokeLen
* ENPP_PokePer
* ENPP_PokeVol
* ENPP_DMABit
*
*	APTR	EUS_SpecialJumpTab	/*private Jumptab (e.g Amplifier)*/
*			AMJ_Init	/*Übergabe der Struktur/test&Init*/
*			AMJ_StartInt	/*Alloc Audio & Start Int*/
*			AMJ_StopInt	/*Free Audio & Stop Int*/
*			AMJ_End		/*Mem freigeben, etc.*/
*
*			AMJ_PokeAdr	/*nur für Amplifier initialisieren,*/
*			AMJ_PokeLen	/*die auf die Hardware poken, wie*/
*			AMJ_PokePer	/*z.B. der Chipram Player, sonst*/
*			AMJ_PokeVol	/*auf 0 setzen, diese Jumps werden mit*/
*			AMJ_DMABit	/*den selben Parametern aufgerufen, wie*/
*					/*die ENPP_Poke...*/
*
* AudioStructure: siehe oben
*
*
*
*
*EUF_NoPDMenu	\ können die nicht eigentlich raus ?
*EUF_PDDisabled	/
*
*
*-> UCM_KillEngine -> oder gibt`s das schon ?
*-> UCM_Engine	-> bei "Hide" darf das UPS auch bei "Eject Engine"
*			nicht rausgeworfen werden, "Eject" ständig ein/aus-
*			zuschalten würde wegen des Menüs zu lange dauern
#endif
