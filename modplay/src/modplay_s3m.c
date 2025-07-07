/* **************************************************************************

   Screamtracker III loader, see modplay.c for the actual player 
   (C) 2024 Henryk Richter

* S3M.Threatening Behaviour - left only

   ************************************************************************** */
#include "modplay_s3m.h"
#include "modplay_tables.h" /* PT_Vibrato_Tab[32], MTM_FineTune_Tab_,PT_FineTune_Tab_ */
#include <stdio.h>

/* ignore mono flag ? (Yes, if defined) */
#define NOMONO

/* local proto */
struct MODPattern *mod_s3m_loadpattern( struct MOD *mod, UBYTE *buf, UBYTE *chanmap );

/* debug printf */
#if 0 
#define D(_x_)  printf _x_
#define D2(_x_) printf _x_
#define D3(_x_) printf _x_
#define DP(_x_) printf _x_
#define DS(_x_) printf _x_
#else
#define D(_x_)  
#define D2(_x_) 
#define D3(_x_) 
#define DP(_x_) 
#define DS(_x_) 
#endif
#if 0
#undef DP
#define DP(_x_) 
#endif
#if 1
#undef DS
#define DS(_x_)
#endif

struct S3MHeader
{
 BYTE  name[28];
 UBYTE sig1; /* const 0x1A */
 UBYTE filetype;
 UBYTE exp[2];
 UWORD songlen;
 UWORD ninstruments;
 UWORD npatterns;
 UWORD flags;
 UWORD cwt; /* "created with tracker" */
 UWORD sampleformat;
 UBYTE s3mid[4]; /* "SCRM" */
 UBYTE gvolume;  /* global volume 0..64 */
 UBYTE speed;
 UBYTE tempo;
 UBYTE mastervolume; /* bits 0..6 volume, bit 7 is stereo flag */
 UBYTE anticlick; /* irrelevant */
 UBYTE pan;
 UBYTE exp2[8];
 UWORD special;
 UBYTE chansettings[32];
} PACKATTR;

struct S3MInstr {
 UBYTE type;
 UBYTE dosname[12];
 UBYTE memseg[3];
 ULONG slen;
 ULONG loopstart;
 ULONG loopend;
 UBYTE vol;
 UBYTE unused;
 UBYTE packtype;
 UBYTE flags;
 UWORD C2SPD;
 UWORD unused2;
 UBYTE unused3[12];
 UBYTE sname[28];
 UBYTE scrs[4];
} PACKATTR;

/* these are generic but slow */
#define LEWORD( _p_ ) ( ( (UWORD)*( (UBYTE*)_p_ ) ) | (( (UWORD)*( (UBYTE*)_p_ + 1 ) ) << 8 ) )

#define LELONG( _p_ ) ( (( (ULONG)*( ((UBYTE*)_p_) + 3 ) )<<24) | \
                        (( (ULONG)*( ((UBYTE*)_p_) + 2 ) )<<16) | \
                        (( (ULONG)*( ((UBYTE*)_p_) + 1 ) )<<8)  | \
			(  (ULONG)*( ((UBYTE*)_p_) + 0 ) ) )

#define BELONG( _p_ ) ( (( (ULONG)*( ((UBYTE*)_p_)     ) )<<24) | \
                        (( (ULONG)*( ((UBYTE*)_p_) + 1 ) )<<16) | \
                        (( (ULONG)*( ((UBYTE*)_p_) + 2 ) )<<8)  | \
			(  (ULONG)*( ((UBYTE*)_p_) + 3 ) ) )


#define CWT_ST300 0x1300
#define CWT_ST301 0x1301
#define CWT_ST303 0x1303
#define CWT_ST320 0x1320
#define S3M_SIGNEDSAMPLES 0x1
#define S3M_UNSIGNEDSAMPLES 0x2

#define S3M_MIN_CHECK_BUFFER 1024 /* make sure to catch all para pointers */

#define S3M_SCRS 0x53435253

