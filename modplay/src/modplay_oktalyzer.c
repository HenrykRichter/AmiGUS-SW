/* **************************************************************************

   Oktalyzer loader, see modplay.c for the actual player 
   (C) 2024 Henryk Richter

   limitations:
   - Oktalyzer got an "Attack-Sustain-Release" feature in later versions,
     where Attack/Sustain were handled in normal Start+Loop fashion. For the 
     release part, a special command was added in later OK versions (CMD27). 
     This is currently unsupported.
     Remedy: Copy the samples after attack_len+sustain_len to a new instrument
     and issue that instrument as one-shot on CMD27.
   - Oktalyzer has 3 arpeggio modes, need to implement Argpeggio 4 (CMD11) and
     Arpeggio 5 (CMD12, Vibrato?)
   - Slow Fade in / Slow Fade out: persistent effect that goes beyond current
     row

   ************************************************************************** */
#include "modplay_oktalyzer.h"
#include "modplay_tables.h"

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
LONG mod_ok_setupsamples( struct MOD *mod, UBYTE *buf, LONG bufsize, LONG song_bytes);
LONG mod_ok_loadpatterns( struct MOD *mod, UBYTE *buf, LONG bufsize, LONG *song_bytes );

#define OK_IDPOS   0
#define OK_IDPOS2  4
#define OK_CMODPOS 8
#define OK_FIRSTCHUNK OK_CMODPOS

#define OK_ID_OKTA 0x4F4B5441
#define OK_ID_SONG 0x534F4E47
#define OK_ID_CMOD 0x434D4F44
#define OK_ID_SAMP 0x53414D50
#define OK_ID_SPEE 0x53504545
#define OK_ID_SLEN 0x534C454E
#define OK_ID_PLEN 0x504C454E
#define OK_ID_PATT 0x50415454
#define OK_ID_PBOD 0x50424F44
#define OK_ID_SBOD 0x53424F44

#define OK_MAX_SAMPLES 36

struct ok_smpl {
	UBYTE name[20];
	ULONG length;
	UWORD repstart;
	UWORD replen;
	UBYTE unused1;
	UBYTE volume;
	UWORD unused2;
};

#if 1
#define ok_pertab OK_FineTune_Tab_
#else
static UWORD ok_pertab[] =
{
/*  C    C#     D    D#      E     F    F#     G    G#      A    A#     B */
 0x358,0x328,0x2FA,0x2D0, 0x2A6,0x280,0x25C,0x23A, 0x21A,0x1FC,0x1E0,0x1C5,
 0x1AC,0x194,0x17D,0x168, 0x153,0x140,0x12E,0x11D, 0x10D, 0xFE, 0xF0, 0xE2,
  0xD6, 0xCA, 0xBE, 0xB4,  0xAA, 0xA0, 0x97, 0x8F,  0x87, 0x7F, 0x78, 0x71
};
#endif


ULONG ok_getlong( UBYTE *buf, LONG offset )
{
	ULONG ret = (((ULONG)buf[offset]  )<<24) + \
		    (((ULONG)buf[offset+1])<<16) + \
		    (((ULONG)buf[offset+2])<<8)  + \
		    (((ULONG)buf[offset+3]));
	return ret;
}

LONG ok_getchunk( UBYTE *buf, LONG bufsize, LONG offset, ULONG  dest_id )
{
	ULONG id;
	LONG found = -1;

	while( offset < (bufsize-10) ) /* require header and at least 1 word left in buffer */
	{
		id = ok_getlong( buf, offset );
		if( id == dest_id )
		{
			found = offset;
			break;
		}
		offset += 8 + ok_getlong( buf, offset+4 );
	}

	return found;
}

