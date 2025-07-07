/* **************************************************************************

   Somewhat generic tracker player, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   ************************************************************************** */
/*
  TODO-List:
  - SongEnd detection and flags (nasty loops!)
  - FX_FT_80_PAN

  // ignored features, probably forever: these modify sample data
  - FX_PT_EF_FUNKREPEAT
  - FX_PT_E8_KARPLUSSTRONG (PT3.0, not in PT2.3)

*/
#define MODPLAY_MAIN
#include "modplay.h"
#include "modplay_tables.h" /* PT_Vibrato_Tab[32], MTM_FineTune_Tab_,PT_FineTune_Tab_ */
#include "modplay_protracker.h"
#include "modplay_s3m.h"
#include <stdio.h>

/* local proto */
//LONG mod_check_pt_internal( struct MOD *mod, UBYTE *buf, LONG bufsize );
struct MOD *mod_pt_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset );

LONG Period_Finetuned( struct ModPeriodTab *tab, ULONG per, LONG ft );
LONG Period_Offset_Finetuned( struct ModPeriodTab *tab, ULONG per, LONG delta, LONG ft );

/* mod_check, where a mod structure is supplied to be pre-filled with nchannels and such */
LONG mod_check_internal( struct MOD *mod, UBYTE *buf, LONG bufsize )
{
  LONG ret;

  /* try other formats if PT fails */
  ret = mod_check_pt_internal( mod, buf, bufsize );
  if( ret > 0 )
  	return ret;

  return mod_check_s3m_internal( mod, buf, bufsize );
}

/* check if we have a supported mod */
/* in:  buf     - buffer pointer
        bufsize - number of bytes in buffer (>=1084 bytes for PT and friends but 
                  best >= 10k for most formats)
   out: number of bytes needed for sample description, song and pattern data
        (minimum for mod_init() )
        numbers <=0 are error conditions 

   notes: if MOD_ERR_BUFFERTOOSMALL is returned, then we have detected a module
          but it is a format, where the song/pattern and sample data is 
	  potentially interleaved and/or the initial buffer was not sufficient
	  to determine the song size. In this case, the best option is to load 
	  the whole module for another try.
   note(2): With some less rigid formats, the song size returned is an estimate that
            just ensures sufficient data for mod_init().
*/
LONG mod_check( UBYTE *buf, LONG bufsize )
{
	return mod_check_internal( NULL, buf, bufsize );
}



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
struct MOD *mod_init( UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset )
{
  LONG sz,tmperr;
  struct MOD *mod;

  if( !fileoffset )
  	fileoffset = &tmperr;

  if( !(buf) )
  {
	*fileoffset = MOD_ERR_GENERAL;
  	return NULL;
  }

  mod = (struct MOD*)mod_AllocVec( sizeof(struct MOD) );
  if( !(mod) )
  {
  	*fileoffset = MOD_ERR_MEMORY;
	return NULL;
  }

  /* fill in nchannels, npatterns etc. */
  sz = mod_check_internal( mod, buf, bufsize );
  if( sz <= 0 )
  {
  	*fileoffset = sz;
  	return mod_free(mod);
  }
  if( sz > bufsize ) /* did we get enough data in buffer ? */
  {
  	*fileoffset = MOD_ERR_BUFFERTOOSMALL; /* tough luck: get me more bytes! */
  	return mod_free( mod );
  }

  /* prepare MOD struct (make sure, all array pointers are NULL) */
  mod->samples  = NULL;
  mod->patterns = NULL;
  mod->channels = NULL;
  mod->song     = NULL;
  mod->name     = NULL;
  mod->counter  = 0;
  mod->flags    = 0; /* FIXME: add and support flags */
  mod->songend_callback = NULL;

  if( !(flags & MODPF_NO_PT3QUIRKS) )
  	mod->flags |= MODPF_FIRST_PORT;

  if( mod->modtype == MOD_ID_S3M )
  {
	return mod_s3m_init( mod, buf, bufsize, flags, fileoffset );
  }

  /* basic setup done, now load relevant information and pattern data,
     branch to specific loader */
  if( (mod->mode == MOD_MODE_PT) ||
      (mod->mode == MOD_MODE_PCPT)
    )
  {
	return mod_pt_init( mod, buf, bufsize, flags, fileoffset );
  }

  /* sorry, loader not available */

  *fileoffset = MOD_ERR_WRONGFORMAT;
  return mod_free(mod);
}




/* done: free allocated resources
   in:  mod     - struct MOD after successful mod_init()
   out: 0 or some error code
*/
struct MOD * mod_free( struct MOD *mod )
{

  if( !mod )
  	return NULL;

  if( mod->patterns )
  {
  	int i;
	struct MODPattern**patts =  mod->patterns;

	for( i = 0 ;  i < mod->npat ; i++ )
	{
		if( patts[i] )
			mod_FreeVec( patts[i] );
	}

	mod_FreeVec( patts );
	mod->patterns = NULL;
  }

  if( mod->samples )
  {
   struct MODSample **ms = mod->samples;
   struct MODSample *smp;
   //LONG idx = -1;
   LONG ns = mod->nsamples;

   while( ns )
   {
   	smp = *ms++;
	//idx++;
	if( !(smp) )
		continue;
	ns--;
	
	if( smp->name )
		mod_FreeVec( smp->name );

	mod_FreeVec( smp );
   }

   mod_FreeVec( mod->samples );
   mod->samples = NULL;
  }