/* check if we have an S3M mod */
/* in:  mod     - mod structure to store nchannels in (if successful), NULL is ok/checked 
        buf     - buffer pointer
        bufsize - number of bytes in buffer (>=1084 bytes)
   out: number of bytes needed for sample description, song and pattern data (minimum for mod_init() )
        numbers <=0 are error conditions 
*/
LONG mod_check_s3m_internal( struct MOD *mod, UBYTE *buf, LONG bufsize )
{
 int i;
 UBYTE *curbuf;
 ULONG id;
 ULONG nchan = 0; /* default: error */
 LONG sz,slen,nsample,npat,ipos,maxposI,maxposP,pdif,lpos,true_slen,true_npat;
 ULONG mode = MOD_MODE_S3M;
 struct S3MHeader *hdr = (struct S3MHeader*)buf;
// struct S3MInstr *inst;

 if( bufsize < S3M_MIN_CHECK_BUFFER )
	return MOD_ERR_BUFFERTOOSMALL;

 /* byte number 29 is always 0x1A */
 if( hdr->sig1 != 0x1A )
 	return MOD_ERR_WRONGFORMAT;

 id = BELONG( hdr->s3mid );

 if( id != MOD_ID_S3M )
 	return MOD_ERR_WRONGFORMAT;

 /* additional checks: don't accept invalid settings */
 if( hdr->speed == 0 )
 	return MOD_ERR_WRONGFORMAT;
 if( hdr->tempo == 0 )
 	return MOD_ERR_WRONGFORMAT;

 /* now go through the header and collect information to calculate song length */
 D(("S3M SCRM signature found\n"));

 slen    = LEWORD(&hdr->songlen); /* song length, potentially with bogus entries */
 nsample = LEWORD(&hdr->ninstruments);
 npat    = LEWORD(&hdr->npatterns);

 D(("song len %ld nsample %ld npattern %ld\n",(long)slen,(long)nsample,(long)npat));

 /* get actual channel count */
 for( i=0 ; i < 32 ; i++ )
 {
	if( hdr->chansettings[i] < 16 ) /* conflicting documentation, let's keep to FS3MDOC.TXT */
		nchan++;
 }
 D(("%ld channels active\n",(long)nchan));
 if( nchan < 1 )
 	return MOD_ERR_WRONGFORMAT; /* no active channel -> bad file */

 /* skip header */
 curbuf = buf + sizeof( struct S3MHeader );
 /* parse song and determine actual song length */
 true_slen = 0;
 true_npat = 0;
 for( i=0 ; i < slen ; i++ )
 {
  if( (*curbuf != 0xff) && (*curbuf != 0xfe) )
  {
  	true_slen++;
	if( *curbuf > true_npat )
		true_npat = *curbuf;
  }
  curbuf++;
 }
 true_npat++;

 D(("header SL  %ld true %ld\n",(long)slen,(long)true_slen));
 D(("header PAT %ld true %ld\n",(long)npat,(long)true_npat));

 /* we're at the para pointers with "curbuf" */
 //printf("para pointers at %ld (0x%lx)\n",curbuf - buf,curbuf-buf );

 /* first parse the instrument para pointers */
 maxposI = 0;
 for( i = 0 ; i < nsample ; i++ )
 {
	ipos = LEWORD( curbuf )<<4;
	if( ipos > maxposI )
		maxposI = ipos;
	curbuf += 2;
 }

 /* next follow the pattern para pointers */
 maxposP = 0;
 ipos = 0;
 pdif = 0;
 for( i = 0 ; i < npat ; i++ )
 {
 	lpos = ipos;
 	ipos = LEWORD( curbuf )<<4;
	if( lpos )
	{
		if( ipos - lpos > pdif )
			pdif = ipos-lpos;
	}
	if( ipos > maxposP )
		maxposP = ipos;
	curbuf += 2;
 }
// printf("max packed pat size %ld\n",(long)pdif );

 /* skip panning, unless "default panning" is specified (redundant here) */
 if( hdr->pan == 0xFC )
 {
//	curbuf += 32; /* skip over panning */
 }

 /* now guess song size: either the instruments are first described in the file
    (common) or the patterns (yet to be seen by this guy) */
 if( maxposP > maxposI )
 {
 	sz = maxposP + nchan*320; /* 5 bytes per entry * rows * channels */
 }
 else
 {
	sz = maxposI + sizeof( struct S3MInstr );
 }

 D(("tempo %ld speed %ld\n",(long)hdr->tempo,(long)hdr->speed));

// printf("%ld (0x%lx)\n",curbuf - buf,curbuf-buf );
 D(("song size %ld\n",(long)sz));

 /* shall we fill something into the global struct ? */
 if( (mod) )
 {
	mod->modtype   = MOD_ID_S3M;
	mod->patlen    = 64;
	mod->nchannels = nchan;
	mod->npat      = npat;
	mod->songlen   = slen; /* potentially redundant but somebody might jump past official song end into a later section */
	mod->mode      = mode;
	mod->maxsamples= 100; /* 99 is the actual maximum */
	mod->filter    = 0;
 }

 return sz;
}