/* check if we have an Oktalyzer mod */
/* in:  mod     - mod structure to store nchannels in (if successful), NULL is ok/checked 
        buf     - buffer pointer
        bufsize - number of bytes in buffer (>=1084 bytes)
   out: number of bytes needed for sample description, song and pattern data (minimum for mod_init() )
        numbers <=0 are error conditions 
*/
LONG mod_check_ok_internal( struct MOD *mod, UBYTE *buf, LONG bufsize )
{
 ULONG id,npat;
 ULONG nchan = 0; /* default: error */
 LONG sz,off,hunklen,smplen,songlen;

// bufsize = 2000; /* debug only */

 if( bufsize < 0x54E ) /* all Ok files seen so far have this header size (54E is including first PBOD header) */
	return MOD_ERR_BUFFERTOOSMALL;

 id = ok_getlong( buf, OK_IDPOS );
 if( id != OK_ID_OKTA )
 	return MOD_ERR_WRONGFORMAT;

 id = ok_getlong( buf, OK_IDPOS2 );
 if( id != OK_ID_SONG )
	 return MOD_ERR_WRONGFORMAT;

 /* technically, chunks can be at any position but every
    Oktalyzer mod I've seen has CMOD first */
 id = ok_getlong( buf, OK_CMODPOS );
 if( id != OK_ID_CMOD )
	 return MOD_ERR_WRONGFORMAT;

 /* channel mode: 4 words, if any word != 0, then we have a split (or multiplexed) channel */
 id = ok_getlong( buf, OK_CMODPOS+4 );
 if( id != 8 )
	 return MOD_ERR_WRONGFORMAT;

 /* at this point, we can be fairly certain to have an Oktalyzer mod */
 nchan = 4;
 id    = ok_getlong( buf, OK_CMODPOS+8  );
 if( id & 0xffff )
	nchan++;
 if( id & 0xffff0000 )
	nchan++;
 id   = ok_getlong( buf, OK_CMODPOS+12 );
 if( id & 0xffff )
	nchan++;
 if( id & 0xffff0000 )
	nchan++;

 /* assume random placement of subsequent chunks */
 /* typ: SAMP, SPEE, SLEN, PLEN, PATT, PBOD, SBOD */
 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SAMP );
 if( off < 0 )
	 return MOD_ERR_BUFFERTOOSMALL;

 /* parse SAMP chunk */
 hunklen = ok_getlong( buf, off+4 );
 if( (hunklen < sizeof( struct ok_smpl ) ) || 
     (hunklen > sizeof( struct ok_smpl ) * OK_MAX_SAMPLES ) )
	 return MOD_ERR_WRONGFORMAT;

 
 {
	struct ok_smpl *osmp;
	LONG dst = sizeof(struct ok_smpl ); 
	ULONG len;
#ifdef DEBUG
	ULONG rs,rl;
#endif

	smplen = 0;

	D(("SAMP chunk found at %d, len %d\n",off,hunklen));

	osmp = (struct ok_smpl*)(buf+off+8);

	while( dst <= hunklen )
	{
		len = ok_getlong( (UBYTE*)&osmp->length,0);
#ifdef DEBUG
		rs  = (((ULONG)(*(UBYTE*)&osmp->repstart))<<8) | *(1+(UBYTE*)&osmp->repstart);
		rl  = (((ULONG)(*(UBYTE*)&osmp->replen))<<8) | *(1+(UBYTE*)&osmp->replen);
#endif
		D(("sample %s len %d rs %d rl %d\n",osmp->name,len,rs,rl));

		if( len )
			smplen += len + 8; /* actual length + chunk header */

		/* next sample */
		dst += sizeof( struct ok_smpl );
		osmp++;
	}
 }
 
 /* number of patterns */
 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SLEN ); 
 if( off < 0 )
	 return MOD_ERR_BUFFERTOOSMALL;
 npat = ((ULONG)(buf[off+8])<<16) + (ULONG)(buf[off+9]);

 /* length of song */
 songlen = 0;
 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_PLEN ); 
 if( off >= 0 )
 {
	songlen = ((ULONG)(buf[off+8])<<16) + (ULONG)(buf[off+9]);
 }
 D(("Npat %d Songlen %d\n",npat,songlen));


 /* shall we fill something into the global struct ? */
 if( (mod) )
 {
	mod->modtype   = OK_ID_OKTA;
	mod->patlen    = 64; /* max length, individuals will be set upon actual loading */
	mod->nchannels = nchan;
	mod->npat      = npat;
	mod->songlen   = songlen;
	mod->mode      = MOD_MODE_OKT;
	mod->maxsamples= OK_MAX_SAMPLES; /* 31 */
	mod->filter    = 0;
 }

 /* if we are supplied with the whole file, size of "song data" is easy,
  * just find the first sample */
 sz = 0;
 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SBOD );
 if( off >= 0 )
 {
	sz = off;
	D(("Found first sample at %d, exact song size is %d (0x%x)\n",off,sz,sz ));
 }
 else
 { /* guess the size: patterns may have unequal lengths, 
      hence asssume "max length" patterns to be safe */

	/* expected at 0x546 */
	off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_PBOD );
	if( off >= 0 )
	{
		D(("First PBOD at %d\n",off));
		sz = off;
	}
	else
		sz = 0x546; /* "educated" guess */

	/* 64 rows*4 bytes per entry + 2 byte pat len + header */
	sz += npat*( nchan*64*4 + 2 + 8); 
	D(("estimated song size %d (0x%x)\n",sz,sz ));
 }

 return sz;
}



