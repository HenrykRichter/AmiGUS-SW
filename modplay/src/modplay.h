#ifndef _INC_MODPLAY_H_
#define _INC_MODPLAY_H_

/* **************************************************************************

   Somewhat generic tracker player, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   ************************************************************************** */

#include "types.h" /* redirects to exec/types.h on Amiga */
#include "compiler.h"

/* error codes */
#define MOD_ERR_GENERAL        -1 /* unspecified catastrophic error  */
#define MOD_ERR_WRONGFORMAT    -2 /* no mod detected in input buffer */
#define MOD_ERR_BUFFERTOOSMALL -3 /* supply more bytes, please. (best call: load whole module after initial try with less data) */
#define MOD_ERR_MEMORY         -4 /* out of mem */

/* operating modes (planned, not implemented) */
#define MOD_MODE_PT	1	/* Noisetracker / ProTracker */
#define MOD_MODE_PCPT   2	/* PT but extended period range (Fast,xCHN,...) */
#define MOD_MODE_OKT	3	/* Oktalyzer */
#define MOD_MODE_MED	4	/* MED, OcataMED */
#define MOD_MODE_FT2	5	/* FastTracker II XM */
#define MOD_MODE_S3M	6	/* ScreamTracker3 */
#define MOD_MODE_IMP	7	/* ImpulseTracker */


/* flags for the mod player */
#define MODPF_SONGEND     0x00000001 /* End of Song was detected */
#define MODPF_VBLANK      0x00000002 /* this module uses VBlank timing and 
                                        associated commands */
#define MODPF_REPEAT      0x00000004 /* don't detect song end and just play in 
                                        a loop */
#define MODPF_PATDELAY    0x00000008 /* pattern delay was active */

#define MODPF_NO_PT3QUIRKS 0x00000010 /* don't apply PT3 quirks (like first PORTUP 0x1xx or DOWN 0x2xx) */
#define MODPF_FIRST_PORT  0x00000020 /* no portamento yet (global mask in PT player) */
#define MODPF_PATSCROLL   0x00000040 /* patternscroll is active */

#define MODPERTAB_MAXTABLES 16 /* should be 2^n, see ptable_idxmask */


struct ModPeriodTab
{
	UWORD * ptables[MODPERTAB_MAXTABLES];	/* up to 16 finetuned tables (depending on source mod) */
	UWORD   ptable_type;    /* MODPERTAB_PERIOD or MODPERTAB_S3M (c2spd) */ 
	UWORD	ptable_idxmask; /* finetuning index mask, typically MODPERTAB_MAXTABLES-1 */
	UWORD	ptable_maxidx;  /* maximum index for offset clamping */
	UWORD	ptable_minper;	/* PT: 113 */
	UWORD	ptable_maxper;	/* PT: 856 (with finetuning, actual max would be 907 but PT clamps the former) */
};
#define MODPERTAB_PERIOD 0
#define MODPERTAB_S3M    1


struct MODSample
{
	/* mod parser / player specific */
	LONG length;   /* length in number of samples      */
	LONG repstart; /* repeat start of first sample     */
	LONG replen;   /* repeat length in samples         */
	LONG finetune; /* period finetune (or S3M C2SPD)   */
	WORD volume;   /* volume (0...64)                  */
	UBYTE type;    /* original sample type, MODSAM_TYPE_* */
	UBYTE unused_1;/* */
        STRPTR name;   /* optional, might be NULL          */
	ULONG fileoffset; /* location in input file */

	/* renderer specific */
	/* the mod loader will not touch these fields after init,
	   so if the renderer loads data here, it has to clean up after itself */
	APTR  data;      /* 8 bit or 16 bit sample data (signed/unsigned) */
	APTR  ext_ptr1;  /* unspecified fields, renderer specific */
	APTR  ext_ptr2;
	ULONG ext_data1;
	ULONG ext_data2;
	ULONG ext_data3;
	ULONG ext_data4;
};
#define MODSAM_TYPE_S8    0x0
#define MODSAM_TYPE_U8    0x1
#define MODSAM_TYPE_S16   0x22
#define MODSAM_TYPE_S16LE 0x23

#define MODSAM_IS_16BIT( a ) ( ((a)&0x20)>>5 )
#define MODSAM_IS_32BIT( a ) ( ((a)&0x40)>>6 )

