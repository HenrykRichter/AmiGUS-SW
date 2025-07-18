; patternscroll_convert.asm
; Henryk Richter
; 
;
;struct MODPatternEntry
;{       WORD  period; /* period in the mod's base unit (perhaps note in S3M notation) or PAT_NOTE_OFF */
;        WORD  sample; /* sample index    */
;        WORD  fx;     /* FX_xx_HH_Title  */
;        UBYTE fxdat;  /* argument for FX */
;        UBYTE volume; /* S3M/FT2: specific volume, apart from FX (1...65 -> vol=0..64 , 0x00=no_volume_chane) */
;};
	rsreset
pe_Period	rs.w	1
pe_Sample	rs.w	1
pe_FX		rs.w	1
pe_FXdat	rs.b	1

FX_PT_BASE		EQU	1
FX_PT_D0_JUMPROW	EQU	FX_PT_BASE+$D0
FX_PT_F0_SPEED		EQU	FX_PT_BASE+$F0
FX_PT_MAX		EQU	FX_PT_F0_SPEED
FX_S3M_BASE		EQU	FX_PT_MAX+$10
FX_OK_BASE		EQU	FX_S3M_BASE+$50

	XDEF	_Pattscroll_ConvertFunc

;        APTR    PI_Convert;             /* Converts Note (a0) */
;                                        /* to Period (D0),Samplenumber (D1), */
;                                        /* Commandstring (D2) and Argument (D3) */
_Pattscroll_ConvertFunc:
	moveq	#0,d3
	moveq	#0,d2
	move.b	6(a0),d3
	move.w	4(a0),d2
	beq.s	.nocmd			;no command, all good

	cmp.w	#FX_PT_F0_SPEED,d2
	bhi.s	.no_PT_CMD
	beq.s	.PT_Regular_CMD

	cmp.w	#FX_PT_D0_JUMPROW,d2	;E0 commands?
	bhi.s	.no_PT_Regulars
.PT_Regular_CMD:
	lsr.w	#4,d2			;eliminates FX_PT_BASE
	bra.s	.nocmd
.no_PT_Regulars:
	subq	#FX_PT_BASE,d2  ;D2 = E0,E1,...,EF
	lsl.b	#4,d2		;(1...F)<<4
	or.b	d2,d3		;
	moveq	#$E,d2
	bra.s	.nocmd
	; Commands not in PT
.no_PT_CMD:
	cmp.w	#FX_OK_BASE,d2
	blo.s	.s3m_cmd
	sub.w	#FX_OK_BASE,d2
	bra.s	.nocmd
.s3m_cmd:
	sub	#FX_S3M_BASE,d2
	lsr.w	#4,d2
.nocmd:
	moveq	#0,d0
	moveq	#0,d1
	move.w	(a0),d0
	move.w	2(a0),d1
	rts


