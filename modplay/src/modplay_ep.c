/* **************************************************************************


   (C) 2024 Henryk Richter

   AGUS_Mod eagleplayer plugin

   Notes:
   Eagleplayer supplies the EPBase in register A5 when calling plugins. Since
   this register is not supported by m68k-amigaos-gcc (GCC) 6.5.0b 221101201233
   to be used as asm register argument, a workaround is implemented below.
   The relevant initial calls like CheckFunc() and ConfigFunc() are augmented
   by trampolines that move a5 into a1 before entering the C function(s).

   ************************************************************************** */
#include "compiler.h"
#include "fileio.h"
#include "modplay.h"
#include "modrender_common.h"
#include "amigus_registers.h"
#include <misc/EaglePlayer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>
#include <libraries/configvars.h>


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
extern ULONG CheckFunc[3];       /* trampoline to get/store EPBase A5->A1  */
extern ULONG ConfigFunc[3];      /* trampoline to get/store EPBase A5->A1  */
extern ULONG Pattscroll_Init[4]; /* trampoline to return result in D0->A0  */

extern ULONG InitPlayerFunc[3];
extern ULONG EndPlayerFunc[3];

ULONG InitSoundFunc(void);
ULONG EndSoundFunc(void);
ULONG StartIntFunc(void);
ULONG StopIntFunc(void);
ULONG VolumeFunc(void);
ULONG _Pattscroll_Init(void);
ULONG Fill_NewModuleInfo(void);
ULONG GetSongPosition(void);

extern const UBYTE name_str[];
extern const UBYTE creator_str[];
extern struct TagItem ModuleInfoTags[20];

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
	{EP_NewModuleInfo,(ULONG)ModuleInfoTags},
	{EP_GetPositionNr,(ULONG)GetSongPosition},

	/* */
	/* TODO: scope stuff, moduleinfo, fwd/back, patternscroll */
	{EP_Flags,EPB_Packable|EPB_Restart|EPB_Songend|EPB_Volume|EPB_Balance},
	{TAG_DONE} 
};


//DTP_NextPatt
//DTP_PrevPatt
//EP_GetPositionNr,Pro_GetPosNr
//EP_Get_ModuleInfo,GetInfos
//EP_NewModuleInfo,MI_TagList

//DTP_RequestDTVersion
//DTP_Interrupt
//EP_InitAmplifier,InitAudstruct
//EP_SampleInit,Pro_SampleInit
//EP_SampleEnd,SampleEnd
//dc.l    EP_flags,EPB_Volvoices!EPB_Packable!EPB_Save!EPB_restart!EPB_songend!EPB_Volume!EPB_Balance!EPB_Voices!EPB_Analyzer!EPB_Moduleinfo!EPB_Prevpatt!EPB_Nextpatt!EPB_Calcduration
//EP_Voices,SetVoices

/* local variables */
struct ExecBase *SysBase = (0);
struct DosLibrary *DOSBase = (0);
struct EaglePlayerGlobals *EPBase = (0);
struct ConfigDev *AmiGUS_Board = (0);
struct {
   struct EP_Patterninfo pi;
   APTR                  stripes[32];
} patinfo_ext;

struct TagItem ModuleInfoTags[20] = {
  {TAG_DONE, 0 },
};

LONG   mod_songsz = 0;
struct MOD *mod = (0);
struct MODRender *mrnd = (0);
#define srate 44100




/* strings */
const UBYTE name_str[] = "AGUS_Mod";
const UBYTE ver_str[] = "$VER: AGUS_Mod 0.8 (18.7.2025) by Henryk Richter";
const UBYTE creator_str[] = "Protracker + similar and S3M player\nfor AmiGUS (C) Henryk Richter";
struct {
	ULONG  id;
	STRPTR name;
} id2namestr[] = {
	{ MOD_ID_M_K_, (STRPTR)"Protracker" },
	{ MOD_ID_MIKI, (STRPTR)"Ext. Protracker" },
	{ MOD_ID_FLT4, (STRPTR)"StarTrekker" },
	{ MOD_ID_FLT8, (STRPTR)"StarTrekker8" },
	{ MOD_ID_8CHN, (STRPTR)"FastTracker" },
	{ MOD_ID_S3M,  (STRPTR)"ScreamTracker III" },
	{ 0, (STRPTR)"FastTracker" } /* End Marker */
};