#define MODCHF_PORTAMENTO 0x00000001 /* tone portamento is on */
#define MODCHF_SLIDEDELAY 0x00000002 /* slide fx delay (wait one tick) */
#define MODCHF_TREMOR     0x00000004 /* S3M: tremor is in effect */
#define MODCHF_EXTRADELAY 0x00000008 
#define MODCHF_LAST_EMPTY 0x00000010 /* last note was empty */

/* these flags are for convenience of the renderer */
#define MODCHF_VOLCHANGE  0x00010000 
#define MODCHF_PERCHANGE  0x00020000
#define MODCHF_SMPLCHANGE 0x00040000
#define MODCHF_CHGFLAGS   (MODCHF_VOLCHANGE|MODCHF_PERCHANGE|MODCHF_SMPLCHANGE)

struct MODChannel
{
	LONG period;     /* current period (non-sticky effects like ARP)                           */
	LONG baseperiod; /* initial/persistent period (sticky effects like Delta_Pitch,Portamento) */
	LONG flags;      /* status flags */

	WORD lvolume;    /* 0..255 - left panning  */
	WORD rvolume;    /* 0..255 - right panning */

        WORD volume;     /* current playback volume */
	WORD basevolume; /* slide start volume      */
	WORD volumediff; /* volume slide target     */
	WORD finevolumediff; /* Fine volume, applied at each counter==0 (i.e. at note or repeated with pattern delay) */


	/* mixing/rendering stuff */
	LONG sample_pos13; /* shifted by 13 bit left (room for fractional positions)  */
	LONG old_samplepos;
	LONG sample;       /* currently playing sample (0...mod->maxsamples, -1=no play) */
	LONG delaysample;  /* PT specialty: if >=0, then use this sample index after finishing the playback from "sample", i.e. the repeat part */

	/* note player support */
	LONG smp_note;     /* valid for one (current) tick: new sample to play (-1=no change) */
	LONG smp_repeat;   /* valid for one (current) tick: repeat part of playing sample (may be different from smp_note) */

	/* effect data */
	 ULONG fx;         /* current effect */
	 LONG retrig, retrig_delay, cutoff;
	 LONG loop_start, loop_count;
	 LONG finetune;

	 LONG arp, arp_base;   /* arpeggio data */
	 LONG port,port_dest;  /* portamento data */
	 LONG delta_pitch;
	 LONG vib_speed, vib_depth, avib_speed, avib_index, vib_wave; /* vibrato */
	 LONG trem_speed, trem_depth, atrem_speed, atrem_index, trem_wave; /* tremolo */
	 LONG glissando;
	 LONG vslideparm; /* S3M: remember volume slide parameters for cases where fxdat=0 */
	 LONG pslideparm; /* S3M: remember portamento (note slide) parameters */
         LONG tremorparm; /* S3M: tremor (vol on/off) */

	/* renderer data */
	 ULONG rnd_per,rnd_perstep;
};

struct MODPatternEntry
{
	WORD  period; /* period in the mod's base unit (perhaps note in S3M notation) or PAT_NOTE_OFF */
	WORD  sample; /* sample index    */
	WORD  fx;     /* FX_xx_HH_Title  */
	UBYTE fxdat;  /* argument for FX */
	UBYTE volume; /* S3M/FT2: specific volume, apart from FX (1...65 -> vol=0..64 , 0x00=no_volume_chane) */
};
#define PAT_NO_VOLUME  0x00 /* no volume signaled */
#define PAT_VOL_OFFSET 0x01 /* offset to distinguish between no volume and supplied volume */
#define PAT_NOTE_OFF  0x8000

#define MOD_ID_M_K_ 0x4D2E4B2E	/* M.K. = classic NoiseTracker and PT   */
#define MOD_ID_MIKI 0x4D214B21  /* M!K! = PT with more than 64 Patterns */
#define MOD_ID_FLT4 0x464C5434  /* FLT4 = StarTrekker */
#define MOD_ID_FLT8 0x464C5438  /* FLT8 = StarTrekker 8CH */
#define MOD_ID_OKTA 0x4F4B5441  /* OKTA */
#define MOD_ID_OCTA 0x4F435441  /* OCTA */
#define MOD_ID_8CHN 0x3843484E  /* 8CHN = FastTracker  */
#define MOD_ID_xCHN 0x0043484E  /*  CHN = TakeTracker? */
#define MOD_ID_xxCH 0x00004348  /*   CH = FastTracker  */
#define MOD_ID_S3M  0x5343524D  /* SCRM */

