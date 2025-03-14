/*
  abstraction for file I/O,
  either using stdio.h or Amiga DOS

  (C) 2022 Henryk Richter
*/
#ifndef _INC_FILEIO_H
#define _INC_FILEIO_H

#undef DEBUG
#define DEBUG

#ifdef __SASC
#define USE_AMIGADOS
#endif

#ifdef USE_AMIGADOS
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#ifdef DEBUG
#define D(_x_) do { Printf((STRPTR)"%s:%ld:",(LONG)__FILE__,__LINE__); Printf _x_; } while(0)
#else
#define D(_x_)
#endif
#else
typedef char* STRPTR;
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* sometimes, malloc resides in stdlib, sometimes in string */
#ifdef DEBUG
#define D(_x_) do { fprintf(stderr,"%s:%ld:",__FILE__,(long)__LINE__); printf _x_; } while(0)
#else
#define D(_x_)
#endif
#endif

#ifdef USE_AMIGADOS

#define FH_T BPTR

#define FILE_OPEN_R( a ) Open( (STRPTR)(a), MODE_OLDFILE )
#define FILE_OPEN_W( a ) Open( (STRPTR)(a), MODE_NEWFILE )
#define FILE_CLOSE( fh ) Close( fh )

#define FILE_READ( dest, nbytes, fh ) Read( fh, dest, nbytes )
#define FILE_WRITE( src, nbytes, fh ) Write( fh, src, nbytes )

#define FILE_SEEK_START( pos, fh ) Seek( fh, pos, OFFSET_BEGINNING )
#define FILE_SEEK_CUR( pos, fh )   Seek( fh, pos, OFFSET_CURRENT )
#define FILE_SEEK_END( pos, fh )   Seek( fh, pos, OFFSET_END )
#define FILE_SEEK_TELL( fh ) Seek( fh, 0, OFFSET_CURRENT )

#define CALLOC( sz ) AllocVec( sz, MEMF_PUBLIC|MEMF_CLEAR )
#define MALLOC( sz ) AllocVec( sz, MEMF_PUBLIC )
#define MALLOC_TYPE( _sz_ , _type_ ) AllocVec( _sz_ , _type_ )
#define MFREE( ptr ) FreeVec( ptr )
#define MEMCPY( _dst_, _src_, _bytes_ ) CopyMem( _src_, _dst_, _bytes_ )

#else /* USE_AMIGADOS */

#define FH_T FILE*

#define FILE_OPEN_R( a ) fopen( (const char *)(a), "rb" )
#define FILE_OPEN_W( a ) fopen( (const char *)(a), "wb" )
#define FILE_CLOSE( fh ) fclose( fh )

#define FILE_READ( dest, nbytes, fh ) fread( (void *)(dest), 1, nbytes, fh )
#define FILE_WRITE( src, nbytes, fh ) fwrite( (void*)(src), 1, nbytes, fh )

#define FILE_SEEK_START( pos, fh ) fseek( fh, pos, SEEK_SET )
#define FILE_SEEK_CUR( pos, fh )   fseek( fh, pos, SEEK_CUR )
#define FILE_SEEK_END( pos, fh )   fseek( fh, pos, SEEK_END )
#define FILE_SEEK_TELL( fh ) ftell( fh )

#define CALLOC( sz ) calloc( (size_t)sz, 1 )
#define MALLOC( sz ) malloc( (size_t)sz )
#define MALLOC_TYPE( _sz_ , _type_ ) malloc( (size_t)_sz_ )
#define MFREE( ptr ) free( ptr )
#define MEMCPY( _dst_, _src_, _bytes_ ) memcpy( _dst_, _src_, _bytes_ )


#endif /* USE_AMIGADOS */

#endif /* _INC_FILEIO_H */