ASM LONG _CheckFunc( ASMR(a1) struct EaglePlayerGlobals *__EPBase ASMREG(a1) )
{ 
 struct EaglePlayerGlobals *EPBase = __EPBase; /* use EPBase from trampoline (in this example) */

	/* better be safe: don't accept anything until we're configured */
	if( !AmiGUS_Board )
		return -1;
 
	mod_songsz = mod_check( EPBase->dt.ChkData, EPBase->dt.ChkSize );
	if( mod_songsz > 0 )	/* supported file detected */
		return 0;

	return -1;
}
ULONG CheckFunc[3] = { 0x224D4EB9,(ULONG)_CheckFunc,0x4A804E75}; /* move.l a5,a1; jsr _Func; tst.l d0;rts */



ASM LONG _ConfigFunc( ASMR(a1) struct EaglePlayerGlobals *_EPBase ASMREG(a1) )
{
	struct Library *ExpansionBase;

	if( DOSBase )
		return 0;

	SysBase = *( (struct ExecBase**)0x4 );
	EPBase  = _EPBase;

	DOSBase = (struct DosLibrary*)EPBase->dt.DOSBase;
	//DOSBase = (struct DosLibrary *)OpenLibrary( (STRPTR)"dos.library", 37 );

	/* check if we actually have an AmiGUS */
	ExpansionBase = OpenLibrary( (STRPTR)"expansion.library", 34 ); /* 1.3+ */
	if( ExpansionBase ) /* shouldn't fail */
	{
	   AmiGUS_Board = FindConfigDev( NULL, AMIGUS_MANUFACTURER_ID, AMIGUS_HAGEN_PRODUCT_ID );
	   CloseLibrary( ExpansionBase );
	}

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
	{
		/* could not get an instance but a driver is registered:
		 * can't be us */
		if( AmiGUS_Board->cd_Driver )
		{
		  void (*TextReqFunc)(void) = (void (*)(void))EPBase->EPG_TextRequest;
		  struct Node *drv = (struct Node*)AmiGUS_Board->cd_Driver;
		  ULONG args[4];

		  args[0] = (ULONG)drv->ln_Name;
		  /*
			*-------------------------------- TextRequest ------------------------------*
			*----   ARG1 = TextAdresse                                              ----*
			*----   ARG2 = Pointer to Pubscreenname (only Kick2.0)                  ----*
			*----   ARG3 = Position on Screen (x.w & y.w)                           ----*
			*----   ARG4 = Pointer to Gadgetnames                                   ----*
			*----   ARG5 = Pointer to Requestername                                 ----*
			*----   ARG6 = Pointer to ArgumentListe                                 ----*
			*----   ARG7 = Pointer to ImageDatas                                    ----*
			*----   ARG8 = Flags            EPTRF_Center...                         ----*
			*----                           EPTRF_TestTimeOut                       ----*
		  */
		  EPBase->EPG_ARG1 = (ULONG)"Error! Cannot allocate AmiGUS Wavetable Engine\nThe  Wavetable is in use by\n%s\nPlease quit the other program.";
		  EPBase->EPG_ARG2 = (ULONG)EPBase->EPG_PubScreen;
		  EPBase->EPG_ARG3 = 0;
		  EPBase->EPG_ARG4 = (ULONG)"OK";
		  EPBase->EPG_ARG5 = (ULONG)name_str;
		  EPBase->EPG_ARG6 = (ULONG)args;
		  EPBase->EPG_ARG7 = 0;
		  EPBase->EPG_ARG8 = 0;
		  TextReqFunc();
		}
        	break;
	}

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
	Fill_NewModuleInfo();

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
ULONG InitPlayerFunc[3] = { 0x224D4EB9,(ULONG)_InitPlayerFunc,0x4A804E75}; /* not strictly necessary here: we could use the stored EPBase */



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
ULONG EndPlayerFunc[3] = { 0x224D4EB9,(ULONG)_EndPlayerFunc,0x4A804E75}; /* not strictly necessary here: we could use the stored EPBase */

/* nothing done in InitSound right now (TODO: reset song/pattern position and speed/ciatiming) */
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


ULONG GetSongPosition(void)
{
	ULONG pos=0;

	if( mod )
		pos = mod->songpos;

	return pos;
}

#define _STAG_( _t_, _tag_, _data_ ) {_t_->ti_Tag = (_tag_);_t_->ti_Data = (ULONG)(_data_);_t_++;}
ULONG Fill_NewModuleInfo(void)
{
 struct TagItem *tgs = ModuleInfoTags;
 int i;

 /* multiple formats supported, pick right one */
 i=0;
 while( id2namestr[i].id != 0 )
 {
  if( mod->modtype == id2namestr[i].id )
	  break;
  i++;
 }
 _STAG_( tgs, MI_Soundsystem, id2namestr[i].name )

 /* mod name */
 if( mod->name )
	 _STAG_( tgs, MI_SongName, mod->name )

 _STAG_( tgs, MI_Voices,     mod->nchannels  )
 _STAG_( tgs, MI_Length,     mod->songlen    )
 _STAG_( tgs, MI_Samples,    mod->nsamples   )
 _STAG_( tgs, MI_MaxSamples, mod->maxsamples )
 _STAG_( tgs, MI_Pattern,    mod->npat       )

 if( mod->flags & MODPF_VBLANK )
 {
	_STAG_( tgs, MI_SpecialInfo, (ULONG)"VBlank Timing used." )
 
 }

 /* MI_AuthorName,MI_SongSize,MI_SamplesSize,MI_OtherSize,MI_Calcsize,MI_Duration */

 /* finalize */
 tgs->ti_Tag  = TAG_DONE;
 tgs->ti_Data = 0;

 return 1;
}


extern ASM ULONG Pattscroll_ConvertFunc( ASMR(a0) APTR note ASMREG(a0) );

/* callback whenever pattern position changes
 * needs to be interrupt-safe
 * */
#ifdef AMIGA
ASM void Pattscroll_Update(ASMR(a1) struct MOD *mod ASMREG(a1))
#else
void Pattscroll_Update( MOD *mod )
#endif
{
  struct EP_Patterninfo *pi = &patinfo_ext.pi;

  pi->PI_Pattpos    = mod->patpos;
  pi->PI_BPM        = mod->ciaspeed;
  pi->PI_Speed      = mod->speed;

  //if( pi->PI_Pattern == *(mod->song + mod->songpos) )
  if( pi->PI_Songpos == mod->songpos )
	  return;

  {
    struct MODPattern **patts,*patt;
    struct MODPatternEntry *pdta;
    int i,sp,patidx;

    patts  = mod->patterns;
    sp     = mod->songpos;
    patidx = *(mod->song + sp );

    pi->PI_Songpos = sp;
    pi->PI_Pattern = patidx;

    patt  = patts[ patidx ];
    pdta  = patt->patdata;
    for( i = 0 ; i < pi->PI_Voices ; i++ )
         patinfo_ext.stripes[i] = pdta++;
  }

}

/* requires return in A0, hence called by the trampoline below */
ULONG _Pattscroll_Init(void)
{
  struct EP_Patterninfo *pi = &patinfo_ext.pi;
  struct MODPattern **patts,*patt;
  struct MODPatternEntry *pdta;
  int i;

  if( !mod )
	 return NULL;

  pi->PI_NumPatts   = mod->npat;
  pi->PI_Pattlength = mod->patlen;
  pi->PI_Voices     = mod->nchannels;
  pi->PI_MaxSongPos = mod->songlen;
  pi->PI_Modulo     = sizeof(struct MODPatternEntry) * mod->nchannels;
  pi->PI_Convert    = (APTR)Pattscroll_ConvertFunc; /* Converts Note (a0) to Period (D0),Samplenumber (D1), Commandstring (D2) and Argument (D3) */

  mod_SetPatScrollCallback( mod, Pattscroll_Update );

  patts = mod->patterns;
  patt  = patts[ *(mod->song + mod->songpos) ];
  pdta  = patt->patdata;
  for( i = 0 ; i < pi->PI_Voices ; i++ )
         patinfo_ext.stripes[i] = pdta++;
 
  pi->PI_Pattern    = *(mod->song + mod->songpos);
  pi->PI_Pattpos    = mod->patpos;
  pi->PI_Songpos    = mod->songpos;
  pi->PI_BPM        = mod->ciaspeed;
  pi->PI_Speed      = mod->speed;
 
  return (ULONG)pi; /* requires a "move.l d0,a0" somewhere outside */
}
ULONG Pattscroll_Init[4] = { 0x4E714EB9,(ULONG)_Pattscroll_Init,0x20407202,0x4e714E75}; /* nop;jsr _...;move.l d0,a0;moveq #1,d1;nop;rts */