/* init Oktalyzer module (song data) 
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
	 mod_check_ok_internal()

*/
struct MOD *mod_ok_init(struct MOD *mod, UBYTE *buf, LONG bufsize, ULONG flags, LONG *fileoffset )
{
  LONG sz,i,*sng,err,nchan,off,spd;
  ULONG chanmap;
  struct MODChannel *chan,**channels;
  LONG song_bytes=0; /* totality of header and pattern data (i.e. offset of first SBOD) */

  sz = 0;

  do
  {
   err = 1;

   mod->flags |= MODPF_VBLANK; /* speed is always 50 Hz ticks, no CIA trickery */

   /* we arrive here with some good idea how many patterns to load (as ok_check_internal()
    * was called with the fresh mod struct */

   /* load/parse pattern data (needs npat,patlen,nchannels in MOD struct) */
   if( mod_ok_loadpatterns( mod, buf, bufsize, &song_bytes ) < 0 )
   	break;

   /* copy song */
   sng = (LONG*)mod_AllocVec( sizeof(LONG) * mod->songlen );
   if( !sng )
  	break;


   mod->song = sng;
   off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_PATT );
   if( off < 0 )
	break;

   for( i = 0 ; i < mod->songlen ; i++ )
   {
	*sng++ = buf[off+8+i];
   }

   if( mod_ok_setupsamples( mod, buf, bufsize, song_bytes ) < 0 )
	   break;

   /* after samples, set up channels */
   nchan = mod->nchannels;
   channels = (struct MODChannel **)mod_AllocVec( sizeof(struct MODChannel*) * nchan );
   if( !channels )
   	break;
   mod->channels = channels;

#if 0
   /* technically, chunks can be at any position but every
    Oktalyzer mod I've seen has CMOD first */
   chanmap = 0;
   shft = 0;
   id    = ok_getlong( buf, OK_CMODPOS+8  );
   if( id & 0xffff ) /* first channel split ? */
	

	nchan++;
 if( id & 0xffff0000 )
	nchan++;
 id   = ok_getlong( buf, OK_CMODPOS+12 );
 if( id & 0xffff )
	nchan++;
 if( id & 0xffff0000 )
	nchan++;
#endif
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

   mod->pertab.ptable_type = MODPERTAB_PERIOD; /* PT style periods */
   { /* classic PT period table */
	const UWORD *ptab = PT_FineTune_Tab_;
	mod->pertab.ptables[0] = (UWORD*)ptab;
	mod->pertab.ptable_idxmask = OK_FINETUNE_IDXMASK;
	mod->pertab.ptable_maxidx = 35;  /* maximum index for offset clamping */
        mod->pertab.ptable_minper = 113; /* PT: 113 */
        mod->pertab.ptable_maxper = 856; /* PT: 856 (with finetuning, actual max would be 907 but PT clamps the former) */
   }

   sz = song_bytes+8; /* after first SBOD header */
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