  if( mod->channels )
  {
   struct MODChannel**channels = mod->channels;
   LONG nchan = mod->nchannels;

   while(nchan--)
   {
	if( channels[nchan] )
		mod_FreeVec( channels[nchan] );
   }

   mod_FreeVec( channels );
   mod->channels = NULL;
  }

  if( mod->song )
  	mod_FreeVec(mod->song );
  mod->song = NULL;

  if( mod->name )
  	mod_FreeVec(mod->name );
  mod->name = NULL;

  mod_FreeVec( mod );

  return NULL;
}


/*
  return number of bytes a sample requires within a mod

  in: mod - mod structure after mod_init()
      idx - sample index (0...n)
      fo  - file offset within the mod itself 

  out: - number of bytes to load from file
       - optional (fo!=NULL), file offset, valid only if nbytes>0

  Please note that the sample index is not just consecutive
  from first to last. There may be unused sample indices
  in the mod leaving gaps in the enumeration - yet to keep the 
  sample index in the mod unchanged, please prepare for a 
  length 0 on occasion.

  example:
  {
   LONG i,fo,bytes_to_load;

   for( i = 0; i < mod->maxsamples ; i++ )
   {
	bytes_to_load = mod_bytes_of_sample(mod,i,&fo);
	if( bytes_to_load )
	{
	 // i is the index of a valid (non-empty) sample...
	 // file offset is optional (convenience) and just 
	 // copied from the sample struct
	}
   }

  }

*/
LONG mod_numbytes_of_sample( struct MOD *mod, LONG idx, ULONG *fileoffset )
{
   struct MODSample **ms;
   struct MODSample *smp;
   LONG ret = 0;

   do
   {
	if( !mod )
		break;

	ms = mod->samples;
	if( !ms )
		break;

	if( idx > mod->maxsamples ) /* caution: maybe something like "lastsample" is required */
		break;

//	fprintf(stderr,"numbytes for sample %ld\n",(long)idx);

	smp = mod->samples[idx];
	if( !smp )
		break;
	
	/* This function does not deal with playback considerations concerning loop start/len
	 * and just returns the raw length of the whole sample (in bytes) */
	ret = smp->length;

	/* samples to bytes, if necessary */
	if( MODSAM_IS_16BIT( smp->type ) )
		ret = ret<<1;
	if( MODSAM_IS_32BIT( smp->type ) )
		ret = ret<<2;

	if( fileoffset != NULL )
	{
		*fileoffset = smp->fileoffset;
	}

  } while(0);

  return ret;
}



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
LONG mod_setendcallback( struct MOD *mod, ASM void (*func)(ASMR(a1) struct MOD *mod ASMREG(a1)))
#else
LONG mod_setendcallback( struct MOD *mod, void (*func)(struct MOD *mod ))
#endif
{
	if(!mod )
		return -1;

	mod->songend_callback = func;
	return 0;
}

/* set Songend flag and issue callback (if appropriate)
 * 
 * */
void mod_SendSongEnd( struct MOD *mod )
{
	mod->flags |= MODPF_SONGEND;
	if( mod->songend_callback )
	{
		mod->songend_callback(mod);
	}
}

/* 
  find base period (note) index in table and return finetuned period

  S3M: current_period = 8363 * periodtab[note] / instrument's C2SPD 
       -> in that mode, "ft" is actually C2SPD and "period" is the note
       -> speed_hz = 14317056L / current_period (Paula_Rate*4)
*/
LONG Period_Finetuned( struct ModPeriodTab *tab, ULONG per, LONG ft )
{
  ULONG idx=0;
  UWORD *tab0,*tabft;

  if( !(tab0 = tab->ptables[0] ) )
  	return (LONG)per;

  /* somewhat flexible: allow for multiple fine tuning tables and a variable number of notes */
  if( tab->ptable_type == MODPERTAB_S3M )
  {
	if( !ft )
		ft = 8363;

	per = (per&0xf) + ((per>>4)&0xf)*12; /* convert note to period table index */
	idx = (tab0[per] * (UWORD)8363) / ft; /* this is Period4 */

	return (LONG)(idx>>2); /* return period (for now, TODO: Period4) */
  }

  /* table is descending: look for exact note but accept close match, table is 0-terminated */
  /* reason: we'd loop forever with bad modules */
  while( tab0[idx] > (UWORD)per )
  	idx++;

  if( idx > tab->ptable_maxidx ) /* 35UL for PT */
  	idx = tab->ptable_maxidx;

  tabft = tab->ptables[ft & tab->ptable_idxmask];

  return tabft[idx];
}


