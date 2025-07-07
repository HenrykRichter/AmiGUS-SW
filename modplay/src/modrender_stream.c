
/* **************************************************************************

   Somewhat generic tracker renderer, starting (of course) with ProTracker
   (C) 2024 Henryk Richter

   This functionality assumes to have a Mod playing whose temporal increments
   yield in a number of played samples to be mixed. A software mixer will just
   take the current state of the replay and mix a number of samples conforming
   to the interval length between player ticks.

 ************************************************************************** */
#include "modplay.h"
#include "modrender_common.h"

#include "types.h" /* redirects to exec/types.h on Amiga */

struct MODRender {
	struct MOD *mod; /* mod we're rendering for */
	ULONG rate;
	ULONG flags;
};

LONG modrender_is_player( struct MODRender *rnd )
{
	return 0;
}

LONG modrender_start_play( struct MODRender *rnd )
{
	return 0; /* TODO: define error codes */
}

LONG modrender_stop_play( struct MODRender *rnd )
{
	return 0;
}


/* init renderer for a mod 
in:  mod     - struct MOD (after mod_init() )
rate    - sampling rate of the renderer
flags   - init flags
out: 

the returned renderer is prepared for the actual number of channels
and will contain placeholders for the samples to be loaded. Please load
the Mod's samples after the init step.
 */
struct MODRender *modrender_init( struct MOD *mod, LONG rate, ULONG flags )
{
	struct MODRender *mrnd = NULL;

	if( !mod )
		return mrnd;

	mrnd = (struct MODRender *)mod_AllocVec( sizeof( struct MODRender ) );
	if( !mrnd )
		return mrnd;

	mrnd->mod   = mod;
	mrnd->rate  = rate;
	mrnd->flags = flags;

	/* TODO: check if number of channels in the mod and their panning is actually supported */

	return mrnd;
}

/*
   Pass the loaded samples to the renderer

out: number of bytes left to be loaded for current sample

This call may receive partial data, i.e. you may feed the whole data set
or just a part.

ext_data3 = loaded samples in "sample" units (i.e. number of bytes or words)
 */
LONG modrender_loadsample( struct MODRender *rnd, LONG idx, UBYTE *buf, LONG bytes )
{
	struct MODSample *smp;

	/* TODO: proper error codes */
	if( !(rnd) || !(buf) || !(bytes) )
		return -1;
	if( !(rnd->mod) )
		return -2;
	if( !(rnd->mod->samples ) )
		return -3;
	if( (idx < 0) || (idx >= rnd->mod->maxsamples ) )
		return -4;

	smp = rnd->mod->samples[idx];
	if( !(smp) )
		return -5;

	/* allocate buffer space for loaded sample (reserve for 16 bit data) */
	if( !smp->data )
	{
		smp->data = mod_AllocVec( smp->length * sizeof(WORD) );
		if( !smp->data )
			return -6;
		smp->ext_data3 = 0; /* loaded bytes */

		if( smp->replen > 2 )
			smp->ext_data1 = (smp->repstart + smp->replen)<<13;
		else	smp->ext_data1 = (smp->length)<<13;
		smp->ext_data2 = 0;
	}

	{ /* TODO: store 16 bit samples, support other sample types (again) */
		BYTE *obuf = (BYTE*)smp->data + smp->ext_data3;
		LONG samples = bytes; /* assume 8 bit for now */
		LONG i;

		if( MODSAM_IS_16BIT( smp->type ) )
			samples >>= 1;
		if( MODSAM_IS_32BIT( smp->type ) )
			samples >>= 1;

		switch( smp->type )
		{
			case MODSAM_TYPE_S8:
			
				if( (smp->ext_data3 + samples)  > smp->length )
					samples = smp->length - smp->ext_data3;
				smp->ext_data3 += samples;

				for( i=0 ; i < samples ; i++ )
					*obuf++ = *buf++;
			
				break;

			case MODSAM_TYPE_U8:
				if( (smp->ext_data3 + samples)  > smp->length )
					samples = smp->length - smp->ext_data3;
				smp->ext_data3 += samples;

				for( i=0 ; i < samples ; i++ )
					*obuf++ = 0x80 ^ *buf++; /* unsigned to signed */

				break;
			default:
				return -7;
				break;

		}



	}


	return (LONG)(smp->length - smp->ext_data3);
}

/*
in: rnd  - renderer instance (after loading samples)
time - time frame in 10us units or number of samples to render (when negative)

out: number of new available samples from renderer

note: time should be reasonably small, not more than 0.5s (i.e. you'd need around 0.02s, anyway)
 */