  spd = 6;
  off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SPEE );
  if( off >= 0 )
	  spd = buf[off+9];


  /* reinit: clear songpos,patpos,patdelay,skiptopos */
  mod->tick     = 0;
  mod->songpos  = 0;
  mod->patpos   = 0;
  mod->speed    = spd;
  mod->ciaspeed = 125;
  mod->patdelay = 0;
  mod->skiptopos= 0;

  mod->interval = 250000UL/(UWORD)(mod->ciaspeed); /* time increment in 10us units */
  *fileoffset   = sz; /* byte offset in file of the first sample */


  return mod;
}



LONG mod_ok_setupsamples( struct MOD *mod, UBYTE *buf, LONG bufsize, LONG song_bytes )
{
 LONG i;
 LONG off,hunklen,ns;
 ULONG len,rs,rl,fpos;
 struct ok_smpl *osmp;
 struct MODSample *smp,**ms;

 /* get sample information (+1 reserve pointer for "note cut" feature)*/
 ms = (struct MODSample **)mod_AllocVec( sizeof(struct MODSample *) * (OK_MAX_SAMPLES+1) );
 mod->samples = ms;
 if( !ms )
	 return MOD_ERR_MEMORY;

 fpos = song_bytes; /* first SBOD header (estimated) */

 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SAMP );
 if( off < 0 )
	 return MOD_ERR_BUFFERTOOSMALL;
 hunklen = ok_getlong( buf, off+4 );
 osmp = (struct ok_smpl*)(buf+off+8);

 for( i=0,ns=0 ; i < (hunklen>>5) ; i++ )
 {
	smp = NULL;
	len = ok_getlong( (UBYTE*)&osmp->length,0);
	if( len > 0 )
	{
		/* "len" is in byte units; rs,rl are in word units */
		ns++;
	
		rs  = (((ULONG)(*(UBYTE*)&osmp->repstart))<<8) | *(1+(UBYTE*)&osmp->repstart);
		rl  = (((ULONG)(*(UBYTE*)&osmp->replen))<<8) | *(1+(UBYTE*)&osmp->replen);

		smp = (struct MODSample *)mod_AllocVec( sizeof(struct MODSample) );
		if( !smp )
			break;
		smp->data      = NULL; /* no data yet */
		smp->length    = len & (~1); /* ignore trailing byte */
		smp->repstart  = rs<<1;
		smp->replen    = rl<<1;
		smp->finetune  = 0;
		smp->volume    = osmp->volume;
		smp->type      = MODSAM_TYPE_S8;
		smp->name      = (STRPTR)mod_AllocVec( PT_SAMPLE_NAMELEN + 1 ); /* 20 bytes like PT */
		smp->ext_data1 = smp->ext_data2 = smp->ext_data3 = smp->ext_data4 = 0;
		if( smp->name )
		{
			int j = PT_SAMPLE_NAMELEN;
			STRPTR s,d;
			s = (STRPTR)osmp->name;
			d = smp->name;
			while( j-- )
				*d++ = *s++;
			*d = 0;
		}

		if( off >= 0 )
			off = ok_getchunk( buf, bufsize, off, OK_ID_SBOD );

		D(("Sample %d len %d, assumed position %d (0x%x) actual position %d (0x%x)\n",i,len,fpos,fpos,off,off));

		if( off >= 0 )
		{
			smp->fileoffset = off+8;
			off   = off+8+ok_getlong(buf,off+4);
#ifdef DEBUG
			fpos  = off;
#else
			fpos += (len&(~1))+8;
#endif
		}
		else
		{
			smp->fileoffset = fpos;
			fpos += (len&(~1))+8;
		}

	}
	*ms++ = smp; /* this way leaves gaps in the sample array but we can keep the original indices */
	osmp++;
 }
 mod->nsamples = ns;
 D(("Total %d samples\n",ns));

 return 0;

}

