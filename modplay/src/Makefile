# buildAMIGA = "1" enables cross compiling for Amiga binaries
buildAMIGA = 0
# buildAGUS = "1" includes AmiGUS player in "modplay" instead of the WAV renderer core
buildAGUS = 0

ifeq ($(buildAMIGA),1)

CC = m68k-amigaos-gcc 
CPU = -m68020
WARN = -Wall
OPT = -noixemul -mregparm=4 -fomit-frame-pointer -msoft-float -O2 -Iinclude/
#DEBUG = -g
DEF  = -DAMIGA
DEF += -DUSE_INLINE_STDARG
CFLAGS = $(CPU) $(WARN) $(OPT) $(DEBUG) $(DEF) -noixemul
LIBS = -lm

else

CC  = gcc
OPT = -O2 -g -I.
WARN = -Wall
CFLAGS = $(WARN) $(OPT)
LIBS = -lm

endif


OBJS1 = obj/modplay.o obj/modplay_protracker.o \
        obj/modplay_s3m.o obj/support.o

OBJS = obj/modplay_main.o $(OBJS1)
ifeq ($(buildAMIGA),1)
ifeq ($(buildAGUS),1)
OBJS += obj/modrender_amigus.o
else
OBJS += obj/modrender_stream.o 
endif
else
OBJS += obj/modrender_stream.o 
endif

ifeq ($(buildAMIGA),1)
all: ../bin/modplay_68k AGUS_Mod
else
all: modplay
endif

# EP plugin, AmiGUS and m68k only (i.e. requires buildAMIGA)
AGUS_Mod: obj/epheader.o obj/modplay_ep.o obj/modrender_amigus.o $(OBJS1)
	$(CC) $(CFLAGS) -o $@ obj/epheader.o obj/modplay_ep.o obj/modrender_amigus.o $(OBJS1) -nostdlib

modplay: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

../bin/modplay_68k: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

ifeq ($(buildAMIGA),1)
clean:	
	-rm -f obj/*.o $(OBJS) obj/modplay_ep.o obj/epheader.o
	-rm -f ../bin/modplay_68k ../bin/modplay modplay AGUS_Mod 
else
clean:	
	-rm -f obj/*.o $(OBJS1)
	-rm -f modplay
endif

obj/support.o: support.c
	$(CC) $(CFLAGS) support.c -c -o $@

obj/main.o: main.c global.h audio.h ptplay.h
	$(CC) $(CFLAGS) main.c -c -o $@

obj/epheader.o: epheader.c modplay.h fileio.h
	$(CC) $(CFLAGS) -Wno-unused-const-variable epheader.c -c -o $@

obj/modplay_ep.o: modplay_ep.c modplay.h fileio.h
	$(CC) $(CFLAGS) modplay_ep.c -c -o $@

obj/modplay_main.o: modplay_main.c modplay.h fileio.h
	$(CC) $(CFLAGS) modplay_main.c -c -o $@

obj/modplay_protracker.o: modplay_protracker.c modplay_protracker.h modplay.h
	$(CC) $(CFLAGS) modplay_protracker.c -c -o $@

obj/modplay_s3m.o: modplay_s3m.c modplay_s3m.h modplay.h
	$(CC) $(CFLAGS) modplay_s3m.c -c -o $@

obj/modplay.o: modplay.c modplay.h
	$(CC) $(CFLAGS) modplay.c -c -o $@

obj/modrender_stream.o: modrender_stream.c modrender_common.h
	$(CC) $(CFLAGS) modrender_stream.c -c -o $@

obj/modrender_amigus.o: modrender_amigus.c modrender_common.h
	$(CC) $(CFLAGS) modrender_amigus.c -c -o $@