/* 
  find finetuned period (note) index in table and return offset from same table

  optionally with note index delta -> if there is a delta, refer to finetuned table for
  closest index, then apply delta
*/
LONG Period_Offset_Finetuned( struct ModPeriodTab *tab, ULONG per, LONG delta, LONG ft )
{
  ULONG idx=0;

#if 1
  /* somewhat flexible: allow for multiple fine tuning tables and a variable number of notes */

  UWORD *tabft;

  if( tab->ptable_type == MODPERTAB_S3M )
  {
   return (LONG)per;
  }

  if( !(tabft = tab->ptables[ft & tab->ptable_idxmask] ) )
  	return (LONG)per;

  /* table is descending: look for exact note but accept close match, table is 0-terminated */
  /* reason: we'd loop forever with bad modules */
  while( tabft[idx] > (UWORD)per )
  	idx++;

  idx += delta;
  if( idx > tab->ptable_maxidx ) /* 35UL for PT */
  	idx = tab->ptable_maxidx;

  return tabft[idx];

#else
 /* PROTRACKER ONLY */
 /* table is descending: look for exact note but accept close match */
 /* reason: we'd loop forever with bad modules */
  while( PT_FineTune_Tab[0][idx] > (UWORD)per ) 
  	idx++;

  idx += delta;
  if( idx > 35UL )
  	idx = 35;

  return PT_FineTune_Tab[ft][idx];
#endif
}

