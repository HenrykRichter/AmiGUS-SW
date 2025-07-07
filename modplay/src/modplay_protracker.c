/* **************************************************************************

   Protracker (and similar) loader, see modplay.c for the actual player 
   (C) 2024 Henryk Richter

   ************************************************************************** */
#include "modplay_protracker.h"
#include "modplay_tables.h" /* PT_Vibrato_Tab[32], MTM_FineTune_Tab_,PT_FineTune_Tab_ */

/* debug printf */
#if 0 
#include <stdio.h>
#define D(_x_)  printf _x_
#define D2(_x_) printf _x_
#define D3(_x_) printf _x_
#define DP(_x_) printf _x_
#else
#define D(_x_)  
#define D2(_x_) 
#define D3(_x_) 
#define DP(_x_) 
#endif

/* local proto */
LONG mod_pt_loadpatterns( struct MOD *mod, UBYTE *buf );


/* check if we have a ProTracker mod or maybe some relative */
/* in:  mod     - mod structure to store nchannels in (if successful), NULL is ok/checked 
        buf     - buffer pointer
        bufsize - number of bytes in buffer (>=1084 bytes)
   out: number of bytes needed for sample description, song and pattern data (minimum for mod_init() )
        numbers <=0 are error conditions 
*/
LONG mod_check_pt_internal( struct MOD *mod, UBYTE *buf, LONG bufsize )
{
 int i;
 unsigned int cur;
 ULONG id;
 ULONG nchan = 0; /* default: error */
 LONG sz;
 ULONG mode = MOD_MODE_PT; /* default: ProTracker */

 if( bufsize < (PT_IDPOS+PT_IDLEN) )
	return MOD_ERR_BUFFERTOOSMALL;

 id = (((ULONG)buf[PT_IDPOS])  <<24) + 
      (((ULONG)buf[PT_IDPOS+1])<<16) + 
      (((ULONG)buf[PT_IDPOS+2])<<8)  + 
      ((ULONG)buf[PT_IDPOS+3]); 

 do
 {
   /* ProTracker, StarTrekker ? */
   if( (id == MOD_ID_M_K_) ||
       (id == MOD_ID_MIKI) ||
       (id == MOD_ID_FLT4)
     )
   {
     nchan = 4;
     break;
   }

   /* 8 channel variants */
   if( id == MOD_ID_FLT8 )
   {
     //mode = MOD_MODE_PT; // redundant
     nchan = 8;
     break;
   }

   if(
       (id == MOD_ID_OKTA) ||
       (id == MOD_ID_OCTA) ||
       (id == MOD_ID_8CHN)
     ) /* TODO: FA08, CD81 */
   {
     mode = MOD_MODE_PCPT;
     nchan = 8;
     break;
   }

   /* variable number of channels (1-8) */
   if( ( (id>>24) >= '1' ) && 
       ( (id>>24)  < '9' ) &&
       ( (id & 0xFFFFFF) == MOD_ID_xCHN )
     )
   {
     nchan = (id>>24)-(ULONG)'0'; /* we could also use (id>>24)&0xf ... */
     mode = MOD_MODE_PCPT;
     break;
   }

   /* variable number of channels (10-32) */
   if( ( (id>>24) >= '1' ) &&          /* first digit 1-3 */
       ( (id>>24)  < '3' ) &&
       ( ((id>>16)&0xFF) >= '0' ) &&   /* second digit 0-9 */
       ( ((id>>16)&0xFF) <= '9' ) &&
       ( (id & 0xFFFF) == MOD_ID_xxCH )
     )
   {
     nchan  = (id>>24)-(ULONG)'0';
     nchan  = ( (nchan<<2) + nchan )<<1; /* nchan * 10 */
     nchan += ( (id>>16) & 0xff ) - (ULONG)'0';
     mode = MOD_MODE_PCPT;
     break;
   }

 } while(0);

 if( !nchan )
 	return MOD_ERR_WRONGFORMAT;

 /* now calculate size of header + pattern data */

 /* check all "mentioned" patterns in song, i.e.
    even when not actually played as told by songlen */
 sz = 0;
 for( i=0 ; i < PT_SONGLENMAX ; i++ ) /* 128 */
 {
	cur = (unsigned int)buf[PT_SONG+i];
	if( cur > sz )
		sz = cur;
 }
 if( id == MOD_ID_FLT8 )
	sz >>= 1; /* 8 Channel StarTrekker counts pattern numbers *2 */

 sz++; /* number of patterns */

 /* shall we fill something into the global struct ? */
 if( (mod) )
 {
	mod->modtype   = id;
	mod->patlen    = 64;
	mod->nchannels = nchan;
	mod->npat      = sz;
	mod->songlen   = buf[PT_LENPOS];
	mod->mode      = mode;
	mod->maxsamples= PT_MAX_SAMPLES; /* 31 */
	mod->filter    = 0;
 }

 /* compute pattern data size */
 sz   = (WORD)sz * (WORD)nchan; /* 16*16=32 multiply is sufficient */
 sz <<= 8; /* nchannels * 64 rows * 4 bytes per entry * npatterns */

 /* add header size to pattern size */
 sz  += PT_IDPOS + PT_IDLEN;

 return sz;
}



