; File:   supporta.asm
;
; Author: Henryk Richter <henryk.richter@gmx.net>
;
; Purpose:
;  These are custom implementations of multiply/divide functions
;  that are referenced by SAS/C with 68000 code and 32 bit mul/div.
;  While the SAS library functions work fine, this implementation was
;  done to build binaries without linking startup code and/or SAS libraries.
;
;

*******************************************************************************
*
* __CXD33 is signed divide 
*
*******************************************************************************
	xdef	__CXD33
__CXD33:
	move.l	d2,-(sp)		;

	moveq	#0,d2
	tst.l	d0			; dividend
	bge.s	.pos_dividend
	neg.l	d0
	not.l	d2
.pos_dividend:
	tst.l	d1			; dividend
	bge.s	.pos_divisor
	neg.l	d1
	not.l	d2
.pos_divisor:
	;d2 == 0 if no sign change is required, 0xffffffff otherwise

	bsr.s	__CXD22

	eor.l	d2,d0			; apply sign change (if necessary)
	sub.l	d2,d0

	move.l	(sp)+,d2
        rts 

*******************************************************************************
*
* __CXD22 is unsigned divide 
*
*******************************************************************************
	xdef	__CXD22
__CXD22:
	;D0: dividend (unsigned)
	;D1: divisor (unsigned)
	cmp.l	#$10000,d1	;divisor is 16 bit ?
	bhi.s	.long_div32

	movem.l	d2-d3,-(sp)	;we could save some stack by using A0/A1

	;/* divide by a 16 bit number: easy peasy */
	;/* D0 = dividend 32 bit */
	;/* D1 = divisor 16 bit */
	move.l	d0,d3	;get dividend
	clr.w	d3	;clear out lower 16 bits (calculated in second step)
	move.l	d0,d2	;save dividend
	swap	d3	;upper word dividend in lower word d3
	divu.w	d1,d3	;high quotient + (high rest << 16)
	move.l	d3,d0	;high quotient + (high rest << 16)
	swap	d0	;(high quotient<<16) + garbage lower word (rest)
	move.w	d2,d3	;low divident + (high rest << 16)
	divu.w	d1,d3	;low quotient + (rest<<16)
	move.w	d3,d0	;low quotient + (high quotient<<16)

	movem.l	(sp)+,d2-d3
	rts

.long_div32:
	movem.l	d2-d3,-(sp)
	move.l	d0,d2	;save dividend
	move.l	d1,d3	;save divisor
.scale_longdiv:
	lsr.l	#1,d0
	lsr.l	#1,d1
	cmp.l	#$ffff,d1      ; divisor still >16 bit ? 
	bcc.s	.scale_longdiv

	divu.w	d1,d0	  ; approximate quotient in D0
	and.l	#$ffff,d0 ; ignore remainder
	
	;now: get product of approximate quotient and actual dividend
	move.l	d3,d1	  ; get divisor back
	mulu.w  d0,d1	  ; low part divisor * approx. quotient
	clr.w	d3	  ; clear lower part divisor
	swap	d3	  ; get high part -> lower 16 bit
	mulu.w	d0,d3	  ; high part divisor * approx. quotient
	swap	d3	  ; move high part in upper 16 bit again
	tst.w	d3	  ; if this is not 0, then the result was 33 bit
	bne.s	.adjust_min1 ; estimate too big, subtract 1 below
	add.l	d1,d3	     ; full intermediate
	bcs.s	.adjust_min1 ; overflow: estimate too big, subtract 1
	cmp.l	d2,d3	     ; larger than original dividend ?
	bls.s	.no_adjust   ; no, then don't adjust
.adjust_min1:
	subq.l	#1,d0
.no_adjust:
	movem.l	(sp)+,d2-d3
	rts

*******************************************************************************
*
* __CXM33 is signed multiply 
* __CXM22 is unsigned multiply
*
*******************************************************************************

        xdef    __CXM33                 ; Signed multiply
        xdef    __CXM22                 ; Unsigned multiply
__CXM22:
__CXM33:
	movem.l	d2-d3,-(sp)
	move.l  d0,d2
	move.l  d1,d3
	swap    d2
	swap    d3
	mulu.w  d1,d2
	mulu.w  d0,d3
	mulu.w  d1,d0
	add.w   d2,d3
	swap    d3
	clr.w   d3
	add.l   d3,d0
	movem.l	(sp)+,d2-d3
        rts




