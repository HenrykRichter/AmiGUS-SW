/* **************************************************************************

   (C) 2024 Henryk Richter

   AGUS_Mod eagleplayer plugin

   ************************************************************************** */
#include "compiler.h"
#include "fileio.h"
#include "modplay.h"
#include "modrender_common.h"
#include <misc/EaglePlayer.h>
#include <proto/exec.h>
#include <proto/dos.h>


#if 0
extern struct TagItem plugin_tags[];

struct	DeliTrackerPlayer ply = {
	0x70004e75,
	"DELIRIUM",
	plugin_tags
};
#endif

ASM void songend_cb( ASMR(a1) struct MOD *mod ASMREG(a1) );
//ASM LONG CheckFunc(  ASMR(a5) struct EaglePlayerGlobals *EPBase ASMREG(a5) );
//ASM LONG ConfigFunc( ASMR(a5) struct EaglePlayerGlobals *EPBase ASMREG(a5) );
//ASM LONG InitPlayerFunc( ASMR(a5) struct EaglePlayerGlobals *EPBase ASMREG(a5) );
//ASM LONG EndPlayerFunc( ASMR(a5) struct EaglePlayerGlobals *EPBase ASMREG(a5) );
//
extern ULONG CheckFunc[3];
extern ULONG ConfigFunc[3];
extern ULONG InitPlayerFunc[3];
extern ULONG EndPlayerFunc[3];
ULONG InitSoundFunc(void);
ULONG EndSoundFunc(void);
ULONG StartIntFunc(void);
ULONG StopIntFunc(void);
ULONG VolumeFunc(void);
ULONG Pattscroll_Init(void);
extern const UBYTE name_str[];
extern const UBYTE creator_str[];


struct TagItem plugin_tags[] = { 
	{EP_Date,(1<<24)|(7<<16)|2025},
	{DTP_PlayerName,(ULONG)name_str},
	{DTP_Creator,(ULONG)creator_str},
	/* */
	{DTP_Config,(ULONG)ConfigFunc},
	{DTP_Check1,(ULONG)CheckFunc},
	/* */
	{DTP_InitPlayer,(ULONG)InitPlayerFunc},
	{DTP_EndPlayer, (ULONG)EndPlayerFunc},
	/* */
	{DTP_InitSound,(ULONG)InitSoundFunc},
	{DTP_EndSound,(ULONG)EndSoundFunc},
	/* */
	{DTP_StartInt,(ULONG)StartIntFunc},
	{DTP_StopInt, (ULONG)StopIntFunc},
	/* */
	{DTP_Volume, (ULONG)VolumeFunc},
	{DTP_Balance,(ULONG)VolumeFunc},

	{EP_PatternInit,(ULONG)Pattscroll_Init},
	/* */
	/* TODO: scope stuff, moduleinfo, fwd/back, patternscroll */
	{EP_Flags,EPB_Packable|EPB_Restart|EPB_Songend|EPB_Volume|EPB_Balance},
	{TAG_DONE} 
};


//DTP_NextPatt
//DTP_PrevPatt
//EP_GetPositionNr,Pro_GetPosNr
//EP_Get_ModuleInfo,GetInfos


//DTP_RequestDTVersion
//DTP_Interrupt
//EP_InitAmplifier,InitAudstruct
//EP_SampleInit,Pro_SampleInit
//EP_SampleEnd,SampleEnd
//dc.l    EP_flags,EPB_Volvoices!EPB_Packable!EPB_Save!EPB_restart!EPB_songend!EPB_Volume!EPB_Balance!EPB_Voices!EPB_Analyzer!EPB_Moduleinfo!EPB_Prevpatt!EPB_Nextpatt!EPB_Calcduration
//EP_Voices,SetVoices

struct ExecBase *SysBase = (0);
struct DosLibrary *DOSBase = (0);
struct EaglePlayerGlobals *EPBase = (0);
struct {
   struct EP_Patterninfo pi;
   APTR                  stripes[32];
} patinfo_ext;

LONG   mod_songsz = 0;
struct MOD *mod = (0);
struct MODRender *mrnd = (0);
#define srate 44100


/* strings */
const UBYTE name_str[] = "AGUS_Mod";
const UBYTE creator_str[] = "Protracker + similar and S3M player\nfor AmiGUS (C) Henryk Richter";



ASM LONG _CheckFunc( ASMR(a1) struct EaglePlayerGlobals *__EPBase ASMREG(a1) )
{ 
 struct EaglePlayerGlobals *EPBase = __EPBase;

	/* better be safe: don't accept anything until we're configured */
	if( !DOSBase )
		return -1;
 
	mod_songsz = mod_check( EPBase->dt.ChkData, EPBase->dt.ChkSize );
	if( mod_songsz > 0 )	/* supported file detected */
		return 0;

	return -1;
}
ULONG CheckFunc[3] = { 0x224D4EB9,(ULONG)_CheckFunc,0x4A804E75}; /* move.l a5,a1; jsr _Func; tst.l d0;rts */



ASM LONG _ConfigFunc( ASMR(a1) struct EaglePlayerGlobals *_EPBase ASMREG(a1) )
{
	if( DOSBase )
		return 0;

	SysBase = *( (struct ExecBase**)0x4 );
	EPBase  = _EPBase;

	DOSBase = (struct DosLibrary*)EPBase->dt.DOSBase;
	//DOSBase = (struct DosLibrary *)OpenLibrary( (STRPTR)"dos.library", 37 );


	return 0;
}
ULONG ConfigFunc[3] = { 0x224D4EB9,(ULONG)_ConfigFunc,0x4A804E75}; /* move.l a5,a1 (work around gcc quirks) */