LONG modrender_render( struct MODRender *mrnd, LONG time_10us, WORD *buf, LONG bufsize )
{
	LONG nsamples;
	LONG i,nchannels,per,rendered_samples,suml,sumr,cur;
	ULONG stp;
	LONG  spos13,spos,vscale;
	struct MOD *mod;
	struct MODSample **samples,*smp;
	struct MODChannel *chn,**channels;

	if( !mrnd )
		return 0;
	if( !mrnd->mod )
		return 0;

	mod = mrnd->mod;

	/* number of mixed samples */
	if( time_10us >= 0 )
	{
		if( !time_10us )
			return 0;

			/*
			1000 < t < 10000 (1/10s in 10us units)
			ns =   r * t / 100000
			ns =   r * (t>>5) / (100000/32)
			     + r * (t&31) / 100000
			*/
		nsamples  = (mrnd->rate * (time_10us>>5)) / 3125; /* /(100000/32) */
		nsamples += (mrnd->rate * (time_10us&31)) / 100000;
		/* nsamples = (mrnd->rate * time_10us)/100000; */
	}
	else
		nsamples = -time_10us;

	if( nsamples < 3 )
		return 0;

	/* we need to fit nsamples*2 for stereo into the output buffer */
	if( bufsize < (nsamples+nsamples) )
		return -1;

	samples  = mod->samples;
	nchannels= mod->nchannels;
	channels = mod->channels;
	/* preparation: period calculations (if necessary) */
	for( i = 0 ; i < nchannels ; i++ )
	{
		chn = channels[i];
		per = chn->period;
		if( (per != 0) && (per != chn->rnd_per) )
		{
			chn->rnd_per     = per;
			/* careful to stay in 32 bit range */
			stp = (3546895UL*1024UL)/(ULONG)(per*mrnd->rate);
			chn->rnd_perstep = stp<<3; /* step_per_sample<<13 */
		}
	}

	vscale = 256/(nchannels>>1);

	/* render loop */
	rendered_samples = nsamples;
	while( nsamples-- )
	{
		suml = 0;
		sumr = 0;
		for( i = 0 ; i < nchannels ; i++ )
		{
			chn    = channels[i];
			spos13 = chn->sample_pos13;
			if( spos13 >= 0 )
			{
				BYTE *dta; 

				smp  = samples[chn->sample];
				if( !smp )
				{
					chn->sample_pos13 = -1;
					continue;
				}
				if( spos13 >= smp->ext_data1 ) /* beyond length (either one-shot or repeat) */
				{
					if( smp->replen > 2 )
					{
						/* beyond repeat */
						spos13 -= (smp->replen<<13); 
						if( chn->delaysample >= 0 )
							chn->sample = chn->delaysample;
						chn->delaysample = -1;
					}
					else
					{ /* no replen, stop playing */
						chn->sample_pos13 = -1;
						continue;
					}
				}
				chn->sample_pos13 = spos13+chn->rnd_perstep;
				spos = spos13>>13;
				dta  = smp->data;
				cur  = (chn->volume * (dta[spos]<<8))>>6; /* TODO: 16 bit samples */

				suml += cur*chn->lvolume;
				sumr += cur*chn->rvolume;


			} /* spos13 */


		} /* for( i = 0 ; i < nchannels ; i++ ) */

		/* normalize suml, sumr */
		suml = (suml*vscale)>>16;
		sumr = (sumr*vscale)>>16;
#if 1
		/* clamping */
		if( suml > 32767 )
			suml = 32767;
		if( suml < -32768 )
		suml = -32768;
	if( sumr > 32767 )
		sumr = 32767;
	if( sumr < -32768 )
		sumr = -32768;
#endif
	*buf++ = suml;
	*buf++ = sumr;

  } /* while(nsamples--) */

  return rendered_samples;
}


/*
  deallocate, free resources
*/
LONG modrender_free( struct MODRender *rnd )
{
 LONG i;
 struct MODSample *smp;

 if( !rnd )
 	return 0;
 if( !rnd->mod )
 	return 0;
 if( !rnd->mod->samples )
 	return 0;

 for( i=0 ; i < rnd->mod->maxsamples ; i++ )
 {
	smp = rnd->mod->samples[i];
	if( !smp )
		continue;
	if( smp->data )
		mod_FreeVec( smp->data );
	smp->data = NULL;
 }

 mod_FreeVec( rnd );

 return 0;
}




