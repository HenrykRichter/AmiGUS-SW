;
;  AmiGUS register definitions
;
;  AmiGUS (C) 2024-2025 Oliver Achten
;  
;  blame Henryk Richter <henryk.richter@gmx.net> for this file
;
        ifnd _INC_AMIGUS_H
_INC_AMIGUS_H   EQU     1

	IFND    EXEC_TYPES_I	; change to "ifne 0" for proper operation (includes exec/types.i then)
	;from exec/types.i
STRUCTURE   MACRO               ; structure name, initial offset
\1          EQU     0
SOFFSET     SET     \2
            ENDM

FPTR        MACRO               ; function pointer (32 bits - all bits valid)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

BOOL        MACRO               ; boolean (16 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

BYTE        MACRO               ; byte (8 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+1
            ENDM

UBYTE       MACRO               ; unsigned byte (8 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+1
            ENDM

WORD        MACRO               ; word (16 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

UWORD       MACRO               ; unsigned word (16 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

SHORT       MACRO               ; obsolete - use WORD
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

USHORT      MACRO               ; obsolete - use UWORD
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

LONG        MACRO               ; long (32 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

ULONG       MACRO               ; unsigned long (32 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

APTR        MACRO               ; untyped pointer (32 bits - all bits valid)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

CPTR        MACRO               ; obsolete
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

RPTR        MACRO               ; unsigned relative pointer (16 bits)
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+2
            ENDM

LABEL       MACRO               ; Define a label without bumping the offset
\1          EQU     SOFFSET
            ENDM

STRUCT      MACRO               ; Define a sub-structure
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+\2
            ENDM
	else
        include "exec/types.i"
	endc

;-------------- AmiGUS Zorro IDs ----------
AMIGUS_MANUFACTURER_ID  EQU     2782

AMIGUS_MAIN_PRODUCT_ID  EQU     16
AMIGUS_HAGEN_PRODUCT_ID EQU     17
AMIGUS_CODEC_PRODUCT_ID EQU     18

;--- AmiGUS Main Register Definitions (relative to board address) ---
        STRUCTURE AmiGUS_Main_Regs,0
        UWORD AGMR_INTC         ;
        UWORD AGMR_INTE         ;
        UWORD AGMR_SMPL_FMT     ;
        UWORD AGMR_SMPL_RATE    ;
        UWORD AGMR_FIFO_RES     ;
        UWORD AGMR_FIFO_WTMK    ;
        ULONG AGMR_FIFO_DATA    ; 
        UWORD AGMR_FIFO_USE     ;
        UWORD AGMR_FIFO_VOL_L   ;
        UWORD AGMR_FIFO_VOL_R   ;
        STRUCT AGMR_unused_1,10 ;

        UWORD AGMR_SPI_ADDRESS    ;
        UWORD AGMR_SPI_WRITE_BYTE ;
        UWORD AGMR_SPI_WTRIG      ;
        UWORD AGMR_SPI_RTRIG      ;
        UWORD AGMR_SPI_READ_BYTE  ;
        UWORD AGMR_SPI_STATUS     ;
        STRUCT AGMR_unused_2,4    ;
        
        ; TODO: mixer registers

;--- AmiGUS Hagen Register Definitions (relative to board address) ---
        STRUCTURE AmiGUS_Hagen_Regs,0
        UWORD AGHR_INTC0        ;
        UWORD AGHR_INTC1        ;
        UWORD AGHR_INTC2        ;
        UWORD AGHR_INTC3        ;
        UWORD AGHR_INTE0        ;
        UWORD AGHR_INTE1        ;
        UWORD AGHR_INTE2        ;
        UWORD AGHR_INTE3        ;

        ULONG  AGHR_WDATA       ; $10/$12
        ULONG  AGHR_WADDR       ; $14/$16
        UWORD  AGHR_WRESET      ; $18
        STRUCT AGHR_unused1,4   ; $1a..$1d
        UWORD  AGHR_VOICE_BNK   ; $1e
        UWORD  AGHR_VOICE_CTRL  ; $20
        ULONG  AGHR_VOICE_PSTRT ; $22/$24
        ULONG  AGHR_VOICE_PLOOP ; $26/$28
        ULONG  AGHR_VOICE_PEND  ; $2a/$2c
        ULONG  AGHR_VOICE_RATE  ; 0x2e / 0x30 
        UWORD  AGHR_VOICE_VOLUMEL    ;0x32
        UWORD  AGHR_VOICE_VOLUMER    ;0x34
        UWORD  AGHR_VOICE_EN_ATTACK  ;0x36
        UWORD  AGHR_VOICE_EN_DECAY   ;0x38
        UWORD  AGHR_VOICE_EN_SUSTAIN ;0x3A
        UWORD  AGHR_VOICE_EN_RELEASE ;0x3C
        UWORD  AGHR_unused2          ; 3E
        UWORD  AGHR_GLOBAL_VOLUMEL   ;0x40
        UWORD  AGHR_GLOBAL_VOLUMER   ;0x42
        STRUCT AGHR_unused3,12       ;0x44-0x4F

        STRUCT AGHR_pad,$A0          ;0x50-0xEF blank space

        ; timer stuff
        UWORD  AGHR_HT_CONTROL       ; 0xF0 
        ULONG  AGHR_HT_RELOAD        ; 0xF2/F4 
        ULONG  AGHR_HT_READ          ; 0xF6/F8 


MAIN_INTC                	EQU	$00
MAIN_INTE                	EQU	$02
MAIN_SMPL_FMT                	EQU	$04
MAIN_SMPL_RATE                	EQU	$06
MAIN_FIFO_RES                	EQU	$08
MAIN_FIFO_WTMK                	EQU	$0a
MAIN_FIFO_DATA                	EQU	$0c
MAIN_FIFO_USE                	EQU	$10

; AmiGUS Main Interrupt Flags 

INT_FLG_FIFO_EMPTY        	EQU	$1
INT_FLG_FIFO_FULL        	EQU	$2
INT_FLG_FIFO_WTMK        	EQU	$4
INT_FLG_SPI_FIN                	EQU	$8
INT_FLG_MASK_SET        	EQU	$8000

; AmiGUS Main Sample Formats 

SMPL_FMT_MONO_8BIT                	EQU	$0
SMPL_FMT_STEREO_8BIT                	EQU	$1
SMPL_FMT_MONO_16BIT                	EQU	$2
SMPL_FMT_STEREO_16BIT                	EQU	$3
SMPL_FMT_MONO_24BIT                	EQU	$4
SMPL_FMT_STEREO_24BIT                	EQU	$5

SMPL_FMT_MONO_8BIT_SWP                	EQU	$8
SMPL_FMT_STEREO_8BIT_SWP        	EQU	$9
SMPL_FMT_MONO_16BIT_SWP                	EQU	$a
SMPL_FMT_STEREO_16BIT_SWP        	EQU	$b
SMPL_FMT_MONO_24BIT_SWP                	EQU	$c
SMPL_FMT_STEREO_24BIT_SWP        	EQU	$d

SMPL_FMT_LITTLE_ENDIAN                	EQU	$10

; AmiGUS Main Sample Rates 

SMPL_RATE_8000                	EQU	$0
SMPL_RATE_11025                	EQU	$1
SMPL_RATE_16000                	EQU	$2
SMPL_RATE_22050                	EQU	$3
SMPL_RATE_24000                	EQU	$4
SMPL_RATE_32000                	EQU	$5
SMPL_RATE_44100                	EQU	$6
SMPL_RATE_48000                	EQU	$7
SMPL_RATE_96000                	EQU	$8


; Memory Buffers */
FIFO_SIZE    EQU                    4096                        ; FIFO has 4096 * 16-bit words 
FIFO_WTMK    EQU                    2048                        ; FIFO watermark (default, the watermark is a variable) 


; AmiGUS Hagen Register Definitions 
HAGEN_INTC0                     	EQU	$00
HAGEN_INTC1                     	EQU	$02
HAGEN_INTC2                     	EQU	$04
HAGEN_INTC3                     	EQU	$06
HAGEN_INTE0                     	EQU	$08
HAGEN_INTE1                     	EQU	$0a
HAGEN_INTE2                     	EQU	$0c
HAGEN_INTE3                     	EQU	$0e

HAGEN_WDATAH            	EQU	$10
HAGEN_WDATAL            	EQU	$12
HAGEN_WADDRH            	EQU	$14
HAGEN_WADDRL            	EQU	$16
HAGEN_WRESET            	EQU	$18

HAGEN_VOICE_BNK         	EQU	$1e
HAGEN_VOICE_CTRL        	EQU	$20
HAGEN_VOICE_PSTRTH      	EQU	$22
HAGEN_VOICE_PSTRTL      	EQU	$24
HAGEN_VOICE_PLOOPH      	EQU	$26
HAGEN_VOICE_PLOOPL      	EQU	$28
HAGEN_VOICE_PENDH       	EQU	$2a
HAGEN_VOICE_PENDL       	EQU	$2c
HAGEN_VOICE_RATEH       	EQU	$2e
HAGEN_VOICE_RATEL       	EQU	$30
HAGEN_VOICE_VOLUMEL     	EQU	$32
HAGEN_VOICE_VOLUMER     	EQU	$34

HAGEN_GLOBAL_VOLUMEL    	EQU	$40
HAGEN_GLOBAL_VOLUMER    	EQU	$42

; Hagen control flags 

HAGEN_CTRLB_16BIT               EQU     0        ; R/W 0=8bit 1=16 Bit 
HAGEN_CTRLB_LOOP                EQU     1        ; R/W 0=one-shot, 1=loop 
HAGEN_CTRLB_INTERPOL            EQU     2        ; R/W 0=nearest neighbor, 1=linear interpol 
HAGEN_CTRLB_LITTLE_ENDIAN       EQU     3        ; W   0=Big Endian, 1=Little Endian 
HAGEN_CTRLB_ENVELOPE_MODULATE   EQU     5        ; W   0=No Envelope Modulation, 1=Use Envelope Modulation 
HAGEN_CTRLB_ENVELOPE_KEYON      EQU    14        ; W   0=Envelope Key off, 1=On 
HAGEN_CTRLB_PBSTART             EQU    15        ; R/W 0=Playback off, 1=Playback On 

HAGEN_CTRLF_16BIT                	EQU	$0001        ; R/W 0=8bit 1=16 Bit 
HAGEN_CTRLF_LOOP                	EQU	$0002        ; R/W 0=one-shot, 1=loop 
HAGEN_CTRLF_INTERPOL                	EQU	$0004        ; R/W 0=nearest neighbor, 1=linear interpol 
HAGEN_CTRLF_LITTLE_ENDIAN        	EQU	$0008        ; W   0=Big Endian, 1=Little Endian 
HAGEN_CTRLF_ENVELOPE_MODULATE        	EQU	$0020        ; W   0=No Envelope Modulation, 1=Use Envelope Modulation 
HAGEN_CTRLF_ENVELOPE_KEYON        	EQU	$4000        ; W   0=Envelope Key off, 1=On 
HAGEN_CTRLF_PBSTART                	EQU	$8000        ; R/W 0=Playback off, 1=Playback On 

        endc ;_INC_AMIGUS_H