/* 
 * init song and load samples onto card
 *
 *
 */
ASM LONG _InitPlayerFunc( ASMR(a1) struct EaglePlayerGlobals *EPBase ASMREG(a1) )
{
 FH_T ifile = (0);
 LONG filesize,mod_sampleoff=0;
 LONG ret = -1;
 UBYTE *buf = (0);
 LONG nbytes,i,bufsz;
 ULONG foff,curoff;

 void (*CopyDirFunc)(void) = (void (*)(void))EPBase->dt.dtg_CopyDir;
 void (*CopyFileFunc)(void) = (void (*)(void))EPBase->dt.dtg_CopyFile;

 /* assemble path */
 CopyDirFunc(); /* implicitly clears PathArrayPtr string before copy */
 CopyFileFunc();

 do
 {
 	if( mod_songsz < 1048 )	/* supported file detected ? */
		break;

 	ifile = FILE_OPEN_R( EPBase->dt.PathArrayPtr );
	if( !ifile )
		break;

	FILE_SEEK_END( 0, ifile );
	filesize = FILE_SEEK_TELL( ifile );
	FILE_SEEK_START(0, ifile );

	if( filesize < mod_songsz )
		break;

	bufsz = mod_songsz;
	buf = (UBYTE*)MALLOC( mod_songsz );
	if( !buf )
		break;

	filesize = FILE_READ( buf, mod_songsz, ifile );
	if( filesize < mod_songsz )
		break;

	mod = mod_init( buf, mod_songsz, 0, &mod_sampleoff );
	if( mod_sampleoff <= 0 )
		break;
	
	if( mod_sampleoff != mod_songsz )
		FILE_SEEK_START(mod_sampleoff,ifile);
	curoff = mod_sampleoff;

	/*                                    */
	/* now prepare to load samples        */
	/*                                    */
	mrnd = modrender_init( mod, srate, 0 );
	if( !mrnd )
        	break;

	for( i = 0, foff = 0 ; i < mod->maxsamples ; i++ )
	{
		nbytes = mod_numbytes_of_sample( mod, i, &foff );
		if( nbytes > foff )
			foff = nbytes;
	}

	/*                                    */
	/* align buffer to largest sample     */
	/*                                    */
	if( foff > bufsz )
	{
		MFREE(buf);
		bufsz = foff;
		buf = MALLOC( bufsz );
		if( !buf )
			break;
	}

	ret = 0; /* we're probably good at this point */
	for( i = 0 ; i < mod->maxsamples ; i++ )
	{
		nbytes = mod_numbytes_of_sample( mod, i, &foff );
		if( nbytes )
		{
			if( curoff != foff )
				FILE_SEEK_START( foff, ifile );
			curoff  = foff;
			curoff += FILE_READ( buf, nbytes, ifile );

			if( 0 != modrender_loadsample( mrnd, i, buf, nbytes ) )
			{
				ret = -1;
				break;
			}
		}
	}

	mod_setendcallback( mod, songend_cb );

 } while(0);

 if( ifile )
 {
 	FILE_CLOSE( ifile );
 }

 if( buf )
 {
	 MFREE(buf);
 }

 if( (ret != 0) && (mrnd != NULL) )
 {
	modrender_free( mrnd );
	mrnd = NULL;
 }
 
 if( (ret != 0) && (mod != NULL) )
 {
	mod_free( mod );
	mod = NULL;
 }
 
 return ret;
}
ULONG InitPlayerFunc[3] = { 0x224D4EB9,(ULONG)_InitPlayerFunc,0x4A804E75};



ASM LONG _EndPlayerFunc( ASMR(a1) struct EaglePlayerGlobals *EPBase ASMREG(a1) )
{
	if( mrnd != NULL )
 	{
        	modrender_free( mrnd );
		mrnd = NULL;
	}

	if( mod != NULL)
 	{
        	mod_free( mod );
		mod = NULL;
 	}

	return 0;
}
ULONG EndPlayerFunc[3] = { 0x224D4EB9,(ULONG)_EndPlayerFunc,0x4A804E75};


ULONG InitSoundFunc() { return 0;}
ULONG EndSoundFunc()  { return 0;}

ULONG StartIntFunc()
{
	modrender_start_play( mrnd );
	VolumeFunc();

	return 0;
}


ULONG StopIntFunc()
{
	modrender_stop_play( mrnd );
	return 0;
}

#ifdef __SASC
void __regargs __chkabort(void)
{}
ASM void __regargs _XCEXIT(void)
{}
#endif

/*
 * callback for songend functionality
 */ 
ASM void songend_cb( ASMR(a1) struct MOD *mod ASMREG(a1) )
{
   void (*SongEndFunc)(void) = (void (*)(void))EPBase->dt.dtg_SongEnd;

   SongEndFunc();
}

/*
 *
 * */
ULONG VolumeFunc(void)
{
   LONG vol  = EPBase->dt.SndVol;  /* 0..64 */
   LONG lbal = EPBase->dt.SndLBal; /* 0..64 */
   LONG rbal = EPBase->dt.SndRBal; /* 0..64 */

   lbal = (lbal*vol)<<4; /* 0..4096 -> 0..65536 */
   rbal = (rbal*vol)<<4; /* 0..4096 -> 0..65536 */

   if( lbal > 0 ) lbal--; /* 0..65535 */
   if( rbal > 0 ) rbal--;

   modrender_setvolume( mrnd, lbal, rbal );

   return 0;
}

ULONG Pattscroll_Init(void)
{
	return NULL;
}


