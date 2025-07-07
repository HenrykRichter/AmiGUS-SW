/*
  debug.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Debugging Macros

  requires -DDEBUG (gcc) or DEFINE DEBUG (sasc)
  in compiler cmdline or before #include

*/
#ifndef _INC_DEBUG_H
#define _INC_DEBUG_H

#ifdef DEBUG
extern void KPrintF(char *, ...), KGetChar(void);

#define D(_x_) do { KPrintF("%s:%ld:",__FILE__,__LINE__); KPrintF _x_; } while(0)

#else  /* DEBUG */

#define D(x)

#endif /* DEBUG */


#endif /* _INC_DEBUG_H */
