
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

#if 0
int __udivsi3(int a, int b )
{
	return a;
}
#endif

#endif
#endif