#if 0
/* assume random placement of subsequent chunks */
 /* typ: SAMP, SPEE, SLEN, PLEN, PATT, PBOD, SBOD */
 off = ok_getchunk( buf, bufsize, OK_FIRSTCHUNK, OK_ID_SAMP );
 if( off < 0 )
	 return MOD_ERR_BUFFERTOOSMALL;

 /* parse SAMP chunk */
 hunklen = ok_getlong( buf, off+4 );
 if( (hunklen < sizeof( struct ok_smpl ) ) || 
     (hunklen > sizeof( struct ok_smpl ) * OK_MAX_SAMPLES ) )
	 return MOD_ERR_WRONGFORMAT;

 
 {
	struct ok_smpl *osmp;
	LONG dst = sizeof(struct ok_smpl ); 
	ULONG len,rs,rl;

	smplen = 0;

	D(("SAMP chunk found at %d, len %d\n",off,hunklen));

	osmp = (struct ok_smpl*)(buf+off+8);

	while( dst <= hunklen )
	{
		len = ok_getlong( (UBYTE*)&osmp->length,0);
		rs  = (((ULONG)(*(UBYTE*)&osmp->repstart))<<8) | *(1+(UBYTE*)&osmp->repstart);
		rl  = (((ULONG)(*(UBYTE*)&osmp->replen))<<8) | *(1+(UBYTE*)&osmp->replen);
		D(("sample %s len %d rs %d rl %d\n",osmp->name,len,rs,rl));

		if( len )
			smplen += len + 8; /* actual length + chunk header */

		/* next sample */
		dst += sizeof( struct ok_smpl );
		osmp++;
	}
 }
 
#if 0
struct ok_smpl {
        UBYTE name[20];
        ULONG length;
        UWORD repstart;
        UWORD replen;
        UBYTE unused1;
        UBYTE volume;
        UWORD unused2;
};      
#endif


   /* */
   /* */
   /* */
   /* */

   /* */
   /* */
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
#endif


