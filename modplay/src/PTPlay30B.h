/*
  PTPlay30B.h

  author: Henryk Richter <henryk.richter@gmx.net>

  purpose: Interface to Protracker Playroutine

*/
#ifndef _INC_PTPLAY_H
#define _INC_PTPLAY_H

#include "compiler.h"

ASM SAVEDS void PT_StartPlay( ASMR(a0) unsigned char *ptmod ASMREG(a0) );
ASM SAVEDS void PT_StopPlay( void );

/* set FX channel (1-4), 0 to disable */
ASM SAVEDS void PT_SetFXChannel( ASMR(d0) unsigned long fxchan ASMREG(d0) );
/* play note on FX channel */
ASM SAVEDS void PT_SetFX( ASMR(d0) unsigned long fx ASMREG(d0) );

/* test/debug: manual init, manual call per tick */
ASM SAVEDS void PT_InitMusic( ASMR(a0) unsigned char *ptmod ASMREG(a0) );
ASM SAVEDS void PT_PlayTick( void );
ASM SAVEDS void PT_SetDryRun( ASMR(d0) unsigned long dry ASMREG(d0) );

/* dry-run mode: get channel state (contents written to shadowed DFF0A0-DFF0DF) */
ASM SAVEDS unsigned char* PT_DryChannelState( void );
ASM SAVEDS unsigned long  PT_DryDMAState( void );
ASM SAVEDS unsigned long  PT_DryCounterSpeedPposSpos( void );
ASM SAVEDS unsigned char* PT_DryPatternEntry(void);

#endif /* _INC_PTPLAY_H */