/* init protracker or similar module (song data) 
   in:  buf     - buffer pointer
        bufsize - number of bytes in buffer (>1084 bytes, see mod_check() )
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
struct MOD *mod_pt_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset )
{
  LONG sz,i,*sng,err,fpos,ns,nchan,shft;
  UBYTE *curbuf;
  struct MODSample *smp,**ms;
  struct MODChannel *chan,**channels;

  /* compute pattern data size */
  nchan = mod->nchannels;
  sz  = (WORD)mod->npat * (WORD)nchan; /* 16*16=32 multiply is sufficient */
  sz <<= 8; /* nchannels * 64 rows * 4 bytes per entry * npatterns */
  sz  += PT_IDPOS + PT_IDLEN; /* add header size to pattern size */
 
  do
  {
   err = 1;

   /* */
   /* */
   /* load/parse pattern data (needs npat,patlen,nchannels in MOD struct) */
   if( mod_pt_loadpatterns( mod, buf ) < 0 )
   	break;

   /* */
   /* */
   /* copy song */
   sng = (LONG*)mod_AllocVec( sizeof(LONG) * mod->songlen );
   if( !sng )
  	break;

   mod->song = sng;
   shft = ( mod->modtype == MOD_ID_FLT8 ) ? 1 : 0;
   for( i = 0 ; i < mod->songlen ; i++ )
   {
	*sng++ = buf[PT_SONG+i]>>shft;
   }

   /* */
   /* */
   /* get sample information (+1 reserve pointer for "note cut" feature)*/
   ms = (struct MODSample **)mod_AllocVec( sizeof(struct MODSample *) * (PT_MAX_SAMPLES+1) );
   if( !ms )
   	break;
   mod->samples = ms;

   curbuf = &buf[PT_SAMPLE_1]; /* first sample (offset 20) */
   fpos = sz; /* first sample already computed */
   for( i = 0, ns = 0 ; i < PT_MAX_SAMPLES ; i++ )
   {
   	smp = NULL;

	/* do we have an active sample here ? */
   	if( (curbuf[PT_SAMPLE_LEN] + curbuf[PT_SAMPLE_LEN+1]) > 0 )
	{
		smp = (struct MODSample *)mod_AllocVec( sizeof(struct MODSample) );
		if( !smp )
			break;
		smp->data      = NULL; /* no data yet */
		smp->length    = (((LONG)curbuf[PT_SAMPLE_LEN])<<9) + (((LONG)curbuf[PT_SAMPLE_LEN+1])<<1); /* words to nsamples (or bytes) */
		smp->repstart  = (((LONG)curbuf[PT_SAMPLE_REP])<<9) + (((LONG)curbuf[PT_SAMPLE_REP+1])<<1);
		smp->replen    = (((LONG)curbuf[PT_SAMPLE_REPLEN])<<9) + (((LONG)curbuf[PT_SAMPLE_REPLEN+1])<<1);
		smp->finetune  = curbuf[PT_SAMPLE_FINETUNE];
		smp->volume    = curbuf[PT_SAMPLE_VOL];
		smp->type      = MODSAM_TYPE_S8;
		smp->name      = (STRPTR)mod_AllocVec( PT_SAMPLE_NAMELEN + 1 );
		if( smp->name )
		{
			int j = PT_SAMPLE_NAMELEN;
			STRPTR s,d;
			s = (STRPTR)curbuf;
			d = smp->name;
			while( j-- )
				*d++ = *s++;
			*d = 0;
		}
		/* workaround for old soundtracker modules whose loopstart was in byte units */
		if( (smp->replen > 2) && ( (smp->repstart+smp->replen) > smp->length ) )
		{
			if( ((smp->repstart>>1) + smp->replen) <= smp->length )
				smp->repstart >>= 1;
		}

		smp->fileoffset = fpos;
		smp->ext_data1 = smp->ext_data2 = smp->ext_data3 = 0;
		fpos += smp->length;
		ns++;
	}

	*ms++ = smp; /* this way leaves gaps in the sample array but we can keep the original indices */
	curbuf += PT_SAMPLE_2_SAMPLE; /* offset 0x1E = 30 */ 
   }
   mod->nsamples = ns;

   /* */
   /* */
   /* channels */
   nchan = mod->nchannels;
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
		chan->lvolume = 0;
		chan->rvolume = 0;

		if( ((i&3)==0) || ((i&3)==3) ) /* Amiga: channels 0,3 = left, 1,2 = right */
			chan->lvolume = 255; /* full left  */
		else	chan->rvolume = 255; /* full right */
	}
	else
		err++;
   }

   /* */
   /* */
   /* name */
   mod->name = (STRPTR)mod_AllocVec( PT_SAMPLE_1 + 1 );
   if( mod->name )
   {
	int j = PT_SAMPLE_NAMELEN;
	STRPTR s,d;
	s = (STRPTR)buf;
	d = mod->name;
	while( j-- )
		*d++ = *s++;
	*d = 0;
   }

   mod->pertab.ptable_type = MODPERTAB_PERIOD; /* PT style periods */
