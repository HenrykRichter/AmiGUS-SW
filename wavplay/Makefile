#
## use with: make 
#
# path to the includes
PREFIX   = /opt/amiga/m68k-amigaos/ndk-include/
#
# build for AmigaOS (1) or other platforms (0)
AMIGA    = 1
# compiler_vcc = 1 will trigger VBCC, else GCC for Amiga builds
# note: GCC builds assume bebbo version for asm files, change -Fhunk to -Faout for gcc2.95
compiler_vcc = 0

DEFINES  = 
ifeq ($(AMIGA),1)
DEFINES += -DUSE_AMIGADOS
TARGET = aguswav 
TARGET2  = aguswtab
else
DEFINES += -DLITTLE_ENDIAN
TARGET   = aguswav
TARGET2  = aguswtab
endif

# tools
RM	= rm -f
ifeq ($(AMIGA),1)
ifeq ($(compiler_vcc),1)
# VBCC (use explicit vlink line for LINK= if complaints about R_PC happen)
CC  = vc
# -Rshort -Bstatic
LINKEXE = vlink -bamigahunk -Cvbcc -mrel -s -x
# -sc
CFLAGS  = -Os -+ -c99 -cpu=68000 -Iinclude/vbcc -Iinclude/ -I. $(DEFINES)
VASM    = vasmm68k_mot -quiet -I$(PREFIX)
LDFLAGS =
LINKEXE = $(CC)
else
CC	= m68k-amigaos-gcc 
CFLAGS	= -I. -Iinclude/ -Wall -O2 -fomit-frame-pointer -msmall-code $(DEFINES) 
CFLAGS  += -Wno-strict-aliasing
#CFLAGS += -m68020 -m68881
LDFLAGS	= -s -noixemul 
VASM    = vasmm68k_mot -quiet -I$(PREFIX)
LINKEXE = $(CC)
endif
else
CC      = gcc
CFLAGS  = -O2 -Wall $(DEFINES) -Iinclude/ 
LDFLAGS =
VASM    = vasmm68k_mot -quiet -I$(PREFIX)
endif

OBJS	= wav_load.o wav_convert16.o wav_convert24.o stringutils.o
ifeq ($(AMIGA),1)
OBJS   += amigus_streamout.o amigus_wtableout.o
endif
OBJST1 = main.o $(OBJS)
OBJST2 = wmain.o $(OBJS)


DEPS1  = $(OBJST1) wav_load.h amigus_streamout.h
DEPS2  = $(OBJST2) wav_load.h amigus_streamout.h

HEADERS = 

all: $(TARGET) $(TARGET2) 

aguswav: $(DEPS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJST1)

aguswtab: $(DEPS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJST2)


clean:
	-$(RM) $(TARGET) $(TARGET2) $(OBJS) main.o wmain.o

%.o:	%.s
	$(VASM) -Fhunk -o $@ $<


# dependencies

