
#ifdef AMIGA


#ifdef __GNUC__
/*
 * m68000 compatible mulsi3, no library call
 */
int __mulsi3(int a, int b )
{
        int res;
        __asm__ ("\
                move.l  %1,d2;\
                move.l  %2,d3;\
                swap    d2;\
                swap    d3;\
                mulu.w  %2,d2;\
                mulu.w  %1,d3;\
                mulu.w  %2,%1;\
                add.w   d2,d3;\
                swap    d3;\
                clr.w   d3;\
                add.l   d3,%1;\
                move.l  %1,%0;\
                " : "=&d" (res) : "d" (a), "d" (b) : "d2", "d3" );
        return res;
}

unsigned int __udivsi3( unsigned int a, unsigned int b );

int __divsi3( int a, int b )
{
 int sgn = 0;
 int res;

 if( a < 0 )
 {
	a   = -a;
	sgn = -1; 
 }
 if( b < 0 )
 {
	 b = -b;
	 sgn = sgn ^ (-1);
 }

 res = ( __udivsi3(a,b) ^ sgn ) - sgn;

 return res;
}

int __modsi3( int a, int b )
{
	/* a % b = a - (a/b)*b */
	int tmp = __divsi3(a,b);
	tmp = __mulsi3( tmp, b);

	return a-tmp;
}


/* res = a / b */
unsigned int __udivsi3( unsigned int a, unsigned int b )
{
	unsigned int res;

	/* curious detail with "m68k-amigaos-gcc (GCC) 6.5.0b 221101201233":
	 * if the compare is b<0x10000 and the two asm blocks are swapped, then the
	 * compiler will generate wrong code
	 * */

	if( b >= 0x10000 )
	{
		/* divisor >0x10000, scale down for approximate quotient by 16 bit divisor */
        __asm__ volatile ("\
                move.l  %1,%0;      /* save dividend   */\
		move.l	%2,d3;      /* save divisor    */\
.L1:		lsr.l	#1,%0;      /* dividend>>1     */\
		lsr.l	#1,%2;      /* divisor>>1      */\
		cmp.l	#0xffff,%2; /* still >16 bit ? */\
		bcc.s	.L1;\
		divu.w	%2,%0;      /* approximate quotient */\
		and.l	#0xffff,%0; /* ignore remainder */\
		/* now: get product of approximate quotient and actual dividend */\
		move.l	d3,%2;      /* 32 bit divisor   */\
		mulu.w  %0,%2;      /* low part divisor * approx. quotient */\
		clr.w   d3;         /* clear low part */\
		swap	d3;         /* get high part -> lower 16 bit */\
		mulu.w	%0,d3;      /* high part divisor * approx. quotient */\
		swap	d3;         /* move high part in upper 16 bit again */\
		tst.w	d3;         /* if this is not 0, then the result was 33 bit */\
		bne.s	L2;        /* estimate too big, subtract 1 below */\
		add.l	%2,d3;	    /* full intermediate */\
		bcs.s   L2;        /* overflow: estimate too big, subtract 1 */\
		cmp.l	%1,d3;\
		bls	L3;\
L2:		subq.l	#1,%0;\
L3:\
                " : "=&d" (res) : "d" (a), "d" (b) : "d3" );
	}
	else
	{
		/* divide by a 16 bit number: easy peasy */
		/* %1 = dividend 32 bit */
		/* %2 = divisor 16 bit */
	        __asm__ volatile ("\
                move.l  %1,d3; /* dividend */\
		clr.w   d3;    /* clear out lower 16 bits */\
                swap    d3;    /* upper word dividend in lower word d3 */\
		divu.w  %2,d3; /* high quotient + (high rest << 16) */\
		move.l	d3,%0; /* high quotient + (high rest << 16) */\
		swap	%0;    /* high quotient in upper word, garbage lower word (rest) */\
		move.w	%1,d3; /* low divident + (high rest << 16) */\
		divu.w	%2,d3; /* low quotient + (rest<<16) */\
		move.w	d3,%0; /* low quotient + (high quotient<<16) */\
                " : "=&d" (res) : "d" (a), "d" (b) : "d3" );
	}
       	return res;
}

#endif /* __GNUC__ */
#endif /* AMIGA */