#if 1
   if( mod->mode == MOD_MODE_PCPT )
   {	/* MTM and similar, no finetuning yet */
	mod->pertab.ptables[0] = (UWORD*)MTM_FineTune_Tab_;
	mod->pertab.ptable_idxmask = MTM_FINETUNE_IDXMASK;
	mod->pertab.ptable_maxidx = MTM_FINETUNE_STRIDE;  /* maximum index for offset clamping */
        mod->pertab.ptable_minper = 45;   /* PT: 113 */
        mod->pertab.ptable_maxper = 1616; /* PT: 856 (with finetuning, actual max would be 907 but PT clamps the former) */
   }
   else
#endif
   { /* classic PT period table */
	int j;
	const UWORD *ptab = PT_FineTune_Tab_;
	for( j = 0 ; j < (PT_FINETUNE_IDXMASK+1) ; j++ )
	{
		mod->pertab.ptables[j] = (UWORD*)ptab;
		ptab += PT_FINETUNE_STRIDE;
	}
	mod->pertab.ptable_idxmask = PT_FINETUNE_IDXMASK;
	mod->pertab.ptable_maxidx = 35;  /* maximum index for offset clamping */
        mod->pertab.ptable_minper = 113; /* PT: 113 */
        mod->pertab.ptable_maxper = 856; /* PT: 856 (with finetuning, actual max would be 907 but PT clamps the former) */
   }

   /* */
   /* */
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
  mod->speed    = 6;
  mod->ciaspeed = 125;
  mod->patdelay = 0;
  mod->skiptopos= 0;

  mod->interval = 250000UL/(UWORD)(mod->ciaspeed); /* time increment in 10us units */
  *fileoffset   = sz; /* byte offset in file of the first sample */

  return mod;
}