/*
   play one new note after the respective ticks expired (mod->speed based)
*/
void mod_playnote( struct MOD* mod )
{
  LONG songpos,nchan,i,sample_pos13,smpnum,per,ft,gosong,fxdat,l;
  UWORD patpos;
  struct MODPattern **patts,*patt;
  struct MODChannel **channels,*chn;
  struct MODSample  **samples,*s;
  struct MODPatternEntry *pdta;

  patpos = mod->patpos;
  songpos= mod->songpos;
  patts    = mod->patterns;
  l = *(mod->song + songpos);
  while( (l == MOD_SNG_SKIP) || (l==MOD_SNG_END) )
  {
	songpos++;

	if( l == MOD_SNG_END )
	{
		songpos = 0;
		mod_SendSongEnd(mod);
	}

	l = *(mod->song + songpos);
  }

  patt     = patts[ *(mod->song + songpos) ];

#if 0
  if( *(mod->song + songpos) == 2 )
  {
	printf("pat %ld spd %ld tmp %ld\n",(long)*(mod->song + songpos),(long)mod->speed,(long)mod->ciaspeed);
  }
#endif

  /* don't play note when pattern delay is active */
  if( mod->patdelay )
  {
	mod->patdelay--;
        /* this could be solved differently (i.e. advance pattern pos/song pos directly without re-considering here.) 
	   Reason for this code: compatibility with PTPlay3.0B (saved song/patpos after call) */
	if( !mod->patdelay ) 
  	{
		patpos = patpos + 1;
		if( patpos >= patt->nrows )
		{
			patpos = 0;
			songpos++;
			if( songpos >= mod->songlen )
			{
				songpos = 0;
				mod_SendSongEnd(mod);
			}
			mod->songpos = songpos;
		}
		mod->patpos = patpos;
	}
	return;
  }

  channels = mod->channels;
  samples  = mod->samples;

  mod->skiptopos = 0;

  pdta   = patt->patdata + ( patpos * (UWORD)patt->rowoff );
  nchan  = patt->nchannels;
  mod->pdta = pdta;

  for( i = 0 ; i < nchan ; i++ )
  {
	chn = channels[i];
	sample_pos13 = chn->sample_pos13;
	s   = NULL;

	if( chn->flags & MODCHF_LAST_EMPTY )
	{
		chn->period = chn->baseperiod;
		chn->flags &= ~MODCHF_LAST_EMPTY;
	}

#if 0
	if( (i==2) && (0x0==*(mod->song + songpos)) )
	{
		printf("%lx %lx %lx %lx (pp %ld)\n",(long)pdta->period,(long)pdta->sample,(long)pdta->fx,(long)pdta->fxdat,(long)patpos);
	}
#endif

	/* new sample and/or period on this channel ? */
	smpnum = pdta->sample; /* 1...maxsamples, 0=none */
	per    = pdta->period;

	/* empty PT note ? */
	l = pdta->fx | pdta->fxdat | smpnum | per;
	if( !l )
		chn->flags |= MODCHF_LAST_EMPTY;

	chn->flags &= ~(MODCHF_PORTAMENTO|MODCHF_TREMOR);

//	if( (!per) || (pdta->fx==FX_PT_30_SLIDENOTE) ||  (pdta->fx == FX_PT_50_VOLSLIDEN)  )
	if( !per ) 
	{
		/* OK, we don't have a period at current note. Here's the deal: PT will 
		   check some EFX and when those are not active, then we have to apply 
		   the current stored period (baseperiod) */
		switch( pdta->fx )
		{
			case FX_PT_90_SOFFSET:
			case FX_PT_B0_JUMPOFFSET:
			case FX_PT_C0_VOLSET:
			case FX_PT_D0_JUMPROW:
			case FX_PT_F0_SPEED:
				break;
			default:
				if( (pdta->fx < FX_PT_E0_FILTER) || (pdta->fx > FX_PT_EF_FUNKREPEAT) ) /* exclude all E commands */
					chn->period = chn->baseperiod;
				break;
		}
	}
	else
	{
		if( per == PAT_NOTE_OFF )
		{
			chn->volume = 0;
			chn->basevolume = 0;
			chn->smp_note = mod->maxsamples; /* unused slot, i.e. sample pointer == 0 */
			chn->sample_pos13 = -1;
			chn->flags |= MODCHF_VOLCHANGE|MODCHF_SMPLCHANGE;
			per = 0;
		}
	}

	if( (per == 0 ) && (chn->avib_speed) && (pdta->fx != FX_PT_40_VIBRATO) && (pdta->fx != FX_PT_60_VOLSLIDEV) )
	{
		per = chn->baseperiod;
	}

	/* default: end some FX with a new note */
	chn->avib_speed  = 0;
	chn->atrem_speed = 0;
	chn->volumediff  = 0;
	chn->delta_pitch   = 0;
	chn->arp    = 0;
	chn->retrig_delay  = 0;
	chn->finevolumediff = 0;

	/* new sample ? */
	if( (smpnum > 0) && (smpnum <= mod->maxsamples ) )
	{
		chn->flags |= MODCHF_SMPLCHANGE;
		if( per )
		{
			chn->delaysample = -1;
			chn->sample = smpnum-1;      /* we have a period, continue */
			chn->smp_note   = smpnum-1;  /* kept for one tick only */
			chn->smp_repeat = smpnum-1;
		}
		else
		{
			chn->delaysample = smpnum-1; /* no period given, store index for later */
			chn->smp_repeat  = smpnum-1; /* note for the repeat part (i.e. don't restart running sample, append new one after) */
		}

		s  = samples[smpnum-1]; /* mod->samples[smpnum-1] */
		ft = -1;
		if( s )
		{
			chn->basevolume = s->volume;
			chn->volume = s->volume;
			ft = s->finetune;
		}
		else
		{
			chn->basevolume = 0;
			chn->volume = 0; /* no sample, no volume */
			chn->sample = 0;
		}

		/* don't restart sample if no period given */
		if( per )
		{
		 if( ft >= 0 ) /* new sample ? */
			 chn->finetune = ft;
		 sample_pos13 = 0;
		}
	}

	do 
	{
		if( !per )
			break;
//		chn->arp_base = per;
		//if( s )
		chn->flags |= MODCHF_PERCHANGE;

		/* don't override tone slide with new note (we'll get there over time :-) */
		if( (pdta->fx == FX_PT_30_SLIDENOTE) || (pdta->fx == FX_PT_50_VOLSLIDEN) )
		{
			chn->period = chn->baseperiod;
			break;
		}

		if( pdta->period )
		{
			chn->avib_index = 0;	/* TODO: n_wavecontrol */
			chn->atrem_index = 0;

			per = Period_Finetuned( &mod->pertab, per , chn->finetune );
			chn->baseperiod = per;
			chn->port_dest = per;
			if( pdta->fx != FX_PT_ED_NOTEDELAY )
				chn->period = per;
		}

		if( s )
		{
			sample_pos13 = 0; /* restart sample */
			chn->smp_note   = chn->sample;
			chn->smp_repeat = chn->sample;
			chn->flags |= MODCHF_SMPLCHANGE;
		}
	} while(0);

	/* indiviual volume (S3M) */
	if( pdta->volume != PAT_NO_VOLUME )
	{
		chn->basevolume = pdta->volume - PAT_VOL_OFFSET;
		chn->volume = pdta->volume - PAT_VOL_OFFSET;
		chn->flags |= MODCHF_VOLCHANGE;
	}

	/* handle FX */
	chn->fx = pdta->fx;
	fxdat = pdta->fxdat;
	switch( pdta->fx )
	{
	 case FX_PT_00_ARP:
	 	chn->arp = fxdat;
		chn->arp_base = chn->baseperiod;
		chn->flags |= MODCHF_SLIDEDELAY;
		break;
	
	 /* S3M: slides, fine slides (0xFx) and extra fine slides (0xEx) */
	 case FX_S3M_E_PORTDOWN:
	 	if( fxdat )
			chn->pslideparm = fxdat;
		else	fxdat = chn->pslideparm;
	 	l = fxdat & 0xF0;
		if( l >= 0xe0 )
		{
		 	fxdat &= 0xf;
		 	if( l== 0xe0 ) /* EXTRA FINE ? */
		 	{
		 		fxdat >>= 2; /* sorry, no extra fine yet */
		 	}
		 	/* fine */
			fxdat += chn->baseperiod;
			chn->baseperiod = fxdat;
			chn->period = fxdat;
			mod->flags &= ~(MODPF_FIRST_PORT);
		}
		else
		{
			chn->delta_pitch = fxdat;
			chn->flags |= MODCHF_SLIDEDELAY;
		}
		chn->flags |= MODCHF_PERCHANGE;
	 	break;

	 case FX_S3M_F_PORTUP:
	 	if( fxdat )
			chn->pslideparm = fxdat;
		else	fxdat = chn->pslideparm;

	 	l = fxdat & 0xF0;
		if( l >= 0xe0 )
		{
		 	fxdat &= 0xf;
		 	if( l== 0xe0 ) /* EXTRA FINE ? */
		 	{
		 		fxdat >>= 2; /* sorry, no extra fine yet */
		 	}
			/* FINE */
			fxdat = chn->baseperiod - fxdat;
			chn->baseperiod = fxdat;
			chn->period = fxdat;
			mod->flags &= ~(MODPF_FIRST_PORT);
		}
		else
		{
			chn->delta_pitch = -fxdat;
			chn->flags |= MODCHF_SLIDEDELAY;
		}
		chn->flags |= MODCHF_PERCHANGE;
		break;

	 case FX_PT_10_SLIDEUP:
		fxdat = -fxdat;
		/* fall through */
	 case FX_PT_20_SLIDEDOWN:
		chn->delta_pitch = fxdat;
		chn->flags |= MODCHF_SLIDEDELAY;
		//if( !pdta->period )
		//	chn->flags |= MODCHF_EXTRADELAY;
		break;

	case FX_PT_50_VOLSLIDEN:
		if( fxdat & 0xf0 )
			chn->volumediff = fxdat>>4;
		else
			chn->volumediff = -(fxdat & 0xf);
		fxdat = 0; /* no new portamento setting */
		/* fall through */
	 case FX_PT_30_SLIDENOTE:
		if( fxdat )
			chn->port = fxdat;

		if( pdta->period )
			chn->port_dest = Period_Finetuned( &mod->pertab, pdta->period, chn->finetune );
		if( chn->port )
			chn->flags |= (MODCHF_PORTAMENTO|MODCHF_SLIDEDELAY);
		sample_pos13 = chn->sample_pos13;
		break;


	 case FX_PT_40_VIBRATO:
		if( fxdat & 0xf0 )
			chn->vib_speed = ((fxdat&0xf0)>>4);
	 	if( fxdat & 0xf )
			chn->vib_depth = fxdat&0xf;
		chn->avib_speed = chn->vib_speed;
		chn->flags |= MODCHF_SLIDEDELAY;
		break;


	 case FX_PT_60_VOLSLIDEV:
		if( fxdat & 0xf0 )
			chn->volumediff = fxdat>>4;
		else
			chn->volumediff = -(fxdat & 0xf);
		chn->avib_speed = chn->vib_speed;
		chn->flags |= MODCHF_SLIDEDELAY;
		break;


	 case FX_PT_70_TREMOLO:
		if( fxdat & 0xf0 )
			chn->trem_speed = ((fxdat&0xf0)>>4);
	 	if( fxdat & 0xf )
			chn->trem_depth = (fxdat&0xf)<<1;
		chn->atrem_speed = chn->trem_speed;
		break;

	 case FX_S3M_S8_SETPAN:
	 	/* S3M Panning, 4 bit only */
		fxdat &= 0xf;
		fxdat |= fxdat<<4;

		chn->lvolume = 255-fxdat;
		chn->rvolume = fxdat;
		chn->flags |= MODCHF_VOLCHANGE;
		break;

	 case FX_FT_80_PAN:
	 	/* TODO: Panning */
		break;


	 case FX_PT_90_SOFFSET:
		if( !pdta->period )
			break;
		/* TODO: check overflow and disable sample if offset too large (len=1 word), */
		fxdat <<= 8;
		if( !fxdat ) fxdat = chn->old_samplepos;
		if( !s ) fxdat += chn->old_samplepos;
		chn->old_samplepos = fxdat;
		sample_pos13 = fxdat<<13;
		
		break;

	 case FX_S3M_I_TREMOR: /* S3M effect: turn volume on/off based on counter */
	 	if( fxdat )
			chn->tremorparm = fxdat; /* S3M: tremor (vol on/off) */
		chn->flags |= (MODCHF_TREMOR);
	 	break;

	 case FX_S3M_D_VOLSLIDE:
	 	if( fxdat )
			chn->vslideparm = fxdat;
		else	
			fxdat = chn->vslideparm;
		 if( (fxdat != 0xff) && ((fxdat & 0xf0) == 0xf0 )) /* not fine up 0xf */
		 {
				chn->finevolumediff = -(fxdat & 0xf);
				break;
		 }
		 else
		 {
			 if( (fxdat & 0x0f) == 0x0f )
			 {
				chn->finevolumediff = (fxdat>>4);
				break;
			 }
		 }

		/* fall through to FX_PT_A0_VOLSLIDE code (if fine slide didn't apply) */
	 case FX_PT_A0_VOLSLIDE:
	        if( fxdat & 0xf0 )
			chn->volumediff = fxdat>>4;
		else
			chn->volumediff = -(fxdat & 0xf);
		chn->flags |= (MODCHF_SLIDEDELAY);
	 	break;


	 case FX_PT_B0_JUMPOFFSET:
	 	songpos = fxdat - 1;
		patpos  = patt->nrows - 1;
		break;


	case FX_PT_C0_VOLSET:
	 	if( fxdat > 64 )
			fxdat = 64;
		chn->basevolume = fxdat;
		chn->flags |= MODCHF_VOLCHANGE;
		break;


	 case FX_PT_D0_JUMPROW:
	 	if( !fxdat )
		{
			mod->skiptopos = 1; //patpos = patt->nrows - 1;
			break;
		}
		/* convert BCD to binary (non-negative) */
		fxdat = ((fxdat&0xf0)>>1) + ((fxdat&0xf0)>>3) + (fxdat&0xf);
		if( fxdat < patt->nrows )
			mod->skiptopos = fxdat+1;
		break;

	 case FX_S3M_A_SPEED:
		if( fxdat )
			mod->speed = fxdat;
		else	mod_SendSongEnd(mod);
	 	break;
	 case FX_S3M_T_TEMPO:
	 	if( fxdat )
		{
			mod->ciaspeed = fxdat;
			mod->interval = 250000UL/(UWORD)(mod->ciaspeed); /* time increment in 10us units */
		}
		else	mod_SendSongEnd(mod);
		break;
	 case FX_PT_F0_SPEED:
		if( (fxdat >= 32) && !(mod->flags & MODPF_VBLANK ))
		{
			mod->ciaspeed = fxdat;
			mod->interval = 250000UL/(UWORD)(mod->ciaspeed); /* time increment in 10us units */
		}
		else
		{
			if( fxdat )
				mod->speed = fxdat;
			else	mod_SendSongEnd(mod);
		}
		break;


	 /* PT Ex commands, pre-processed into specific FX codes here */
	 case FX_PT_E0_FILTER:
		mod->filter = (fxdat^1)&1;
	 	break;

	 case FX_PT_E1_FINESLUP:
	 	fxdat = -fxdat;
		/* fall through */
	 case FX_PT_E2_FINESLDOWN:
		// fxdat &= 0xf; /* done in pattern conversion */
		fxdat += chn->baseperiod;
		chn->baseperiod = fxdat;
		chn->period = fxdat;
		chn->flags |= MODCHF_PERCHANGE;

		/* this needs further work to be fully compatible (or as quirky as PT ;-)
		   repeat fine slide whenever mod->counter==0 (down in playinterval)
		*/
		mod->flags &= ~(MODPF_FIRST_PORT);
		break;

	 case FX_PT_E3_GLISSCONTROL:
	 	chn->glissando = fxdat;
		break;

	 case FX_PT_E4_SETVIBWAVE:
	 	chn->vib_wave = (fxdat & 3);
		break;

	 case FX_PT_E5_SETFINETUNE:
		if( !pdta->period )
			break;
		sample_pos13 = 0; /* retrig sample */
		chn->finetune = fxdat;
		chn->baseperiod = Period_Finetuned( &mod->pertab, pdta->period , chn->finetune );
		chn->port_dest = chn->baseperiod;
		chn->period = chn->baseperiod;
		chn->flags |= MODCHF_PERCHANGE;
		break;

	 case FX_PT_E6_JUMPLOOP:
		if( !fxdat )
		{
			chn->loop_start = patpos;
			break;
		}

		l = chn->loop_count;
		if( !l )
			chn->loop_count = fxdat;
		else
		{
			l--;
			chn->loop_count = l;
			if( !l )
				break;
		}
		patpos = chn->loop_start - 1;
		break;

	 case FX_PT_E7_SETTREMWAVE:
	 	chn->trem_wave = (fxdat&3);
		break;

	 case FX_PT_E8_KARPLUSSTRONG:
	 	/* FIXME: nasty in-place filter (2 tap box filter) on current sample as of PT3.10 */
	 	break;

	 case FX_PT_E9_RETRIGNOTE:
		// sample_pos13 = 0; /* retrig sample */
		chn->retrig = fxdat; /* note: multiple retrig cycles may happen before next note */
		chn->retrig_delay  = fxdat+1;
		break;

	 case FX_PT_EA_FINEVOLUP:
	 	chn->finevolumediff = fxdat;
#if 0
	 	fxdat += chn->basevolume;
		if( fxdat > 64 )
			fxdat = 64;
		chn->basevolume = fxdat;
		chn->volume = fxdat;
#endif
		break;

	 case FX_PT_EB_FINEVOLDOWN:
	 	chn->finevolumediff = -fxdat; /* applied later */
#if 0
	 	fxdat = chn->basevolume - fxdat;
		if( fxdat < 0 )
			fxdat = 0;
		chn->basevolume = fxdat;
		chn->volume = fxdat;
#endif
	 	break;

	 case FX_PT_EC_NOTECUT:
	 	chn->cutoff = fxdat+1;
	 	break;

	 case FX_PT_ED_NOTEDELAY:
	 	if( !pdta->period )
			break;
		chn->retrig_delay = fxdat + 1;
		chn->retrig = 0;
//		chn->nextsmp = 
		sample_pos13 = chn->sample_pos13;
	 	break;

	 case FX_PT_EE_ROWDELAY:
		mod->patdelay = fxdat;
	 	break;

	 case FX_PT_EF_FUNKREPEAT:
	 	/* FIXME: nasty in-place modification of samples */
		break;

	 default:
	 	/* unknown effect */
	 	break;

	} /* end switch( pdta->fx ) */

	/* end of FX */

        chn->sample_pos13 = sample_pos13;
	pdta++; /* next column */
  }

  /* pattern/song position, skipping */
  gosong = 1;
  if( mod->skiptopos )
  	patpos = mod->skiptopos-1; /* advance song pos with "skip" */
  else
  {
  	if( !mod->patdelay ) /* defer position increment with pattern delay */
	{
		patpos = patpos + 1;
		if( patpos >= patt->nrows )
			patpos = 0;
		else	gosong = 0; /* don't advance song pos in current pattern */
	}
	else
		gosong = 0;
  }
  
  if( gosong )
  {
	songpos++;
	if( songpos >= mod->songlen )
	{
		songpos = 0;
		mod_SendSongEnd(mod);

		/* TODO: SONGEND, re-init mod */
	}
	mod->songpos = songpos;
  }
  mod->patpos = patpos;

  /* done: just update tick */
//  mod->tick = mod->speed;
}