/* applies to (many, not all) PT Clones, too */
#define PT_IDPOS	0x438	/* 1080 */
#define PT_IDLEN	4
#define PT_LENPOS	0x3B6
#define PT_RESTARTPOS	0x3B7
#define PT_SONG         0x3B8
#define PT_SONGLENMAX	128
#define PT_SAMPLE_1	0x14
#define PT_SAMPLE_LEN	0x16
#define PT_SAMPLE_FINETUNE 0x18
#define PT_SAMPLE_VOL   0x19
#define PT_SAMPLE_REP   0x1A
#define PT_SAMPLE_REPLEN 0x1C
#define PT_SAMPLE_2_SAMPLE 0x1E
#define PT_MAX_SAMPLES 31
#define PT_SAMPLE_NAMELEN 0x14

/* Protracker Effects: 0x01-0xF1 */
#define FX_PT_NULL            0x0 /* don't ever change this! */
#define FX_PT_BASE            0x1 /* >0 */
#define FX_PT_00_ARP          (FX_PT_BASE+0x00)
#define FX_PT_10_SLIDEUP      (FX_PT_BASE+0x10)
#define FX_PT_20_SLIDEDOWN    (FX_PT_BASE+0x20)
#define FX_PT_30_SLIDENOTE    (FX_PT_BASE+0x30)
#define FX_PT_40_VIBRATO      (FX_PT_BASE+0x40)
#define FX_PT_50_VOLSLIDEN    (FX_PT_BASE+0x50)
#define FX_PT_60_VOLSLIDEV    (FX_PT_BASE+0x60)
#define FX_PT_70_TREMOLO      (FX_PT_BASE+0x70)
#define FX_FT_80_PAN          (FX_PT_BASE+0x80)
#define FX_PT_90_SOFFSET      (FX_PT_BASE+0x90)
#define FX_PT_A0_VOLSLIDE     (FX_PT_BASE+0xA0)
#define FX_PT_B0_JUMPOFFSET   (FX_PT_BASE+0xB0)
#define FX_PT_C0_VOLSET       (FX_PT_BASE+0xC0)
#define FX_PT_D0_JUMPROW      (FX_PT_BASE+0xD0)
#define FX_PT_E0_FILTER        (FX_PT_BASE+0xE0)
#define FX_PT_E1_FINESLUP      (FX_PT_BASE+0xE1)
#define FX_PT_E2_FINESLDOWN    (FX_PT_BASE+0xE2)
#define FX_PT_E3_GLISSCONTROL  (FX_PT_BASE+0xE3)
#define FX_PT_E4_SETVIBWAVE    (FX_PT_BASE+0xE4)
#define FX_PT_E5_SETFINETUNE   (FX_PT_BASE+0xE5)
#define FX_PT_E6_JUMPLOOP      (FX_PT_BASE+0xE6)
#define FX_PT_E7_SETTREMWAVE   (FX_PT_BASE+0xE7)
#define FX_PT_E8_KARPLUSSTRONG (FX_PT_BASE+0xE8)
#define FX_PT_E9_RETRIGNOTE    (FX_PT_BASE+0xE9)
#define FX_PT_EA_FINEVOLUP     (FX_PT_BASE+0xEA)
#define FX_PT_EB_FINEVOLDOWN   (FX_PT_BASE+0xEB)
#define FX_PT_EC_NOTECUT       (FX_PT_BASE+0xEC)
#define FX_PT_ED_NOTEDELAY     (FX_PT_BASE+0xED)
#define FX_PT_EE_ROWDELAY      (FX_PT_BASE+0xEE)
#define FX_PT_EF_FUNKREPEAT    (FX_PT_BASE+0xEF)
#define FX_PT_F0_SPEED        (FX_PT_BASE+0xF0)
#define FX_PT_MAX             FX_PT_F0_SPEED