/*
  internal: load/convert pattern data
  in:  mod - internal structure, pre-filled with npat, nchannels,patlen
       buf - input buffer (first byte of MOD)
  out: npatterns or <0 for failure
*/
LONG mod_pt_loadpatterns( struct MOD *mod, UBYTE *buf )
{
 int i,j;
 struct MODPattern **mp,*pat;//,*tmppat;
 struct MODPatternEntry *patdta;
 UBYTE *curbuf;
 ULONG patsz,entries,tmp,tfx;
 ULONG rows,passes,cols1,ps,rw,cl;

 if( mod->npat < 1 )
 	return MOD_ERR_GENERAL;

 mp = (struct MODPattern**)mod_AllocVec( mod->npat * sizeof( struct MODPattern* ) );
 mod->patterns = mp;
 if( !mp )
 	return -1;

 curbuf   = buf + PT_IDPOS + PT_IDLEN; /* first pattern (index 0) */
 entries  = ((UWORD)mod->nchannels * (UWORD)mod->patlen);
 patsz    = (UWORD)entries * (UWORD)sizeof(struct MODPatternEntry); /* nchannels * length * 8 bytes per entry */

 rows = mod->patlen;
 if( mod->modtype != MOD_ID_FLT8 )
 {
	passes = 1;
	cols1  = mod->nchannels;
 }
 else
 {
	passes = 2;
	cols1  = 4;
 }

// tmppat   = NULL;
// if( mod->modtype == MOD_ID_FLT8 )
//	tmppat   = mod_AllocVec( patsz + sizeof(struct MODPattern) ); 

 for( i=0 ; i < mod->npat ; i++ )
 	mp[i] = NULL;

 for( i=0 ; i < mod->npat ; i++ )
 {
  /* header (struct MODPattern) plus Body (struct MODPatternEntry) */
  pat   = mod_AllocVec( patsz + sizeof(struct MODPattern) ); 
  *mp++ = pat;

  if( !pat )
  	return -1;

  pat->rowoff    = mod->nchannels;
  pat->nchannels = mod->nchannels;
  pat->nrows     = mod->patlen;

  patdta = (struct MODPatternEntry *)(pat+1);
  pat->patdata = patdta;

  DP(("\n------- pattern %ld -------",(long)i));

  /* parse input pattern */
  /* extend loop to loops:
   *  - 1 or 2 passes
   *  - rows
   *  - columns, nchannels-columns step
   *  - column offset for pass 2 (same as columns)
   */
  for( ps = 0; ps < passes ; ps++ )
  {
   patdta = pat->patdata;
   if( ps )
	   patdta += cols1; /* second pass: column offset */

   for( rw = 0; rw < rows ; rw++ )
   {
    for( cl = 0; cl < cols1; cl++ )
    {
	   patdta->period = (((WORD)(0xf  & curbuf[0]))<<8)+(WORD)curbuf[1];
	   patdta->sample = ((WORD)(0xf0 & curbuf[0]))+((WORD)(curbuf[2]>>4)&0xf);
	   patdta->volume = PAT_NO_VOLUME;
	   tmp 	 = curbuf[2] & 0xf; /* primary effect */
	   tfx   = curbuf[3]; /* effect argument */
	   curbuf += 4; /* patterns are consecutive in ST/NT/PT/StarTrekker... (but segmented in ST8) */

	/* debug sample indices */
#if 0
	   if( (j % mod->nchannels) == 0 )
		printf("\n");
	   if( (j % (pat->nrows * pat->nchannels)) == 0)
		printf("--\n");
	   printf("%2x ",patdta->sample);
#endif

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

	   if( !(cl) )
	   {
		DP(("\n"));
	   }
	   else
	   {
		DP(("|"));
	   }
	   {
		DP(("%3ld %02lx %1lx%02lx",\
		   (long)patdta->period,\
		   (long)patdta->sample,\
		   (long)patdta->fx, \
		   (unsigned long)patdta->fxdat ) );
	   }
     	patdta++;
    }
    patdta += mod->nchannels-cols1;
   }
  }
  patdta = pat->patdata;

#if 0
  for( j = 0 ; j < entries ; j++ )
  {
   patdta->period = (((WORD)(0xf  & curbuf[0]))<<8)+(WORD)curbuf[1];
   patdta->sample = ((WORD)(0xf0 & curbuf[0]))+((WORD)(curbuf[2]>>4)&0xf);
   patdta->volume = PAT_NO_VOLUME;
   tmp 	 = curbuf[2] & 0xf; /* primary effect */
   tfx   = curbuf[3]; /* effect argument */
   curbuf += 4; /* patterns are consecutive in ST/NT/PT/StarTrekker... */


/* debug sample indices */
#if 0
   if( (j % mod->nchannels) == 0 )
   	printf("\n");
   if( (j % (pat->nrows * pat->nchannels)) == 0)
   	printf("--\n");
   printf("%2x ",patdta->sample);
#endif

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

   if( !(j%pat->nchannels) )
   {
	DP(("\n"));
   }
   else
   {
   	DP(("|"));
   }
   {
	DP(("%3ld %02lx %1lx%02lx",\
	   (long)patdta->period,\
	   (long)patdta->sample,\
	   (long)patdta->fx, \
	   (unsigned long)patdta->fxdat ) );
   }


   patdta++;
  }
#endif
 }


// if( tmppat )
//	 mod_FreeVec(tmppat);

 return mod->npat;
}