/* init S3M module (song data) 
   in:  buf     - buffer pointer
        bufsize - number of bytes in buffer (song size as of mod_check_s3m_internal() )
        flags   - init flags
	off     - (optional, may be NULL) pointer to store first sample's 
	          offset in file
   out: 
        mod     - struct MOD* (or NULL for error)
        off     - offset of first sample in file (>0) or error condition (<=0)

   note: It is assumed that the pointers are valid and the buffer is large enough
         as the checks are done in mod_init() already with a little help of
	 mod_check_pt_internal()

*/
struct MOD *mod_s3m_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset )
{
  LONG i,*sng,err,fpos,nchan;
  LONG sz,tmp,slen,nsample,npat,ipos,format,firstsmp=0,realnsample;
  UBYTE *curbuf;
  struct MODSample *smp,**ms;
  struct MODChannel *chan,**channels;
  struct S3MHeader *hdr = (struct S3MHeader*)buf;
  struct S3MInstr *inst;
  UBYTE  map_channels[32];  /* channel order is loose in S3M, remap channels tightly (internal idx -> S3M idx */
  UBYTE  rmap_channels[32]; /* reverse channel map (S3M idx -> internal idx) */
  struct MODPattern **mp,*pat;


  do
  {
   err = 1;

   slen    = LEWORD(&hdr->songlen); /* song length, potentially with bogus entries */
   nsample = LEWORD(&hdr->ninstruments);
   npat    = LEWORD(&hdr->npatterns);
   nchan   = mod->nchannels;

   /* */
   /* */
   /* copy song */
   sng = (LONG*)mod_AllocVec( sizeof(LONG) * slen );
   if( !sng )
  	break;
   mod->song = sng;

   curbuf = buf + sizeof( struct S3MHeader ); /* song follows S3M header */
   for( i=0 ; i < slen ; i++ )
   {
    if( *curbuf == 0xff )
    	*sng++ = MOD_SNG_END;
    else
    {
	if( *curbuf == 0xfe )
		*sng++ = MOD_SNG_SKIP;
	else	*sng++ = *curbuf;
    }
    curbuf++;
   }

   /* */
   /* */
   /* channels */
   for( i=0,tmp=0 ; i < 32 ; i++ )
   {
	rmap_channels[i] = 255; /* invalid by default */
	if( hdr->chansettings[i] < 16 )
	{
		map_channels[tmp] = i;  /* internal idx -> S3M idx */
		rmap_channels[i] = tmp; /* S3M idx -> internal idx */
		tmp++;
	}
   }

   //nchan = mod->nchannels;
   channels = (struct MODChannel **)mod_AllocVec( sizeof(struct MODChannel*) * nchan );
   if( !channels )
   	break;
   mod->channels = channels;

   for( i = 0 ; i < nchan ; i++ )
   {
	chan = (struct MODChannel *)mod_AllocVec( sizeof(struct MODChannel) );
	channels[i] = chan; 
	if( chan )
	{
		chan->volume = 64; /* default */
		tmp = map_channels[i]; /* remapped internal index to S3M index */

		tmp = hdr->chansettings[tmp] & 0xf; /* range is 0...F, where 8 is middle */

		/* orient the channels as suggested by the chansettings, typically 0x3/0xc */
		chan->lvolume = 255-( tmp | (tmp<<4) );
		chan->rvolume = ( tmp | (tmp<<4) );
#if 0
		/* default panning, potentially overridden after pattern parsing (full left/right) */
		if( hdr->chansettings[tmp] <= 7 )
		{
		 /* range is 0...F, where 8 is middle */
		 chan->lvolume = 255;
		 chan->rvolume = 0;
		}
		else
		{
		 chan->lvolume = 0;   /* 0...255 */
		 chan->rvolume = 255;
		}
#endif
	}
	else
		err++;
   }


   /* we're at the para pointers with "curbuf" */
   D(("init: para pointers at %ld (0x%lx)\n",(LONG)(curbuf - buf),(ULONG)(curbuf-buf) ));

   /* get sample information
      nsample is quite likely redundant for this purpose -but- we don't have to
      rename samples this way
   */
   ms = (struct MODSample **)mod_AllocVec( sizeof(struct MODSample *) * (mod->maxsamples+1) );//nsample );
   if( !ms )
   	break;
   mod->samples = ms;

   tmp = LEWORD( &hdr->sampleformat );
   format = (tmp == 1) ? MODSAM_TYPE_S8 : MODSAM_TYPE_U8;
   sz = 0;
   realnsample = 0;
   firstsmp = 0x7fffffff; /* LARGE, 0x01000000 would be sufficient, too :-) */
   /* first parse the instrument para pointers */
   for( i = 0 ; i < nsample ; i++ )
   {
	ipos = LEWORD( curbuf )<<4;
	inst = (struct S3MInstr *)(buf + ipos);
   	smp = NULL;

	tmp = LELONG( &inst->slen ); 
	if( (tmp > 0) && (inst->type==1) ) /* type2=adlib melody,type3=adlib drum */
	{
		smp = (struct MODSample *)mod_AllocVec( sizeof(struct MODSample) );
		if( !smp )
		{
			err++;
			break;
		}
		realnsample++;

		smp->data = NULL; /* no data yet */
		smp->ext_data1 = smp->ext_data2 = smp->ext_data3 = 0; /* renderer data */

		smp->type = format; /* S8/U8 */
		fpos = (((LONG)inst->memseg[0]) << 16) | \
		       (((LONG)inst->memseg[2]) << 8)  | \
		       ((LONG)inst->memseg[1]);
		fpos <<= 4; /* like para pointers, *16 */
		smp->fileoffset = fpos;
		if( fpos < firstsmp )
			firstsmp = fpos;

		smp->length     = tmp;
		sz += tmp;

		if( inst->flags & 1 ) /* loop enabled ? */
		{
		 tmp = LELONG( &inst->loopstart );
		 smp->repstart   = tmp;
		 tmp = LELONG( &inst->loopend );
		 smp->replen     = tmp - smp->repstart;
		}

		smp->finetune  = LEWORD( &inst->C2SPD );
		smp->volume    = inst->vol;
		smp->name      = (STRPTR)mod_AllocVec( 28 + 1 );
		if( smp->name )
		{
			int j = 28;
			STRPTR s,d;
			s = (STRPTR)inst->sname;
			d = smp->name;
			while( j-- )
				*d++ = *s++;
			*d = 0;
			DS(("%s (%ld,%ld,%ld,ft %ld)\n",smp->name,(long)smp->length,(long)smp->repstart,(long)smp->replen,(long)smp->finetune));
		}
	}

	*ms++ = smp;
	curbuf += 2;
   }
   mod->nsamples = realnsample;

   DS(("sample size %ld nsamples %ld real %ld\n",(long)sz,(long)nsample,(long)realnsample));

   mp = (struct MODPattern**)mod_AllocVec( mod->npat * sizeof( struct MODPattern* ) );
   mod->patterns = mp;
   if( !mp )
   {
   	err++;
	break;
   }

   /* next follow the pattern para pointers */
   for( i = 0 ; i < npat ; i++ )
   {
	ipos  = LEWORD( curbuf )<<4;
	//printf("%lx, ",(long)ipos);

	DP(("pattern 0x%lx\n",(long)i));
	pat   = mod_s3m_loadpattern( mod, buf + ipos, rmap_channels );
	*mp++ = pat;

	curbuf += 2;
   }
//   printf("\n");


   /* skip panning, unless explicitly specified as present */
   if( hdr->pan == 0xFC )
   {
   	D(("Read panning information: "));
	for( i=0 ; i < 32 ; i++ )
	{
		tmp = *curbuf++;
		if( rmap_channels[i] != 255 ) /* valid channel ? */
		{
			tmp &= 0xf; /* get lower bits for panning */
			D(("%ld ",(long)tmp));
			chan = channels[ map_channels[i] ];
			if( chan )
			{
				/* orient the channels as suggested by the explicit pan */
				chan->lvolume = 255-( tmp | (tmp<<4) );
				chan->rvolume = ( tmp | (tmp<<4) );
			}
			else
			{
				D(("error: invalid panning map\n"));
			}
		}
	}
	D(("\n"));
   }

#ifndef NOMONO
   if( hdr->mastervolume & 128 )
   {
   	D(("Mono flag is set\n"));
	for( i = 0 ; i < nchan ; i++ )
	{
			chan = channels[i];
			chan->lvolume = 128;
			chan->rvolume = 128;
	}
   }
#endif

   D(("first sample at %ld\n",(long)firstsmp));

   /* special period tab: we re-define the finetune as a base
      note (S3M specific) and apply the note index and associated
      period to compute the final sampling rate (or actually, period)
   */
   mod->pertab.ptable_type = MODPERTAB_S3M;
   mod->pertab.ptables[0] = (UWORD*)S3M_FineTune_Tab_;
   mod->pertab.ptable_idxmask = S3M_FINETUNE_IDXMASK;
   mod->pertab.ptable_maxidx = S3M_FINETUNE_STRIDE-2;  /* maximum index for offset clamping */
   mod->pertab.ptable_minper = 14;   /* 14 is in Amiga_Period notation, use 56 for Period4 */
   mod->pertab.ptable_maxper = 6848; /* 6848 is in Amiga_Period notation, use 27392 for Period4 */


   /* all good (unless "err" was incremented) */
   err--;

  } while(0);