#define FX_S3M_BASE	      (FX_PT_MAX+0x10)   /* 0x101 */
#define FX_S3M_A_SPEED	      (FX_S3M_BASE+0x00) /* same as PT F but larger range */
#define FX_S3M_E_PORTDOWN     (FX_S3M_BASE+0x10)
#define FX_S3M_F_PORTUP       (FX_S3M_BASE+0x20)
#define FX_S3M_I_TREMOR       (FX_S3M_BASE+0x30) /* TODO */
#define FX_S3M_Q_RETRIG_VOL   (FX_S3M_BASE+0x40) /* TODO */
#define FX_S3M_S8_SETPAN      (FX_S3M_BASE+0x50)
#define FX_S3M_SA_SETSTEREO   (FX_S3M_BASE+0x60) /* TODO (low priority) */
#define FX_S3M_T_TEMPO        (FX_S3M_BASE+0x70) /* same as PT, numbers >32 */
#define FX_S3M_U_FINEVIB      (FX_S3M_BASE+0x80) /* TODO */
#define FX_S3M_V_GVOLUME      (FX_S3M_BASE+0x90) /* TODO */
#define FX_S3M_D_VOLSLIDE     (FX_S3M_BASE+0xA0) 

#define FX_OK_BASE            (FX_S3M_BASE+0x50)
#define FX_OK_11_ARP4	      (FX_OK_BASE+0x0B)  /* TODO */
#define FX_OK_12_ARP5         (FX_OK_BASE+0x0C)  /* TODO */
#define FX_OK_13_NOTEDOWN     (FX_OK_BASE+0x0D)  /* TODO */
#define FX_OK_17_NOTEUP       (FX_OK_BASE+0x11)  /* TODO */
#define FX_OK_21_NOTEDOWNSLOW (FX_OK_BASE+0x15)  /* TODO */
#define FX_OK_30_NOTEUPSLOW   (FX_OK_BASE+0x1E)  /* TODO */

#define FX_MAX_DEFINED FX_OK_30_NOTEUPSLOW /* TODO: adjust with changes to FX list */

/* 4-8 channels (PT,ST,Fast,Take,...) */
struct MODPattern
{
	struct MODPatternEntry *patdata; /* since we like to stay flexible wrt.
	                                   #channels and #rows, a generic pointer
					   must do */
	LONG rowoff;    /* in (struct MODPatternEntry) units, usually 
	                   equal to nchannels */
	LONG nchannels; /* must be <= Mod->nchannels (4 for PT) */
	LONG nrows;     /* length of pattern (64 for PT) */
};

struct MOD
{
	struct MODSample **samples;   /* arranged as table */
	struct MODPattern **patterns; /* arranged as table */
	struct MODChannel **channels; /* arranged as table */
	LONG   *song;  /* 1D array */
	STRPTR name;   /* song title (maybe NULL) */
	ULONG  flags;

	struct ModPeriodTab pertab; /* period and finetuning tables, depending on source file type */

	LONG   patlen;    /* 64 for PT, maybe different for others */
	LONG   nchannels; /* 4 for PT, up to 16 with other clones  */

	LONG   npat;    /* up to 64 patterns for PT */
	LONG   songlen; /* number of entries in "song" */
        LONG   nsamples; /* number of samples in the file */
	LONG   maxsamples; /* max. valid sample index 0..MAX, 31 FOR PT */

	ULONG  modtype; /* just for fun: 'M.K.','FLT4','FLT8','OCTA',... */
	ULONG  mode;    /* internal: initial parser mode (PT,OCTALYZER,MED,...) */

	/* play data */
	LONG   filter;  /* 7kHz LP filter on (1) or off (0) */
	LONG   tick;
	LONG   counter; /* same as PT counter 0...speed-1 */
	LONG   songpos;
	LONG   patpos;
	LONG   speed;
	LONG   ciaspeed; /* */
	LONG   interval; /* interval between calls in 10us units */
	LONG   patdelay;
	LONG   skiptopos;

#ifdef AMIGA
	ASM void (*songend_callback)(ASMR(a1) struct MOD *mod ASMREG(a1));
	ASM void (*patscroll_callback)(ASMR(a1) struct MOD *mod ASMREG(a1));
#else
	void (*songend_callback)(struct MOD *mod );
	void (*patscroll_callback)(struct MOD *mod );
#endif

	/* debug */
	struct MODPatternEntry *pdta; /* current row */
};

/* this does happen with S3M */
#define MOD_SNG_END  -1 /* SONGEND */
#define MOD_SNG_SKIP -2 /* ignore, go to next position */