/* play one tick (without actual rendering) */
/*
   in:  mod  - from previous init call
   out: LONG - time interval in 10us units until next call to
               mod_playinterval() is expected
	       == 0 -> end of song
	        < 0 -> error
*/
LONG mod_playinterval( struct MOD*mod )
{
 struct MODChannel **channels,*chn;
 LONG i,nchan,tmp,tmp2;
 
 if( !mod )
 	return -1;

 channels = mod->channels;
 nchan    = mod->nchannels;

 /* ensure that new notes and/or changed repeat parts are valid for one tick only */
 for( i = 0 ; i < nchan ; i++ )
 {
  chn = *channels++;
  chn->smp_note   = -1;
  chn->smp_repeat = -1;
  chn->flags  &= ~(MODCHF_CHGFLAGS);
 }

 /* play new note, if counter is up */
 mod->counter++;
 if( mod->counter >= mod->speed )
 {
	mod_playnote( mod );
	mod->counter = 0;
 }
#if 0
if( !(mod->tick) )
 	mod_playnote( mod );
 mod->tick--;
#endif

 channels = mod->channels;

 /* go through active effects */
 for( i = 0 ; i < nchan ; i++ )
 {
  LONG vol;
  LONG per;

  chn = *channels++;

  /* period effects */
  do
  {
#if 1 
	if( (mod->counter != 0) )//|| (chn->avib_speed) )
	{
		/* TODO: extend to all ECommands */
		if( (chn->fx != FX_PT_E9_RETRIGNOTE) && (chn->fx != FX_PT_EC_NOTECUT) && (chn->fx != FX_PT_ED_NOTEDELAY ) )
//		if( (!chn->retrig_delay) && (!chn->cutoff) )
		{
			chn->period = chn->baseperiod;
		}
	}
#endif

  	if( chn->flags & MODCHF_SLIDEDELAY ) /* don't start slide effects immediately */
	{
		break; /* skip all period effects */
	}

	per = chn->delta_pitch;
	/* first: skip other effects if we have delta_pitch */
	if( !per )
	{
	 /* next: tone portamento */
	 if( chn->flags & MODCHF_PORTAMENTO )
	 {
		per  = chn->port_dest;   /* target period */
		per -= chn->baseperiod;  /* minus current */
		if( !per )
			break;         /* diff is 0, no more period changes (and fx) */
		tmp = chn->port;
		if( per < 0 )
		{
			tmp = -tmp;
			if( per < tmp )	/* period gets smaller (slide pitch up) */
				per = tmp;
		}
		else
		{ /* per >= 0 */
			if( per > tmp )	/* period gets larger (slide pitch down) */
				per = tmp;
		}
		/* fall through towards delta_pitch */
		chn->flags |= MODCHF_PERCHANGE;

	 }
	 else
	 {
		/* remaining FX: Arpeggio, Vibrato */
		if( chn->arp )
		{
			tmp = mod->counter % 3;
			if( !tmp )
			{
				chn->period = chn->baseperiod;
			}
			else
			{
				if( tmp == 1 )
					tmp = (chn->arp>>4)&15; /* 1 */
				else
					tmp = chn->arp & 15; /* 2 */
				per = Period_Offset_Finetuned( &mod->pertab, chn->arp_base, tmp, chn->finetune );
				chn->period = per;
			}
			chn->flags |= MODCHF_PERCHANGE;
#if 0
			tmp = ((chn->arp)>>((2-per)<<2)) & 15;
			//per = chn->arp_base;
			per = Period_Offset_Finetuned( &mod->pertab, chn->arp_base, tmp, chn->finetune );
			chn->period = per;
#endif
			break;
		}

		/* Vibrato ? */
		/*if( !mod->counter )
			break;*/

		if( !chn->avib_speed )
			break; /* no Vibrato active */

		tmp  = chn->avib_speed;
		per  = chn->avib_index;
		tmp += per;
		chn->avib_index = tmp&63; 

		tmp2 = 1;
		switch( chn->vib_wave )
		{
			case 0:
				tmp = PT_Vibrato_Tab[ per & 31 ]; /* avib_index */
				if( per & 32 ) 
					tmp2 = -1;
				break;
			case 2:
				if( per & 32 )
					tmp2 = -1;
				tmp = 255;
				break;
						
			default: 
				tmp = per<<3;
				if( per & 32 )
					tmp2 = -1;
				break;
		}

		per  = (chn->vib_depth * tmp)>>7;
		per *= tmp2; /* *1 or *-1, optimize! */
		per += chn->baseperiod;
//	        per  = (per > mod->pertab.ptable_maxper ) ? mod->pertab.ptable_maxper : ( (per < mod->pertab.ptable_minper ) ? mod->pertab.ptable_minper : per);
		chn->period = per;
		chn->flags |= MODCHF_PERCHANGE;
		break;
	 }
	}
	/* landing point for MODCHF_PORTAMENTO */

#if 1 
	/* PTPlay 2.3 and 3.0B quirk: first Portamento is off (due to initial mask == 0) */
	if( (mod->flags & MODPF_FIRST_PORT) && !(chn->flags & MODCHF_PORTAMENTO) )
	{
		mod->flags &= ~(MODPF_FIRST_PORT);
		break;
	}
#endif

/*	if( chn->flags & MODCHF_EXTRADELAY )
	{
		chn->flags &= ~MODCHF_EXTRADELAY;
		break;
	}*/

/*	if( mod->counter == 0 )
		break;*/

	/* delta_pitch */
	per += chn->baseperiod;
	if( chn->delta_pitch ) /* clamp only for delta_pitch, not for portamento */
	{
	        per  = (per > mod->pertab.ptable_maxper ) ? mod->pertab.ptable_maxper : ( (per < mod->pertab.ptable_minper ) ? mod->pertab.ptable_minper : per);
		chn->flags |= MODCHF_PERCHANGE;
	}
	chn->baseperiod = per;

	if( chn->glissando )
	{
	  /* re-tune period to closest finetuned note */
	  ULONG idx=0;
	  UWORD *tabft = mod->pertab.ptables[chn->finetune & mod->pertab.ptable_idxmask];
	  if( tabft ) // sanity
	  {
	   while( tabft[idx] > (UWORD)per )
	   	idx++;
	   per = tabft[idx];
	  }
	  chn->flags |= MODCHF_PERCHANGE;
	}
	chn->period = per;

  } while(0); /* end of period effects */
  
  /* volume effects */
  vol = chn->basevolume;

  if( (chn->volumediff) && !(chn->flags & MODCHF_SLIDEDELAY) )
  {
	vol += chn->volumediff;
	vol = (vol > 64 ) ? 64 : ( (vol < 0) ? 0 : vol);
	chn->basevolume = vol;
	chn->flags |= MODCHF_VOLCHANGE;
  }

  if( chn->finevolumediff) 
  {
   if( !(mod->counter) )
   {
	vol += chn->finevolumediff;
	vol =  (vol > 64 ) ? 64 : ( (vol < 0) ? 0 : vol);
	chn->basevolume = vol;
	chn->flags  |= MODCHF_VOLCHANGE;
   }
  }

  chn->flags &= ~MODCHF_SLIDEDELAY;

  if( chn->atrem_speed )
  {
	vol = chn->atrem_index;
	tmp = chn->atrem_speed + vol;
	chn->atrem_index = tmp&63;
	switch( chn->trem_wave )
	{
		case 0:
			tmp = PT_Vibrato_Tab[ vol & 31 ];
			if( vol > 31 )
				tmp = -tmp;
			break;
		case 2:
			tmp = ( vol > 31 ) ? -255 : 255;
			break;
		default:
			/* mode 1 */
			tmp = vol<<3;
			if( vol > 31 ) tmp -= 512;
	}
	vol = (tmp * chn->trem_depth) >> 8;
	vol += chn->basevolume;
	vol = (vol > 64 ) ? 64 : ( (vol < 0) ? 0 : vol);
	chn->flags  |= MODCHF_VOLCHANGE;
  }

  if( chn->cutoff )
  {
	chn->cutoff--;
	if( !chn->cutoff )
	{
		chn->basevolume = 0;
		vol = 0;
		chn->flags  |= MODCHF_VOLCHANGE;
	}
  }

  chn->volume = vol;

  /* retrig note and note delay */
  if( chn->retrig_delay )
  {
  	chn->retrig_delay--;
	if( !chn->retrig_delay )
	{
		chn->retrig_delay = chn->retrig;
		chn->smp_note   = chn->sample;
		chn->smp_repeat = chn->sample;
		chn->sample_pos13 = 0;
		chn->period = chn->baseperiod;
		chn->flags  |= MODCHF_PERCHANGE|MODCHF_SMPLCHANGE;
	}
  }
 }


 return mod->interval;
}

/* tempo calculations 
;tempo_min = 32   ; EP internal clamp, also: tempo is speed>=32
;tempo_speed == 0 ; ignored
;tempo range: 32....255 (78.1ms...9.8ms per tick)
;
;TEMPO=PRO_TEMPO ; def: 125
;CIACLK=709379   ; 0.7 MHz
;TCLK=1773447	 ;1789773
;TCLK/CIACLK=2.5
;
;CIATICKS=TCLK/TEMPO
;time_sec=CIATICKS/CIACLK=(TCLK/TEMPO)/CIACLK
;time_sec=TCLK/CIACLK/TEMPO
;time_ms=(1000*TCLK/CIACLK)/TEMPO
;time_us=(1e6*TCLK/CIACLK)/TEMPO
;time_us=2.5e6/TEMPO
;32/16 divide compatible approach (divu.w):
;time_10us=250000UL/(UWORD)TEMPO
;with rounding:
;time_10us=(250000UL+(TEMPO>>1)) / (UWORD)TEMPO
*/