/*
  internal: load/convert pattern data
  in:  mod - internal structure, pre-filled with npat, nchannels,patlen
       buf - input buffer (first byte of MOD)
  out: npatterns or <0 for failure
*/
LONG mod_ok_loadpatterns( struct MOD *mod, UBYTE *buf, LONG bufsize, LONG *song_bytes )
{
 int i,j;
 struct MODPattern **mp,*pat;//,*tmppat;
 struct MODPatternEntry *patdta;
 UBYTE *curbuf;
 ULONG patsz,entries,tmp,tfx,cl;
 LONG  off;
 ULONG rows;

 if( mod->npat < 1 )
 	return MOD_ERR_GENERAL;

 mp = (struct MODPattern**)mod_AllocVec( mod->npat * sizeof( struct MODPattern* ) );
 mod->patterns = mp;
 if( !mp )
 	return -1;

 for( i=0 ; i < mod->npat ; i++ )
 	mp[i] = NULL;

 off = OK_FIRSTCHUNK;
 for( i=0 ; i < mod->npat ; i++ )
 {
	off = ok_getchunk( buf, bufsize, off, OK_ID_PBOD );
	if( off < 0 )
		return -1;
	rows    = buf[off+9]; /* length 1..64 */
	entries = mod->nchannels * rows;
	patsz   = entries * sizeof(struct MODPatternEntry);

//	D(("Pattern %d at %d len %d\n",i,off,rows));

	/* header (struct MODPattern) plus Body (struct MODPatternEntry) */
	pat   = mod_AllocVec( patsz + sizeof(struct MODPattern) ); 
	*mp++ = pat;

	if( !pat )
  		return -1;

	pat->rowoff    = mod->nchannels;
	pat->nchannels = mod->nchannels;
	pat->nrows     = rows;

	patdta = (struct MODPatternEntry *)(pat+1);
	pat->patdata = patdta;

	DP(("\n------- pattern %ld -------",(long)i));

	curbuf = buf+off+10; /* skip chunk header and nrows */
	cl = 0;
	for( j = 0 ; j < entries ; j++ )
	{
	   tmp = curbuf[0];
	   patdta->period = 0;
	   patdta->sample = 0;
	   if( tmp )
	   {
		   patdta->period = ok_pertab[tmp-1];
		   patdta->sample = curbuf[1]+1;
	   }
	   if( curbuf[1] )
		   patdta->sample = curbuf[1]+1;
	   patdta->volume = PAT_NO_VOLUME;

	   tmp = curbuf[2];
	   tfx = curbuf[3];
	   /* translate FX to existing FX code, where applicable */

	   switch( tmp )
	   {
		case 0:		/* nothing to do */
			break;
		case 1:
			tmp = FX_PT_20_SLIDEDOWN;
			break;
		case 2:
			tmp = FX_PT_10_SLIDEUP;
			break;
		case 10: /* ARP3: L,N,H -> PT compatible */
			tmp = FX_PT_00_ARP;
			break;
		case 11: /* ARP4: N,H,N,L */
			tmp = FX_OK_11_ARP4;
			break;
		case 12: /* ARP5: H,H,N */
			tmp = FX_OK_12_ARP5;
			break;		
		case 13:
			tmp = FX_OK_13_NOTEDOWN;
			break;
		case 15:
			tmp = FX_PT_E0_FILTER;
			break;
		case 17:
			tmp = FX_OK_17_NOTEUP;
			break;
		case 21:
			tmp = FX_OK_21_NOTEDOWNSLOW;
			break;
		case 25: /* Jump, same as PT */
			tmp = FX_PT_B0_JUMPOFFSET;
			break;
		case 28: /* Speed, same as PT */
			tmp = FX_PT_F0_SPEED;
			break;
		case 30:
			tmp = FX_OK_30_NOTEUPSLOW;
			break;
		case 31: /* volume is a little more complicated, 3 modes */

			/* volset */
			if( tfx <= 0x40 )
			{
				tmp = FX_PT_C0_VOLSET;
				break;
			}

			/* volslide */
			if( tfx <= 0x50 ) /* down */
			{
				tmp  = FX_PT_A0_VOLSLIDE;
				tfx  = (tfx==0x50) ? 0xF : tfx-0x40; /* 0x41...0x50 -> 0x1...0xF */
				break;
			}
			if( tfx <= 0x60 ) /* up */
			{
				tmp  = FX_PT_A0_VOLSLIDE;
				tfx  = (tfx==0x60) ? 0xF0 : (tfx-0x50)<<4; /* 0x51...0x60 -> 0x10...0xF0 */
				break;
			}

			/* Slow Fade - redirect to PT fine volume */
			if( tfx <= 0x70 ) /* fade out */
			{
				tmp = FX_PT_EB_FINEVOLDOWN;
				tfx = (tfx==0x70) ? 0xF : tfx-0x60;
			}

			if( tfx <= 0x80 ) /* fade in */
			{
				tmp = FX_PT_EA_FINEVOLUP;
				tfx = (tfx==0x80) ? 0xF : tfx-0x60;
			}

			tmp = 0; /* illegal, discard */
			tfx = 0;
			break;
		case 27: /* release, unsupported by now (FIXME) */
		default:	/* unknown/unsupported FX: ignore */
			tmp=0;
			tfx=0;
			break;

	   }

	   patdta->fx = tmp;
	   patdta->fxdat = tfx;


	/* debug sample indices */
#if 0
	   if( (j % mod->nchannels) == 0 )
		printf("\n");
	   if( (j % (pat->nrows * pat->nchannels)) == 0)
		printf("--\n");
	   printf("%2x ",patdta->sample);
#endif
#if 1
	   if( !(cl) )
	   {
		DP(("\n"));
	   }
	   else
	   {
		DP(("|"));
	   }
	   {
		DP(("%3ld %02lx %2lx%02lx",\
		   (long)patdta->period,\
		   (long)patdta->sample,\
		   (long)patdta->fx, \
		   (unsigned long)patdta->fxdat ) );
	   }
#endif

	   cl++;
	   if( cl >= mod->nchannels )
		   cl = 0;

	   patdta++;
	   curbuf+=4;
	}

	off += ok_getlong( buf, off+4 ) + 8;
 }

 *song_bytes = off;

 D(("\n Patterns done at file offset %d (0x%x).\n",off,off));

 return mod->npat;
}