/* check if we have a ProTracker mod or maybe some relative */
/* in:  buf     - buffer pointer
        bufsize - number of bytes in buffer (>=1084 bytes)
   out: number of bytes needed for sample description, song and pattern data (minimum for mod_init() )
        numbers <=0 are error conditions 
*/
LONG mod_check( UBYTE *buf, LONG bufsize );


/* init module (song data) 
   in:  buf     - buffer pointer
        bufsize - number of bytes in buffer (>1084 bytes, see mod_check() )
        flags   - init flags
	off     - (optional, may be NULL) pointer to store first sample's 
	          offset in file
   out: 
        mod     - struct MOD* (or NULL for error)
        off     - offset of first sample in file (>0) or error condition (<=0)

   note: This player might be used on computers with low memory. Therefore,
         init is divided into two phases: song (here) and sample data.
	 Consequently, sample loading was relegated to the renderer (which
	 may offload the sample data elsewhere).
	  {
	        struct MOD *mod;
		LONG off = 0;
		LONG bufsize = 100000; // TODO: real file size
		UBYTE  *buf = (UBYTE*)malloc( 100000 ); // TODO: actually load a MOD into "buf"

		mod = mod_init( buf, bufsize, 0, &off );
	  }
   note: This function may need to allocate memory.
   note: The contents of "buf" are no longer needed after successful init. Everything
         relevant is copied such that the buffer may be free'd right away.
*/
struct MOD *mod_init( UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset );

/*
  get number of bytes for a sample (based on index 0..maxsamples).

  The output reflects the storage size within the mod file.
*/
LONG mod_numbytes_of_sample( struct MOD *mod, LONG idx, ULONG *fileoffset );

#if 0
/* incrementally init/load sample data (may be passed fully or in chunks) 
   in:  mod     - struct MOD after successful mod_init()
        buf     - sample data (starting at first sample offset as returned by 
	          mod_init(), then incrementally adding more consecutive
		  chunks from the mod file)
        bufsize - number of bytes in buffer 
   out: 0=no more data to read

   note: This function may need to allocate memory.
*/
LONG mod_sampleinit( struct MOD *mod, UBYTE *buf, LONG bufsize );
#endif

/* play one tick (without actual rendering) */
LONG mod_playinterval( struct MOD*mod );


/* done: free allocated resources
   in:  mod     - struct MOD after successful mod_init()
   out: NULL
*/
struct MOD *mod_free( struct MOD *mod );


/* INTERNAL */
#ifdef AMIGA
/* caution: if the binary/library is built without startup code,
            make sure you have SysBase somewhere as a global or
	    just define mod_AllocVec/mod_FreeVec as functions
	    
              APTR mod_AllocVec( LONG size )
              {
               struct Library *SysBase = *(struct Library**)0x4;
               return AllocVec(size,0x10001); // MEMF_PUBLIC|MEMF_CLEAR
              }
              void mod_FreeVec( APTR ptr )
              {
               struct Library *SysBase = *(struct Library**)0x4;
               FreeVec( ptr );
              }
*/
#include <proto/exec.h>
#define mod_AllocVec( size ) AllocVec( (size), 0x10001 )
#define mod_FreeVec( ptr ) FreeVec( ptr )
#else
#include <string.h>
#include <stdlib.h>
//#define mod_AllocVec(a) malloc(a)
#define mod_AllocVec(a) calloc(a,1)
#define mod_FreeVec(a) free(a)
#endif

/* parse/convert pattern data */
LONG mod_pt_loadpatterns( struct MOD *mod, UBYTE *buf );

/* provide End-of-Song callback  
 *
 * The passed pointer will be called once song end is reached in the player.
 * Please note that this function needs to be interrupt-safe.
 *
 *   in: mod  - modplay instance
 *       func - function to call (or 0 to remove)
 *
 */
#ifdef AMIGA
LONG mod_setendcallback( struct MOD *mod, ASM void (*func)(ASMR(a1) struct MOD *mod ASMREG(a1)));
#else
LONG mod_setendcallback( struct MOD *mod, ASM void (*func)(struct MOD *mod));
#endif

#ifdef AMIGA
LONG mod_SetPatScrollCallback( struct MOD *mod, ASM void (*func)(ASMR(a1) struct MOD *mod ASMREG(a1)));
#else
LONG mod_SetPatScrollCallback( struct MOD *mod, ASM void (*func)(struct MOD *mod));
#endif


#endif /* _INC_MODPLAY_H_ */