  if( err )
  {
  	*fileoffset = MOD_ERR_MEMORY;
	return mod_free( mod );
  }

  /* reinit: clear songpos,patpos,patdelay,skiptopos */
  mod->tick     = 0;
  mod->songpos  = 0;
  mod->patpos   = 0;
  mod->speed    = hdr->speed;
  mod->ciaspeed = hdr->tempo;
  mod->patdelay = 0;
  mod->skiptopos= 0;

  mod->interval = 250000UL/(UWORD)(mod->ciaspeed); /* time increment in 10us units */
  *fileoffset   = firstsmp; /* byte offset in file of the first sample */

  return mod;
}



/*
  internal: load/convert pattern data
  in:  mod - internal structure, pre-filled with npat, nchannels,patlen
       buf - input buffer (first byte of MOD)
  out: npatterns or <0 for failure
*/
struct MODPattern *mod_s3m_loadpattern( struct MOD *mod, UBYTE *buf, UBYTE *chanmap )
{
	struct MODPattern *pat;
	struct MODPatternEntry *patdta;
	LONG  patsz,patbytes,tmp,nchan,tmp2;
	UWORD entries,nrow,ctrl,dchan;
	UBYTE *startbuf = buf;

	nchan   = mod->nchannels;
	entries = (UWORD)(nchan << 6); /* fixed 64 rows per pattern */
	patsz   = entries * (UWORD)sizeof(struct MODPatternEntry);

	pat     = (struct MODPattern*)mod_AllocVec( patsz + sizeof(struct MODPattern) ); 
	if( !pat )
		return pat;

	patdta = (struct MODPatternEntry *)(pat+1);

	pat->rowoff    = mod->nchannels;
	pat->nchannels = mod->nchannels;
	pat->nrows     = mod->patlen;
  	pat->patdata = patdta;

	patbytes = LEWORD( buf );
	buf += 2; /* skip packed size */

	/* always 64 rows */
	for( nrow = 0 ; nrow < 64 ; nrow++ )
	{
	 while( (buf-startbuf) < patbytes ) /* make sure not to loop indefinitely */
	 {
	 	ctrl = *buf++; 
//	 	printf("<%2lx> ",(long)ctrl);
	 	if( !ctrl )
	 	{
#if 1
			/* debug: print row */
			{
				int i;
				for( i=0 ; i < nchan ; i++ )
				{
				 DP( ("%3ld %02lx %2lx %03lx %02lx",  \
				      (long)patdta[i].period,\
				      (long)patdta[i].sample,\
				      (long)patdta[i].volume,\
				      (long)patdta[i].fx, \
				      (long)patdta[i].fxdat ) );
				 if( i != (nchan-1) )
				 {
				  DP(("|"));
				 }
				}
				DP(("\n"));
			}
#endif
			/* control byte == 0 -> end of row */
			patdta += nchan;
//			printf("\n");
			break;
		 }
		 dchan = chanmap[ ctrl & 31 ]; /* channel in S3M notation */
		 if( dchan == 255 )
		 {
		 	if( ctrl & 32 )
				if( *buf++ != 255 )
					buf++;
			if( ctrl & 64 )
				buf++;
			if( ctrl & 128 )
				buf+=2;
			ctrl = 0;
#if 0

		 	int i;
		 	printf("invalid dest channel %ld in control byte 0x%lx: bailing out, map:",(long)(ctrl&31),(long)ctrl);
			for( i=0; i < 32 ; i++ )
				printf("%ld ",(long)chanmap[i]);
			printf("\n");
			return pat;
#endif
		 }
		 if( ctrl & 32 )
		 {
			tmp = *buf++;
			if( tmp != 255 ) /* NO_NOTE -> isn't this redundant? */
			{
				if( tmp == 254 )
					tmp = PAT_NOTE_OFF;
		 		patdta[dchan].period = tmp;
				patdta[dchan].sample = *buf++;
			} else buf++;
		 }
		 if( ctrl & 64 )
		 {
		 	tmp = *buf++;
		 	patdta[dchan].volume = tmp + PAT_VOL_OFFSET;
		 }
		 if( ctrl & 128 )
		 {
		 	tmp = *buf++;
			tmp2= *buf++;
#if 1
			/* FX conversion */
			switch( tmp )
			{
				case 1:  tmp  = FX_S3M_A_SPEED; 
						if( tmp2 > 31 )
							tmp2 = 31;
						break;     /* A */
				case 2:  tmp  = FX_PT_B0_JUMPOFFSET; break;/* B */
				case 3:  tmp  = FX_PT_D0_JUMPROW; break;   /* C pattern break */
				case 4:  tmp  = FX_S3M_D_VOLSLIDE; break;  /* D (different from PT) */
				case 5:  tmp  = FX_S3M_E_PORTDOWN; break;  /* E same as PT 1 but more parameters */
				case 6:  tmp  = FX_S3M_F_PORTUP; break;    /* F see PT 2 but more parameters */
				case 7:  tmp  = FX_PT_30_SLIDENOTE; break; /* G slide to note */
				case 8:  tmp  = FX_PT_40_VIBRATO; break;   /* H */
				case 9:  tmp  = FX_S3M_I_TREMOR; break;    /* I */
				case 10: tmp  = FX_PT_00_ARP; break;       /* J */
				case 11: tmp  = FX_PT_60_VOLSLIDEV; break; /* K Vibrato + Volume Slide */
				case 12: tmp  = FX_PT_50_VOLSLIDEN; break; /* L Porta + Volume Slide */
				/* M,N ? */
				case 15: tmp  = FX_PT_90_SOFFSET; break;   /* O Sample offset */
				/* P ? */
				case 17: tmp  = FX_S3M_Q_RETRIG_VOL;break; /* Q Retrig + Volume Slide */
				case 18: tmp  = FX_PT_70_TREMOLO;break;    /* R Tremolo */
				case 19: /* S */
				{
					switch( (tmp2>>4)&0xf ) /* S0x...Sfx) */
					{
						case 0: /* S0x (Set filter) */
							tmp = FX_PT_E0_FILTER;break;
						case 1: /* S1x (Set glissando) */
							tmp = FX_PT_E3_GLISSCONTROL;break;
						case 2: /* S2x (Set finetune) */
							tmp = FX_PT_E5_SETFINETUNE;break;
						case 3: /* S3x (Set Vibrato waveform) */
							tmp = FX_PT_E4_SETVIBWAVE;break;
						case 4: /* S4x (Set Tremolo waveform) */
							tmp = FX_PT_E7_SETTREMWAVE;break;
						case 8: /* S8x (Pan position) */
							tmp = FX_S3M_S8_SETPAN;break;
						case 10: /* SAx (Stereo control) */
							tmp = FX_S3M_SA_SETSTEREO;break;
						case 11: /* SBx (Pattern loop) */
							tmp = FX_PT_E6_JUMPLOOP;break;
						case 12: /* SCx (Notecut) */
							tmp = FX_PT_EC_NOTECUT;break;
						case 13: /* SDx (Notedelay) */
							tmp = FX_PT_ED_NOTEDELAY;break;
						case 14: /* SEx (Patterndelay) */
							tmp = FX_PT_EE_ROWDELAY;break;
						case 15: /* SFx (Funkrepeat) */
							tmp = FX_PT_EF_FUNKREPEAT;break;
						default:
							DP(("Unhandled Sx effect 0x%lx 0x%lx\n",(long)tmp,(long)tmp2));
							tmp = 0;
							tmp2= 0;
							break;
					}
					tmp2 &= 0xf; /* args are 4 bit only */
				}
				break;
				case 20: tmp = FX_S3M_T_TEMPO;
					if( tmp2 < 33 )
					{
						tmp = FX_PT_NULL;
						tmp2 = 0;
					}
					break;  /* Txx (Set Tempo) */
				case 21: tmp = FX_S3M_U_FINEVIB;break; /* Uxy (Fine Vibrato) */
				case 22: tmp = FX_S3M_V_GVOLUME;break; /* Vxx (Global volume) */
				/* W,X,Y,Z ? */
				default:
//					DP(("Unhandled Effect %ld\n",(long)tmp));
					tmp = FX_PT_NULL; /* ignore */ 
					tmp2= 0;
					break;
			}
#endif
			patdta[dchan].fx = tmp;
			patdta[dchan].fxdat = tmp2;
		 }
	 }
	}

//	printf("%ld bytes read, patbytes %ld\n",(long)(buf-startbuf),(long)patbytes);
	DP(("\n"));

	return pat;
#if 0
   if( (tmp|tfx) ) /* effect code != 0 _or_ effect value */
   {
    if( tmp != 0xE )
    {
	tmp = FX_PT_BASE + (tmp<<4); /* FT_PT_BASE+0,0x10,0x20,... */
    }
    else
    {
	tmp  = FX_PT_BASE + (tmp<<4) + (tfx>>4); /* FT_PT_BASE+0xE0,0xE1,... */
	tfx &= 0xf; /* keep lower 4 bits only */
    }
   }
   patdta->fx    = tmp;
   patdta->fxdat = tfx;
#endif
}


