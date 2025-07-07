/**
 **  $Filename: misc/EaglePlayerConverter.h
 **  $Release: 2.00 $
 **  $Revision: 1$
 **  $Date: 24/07/97$
 **
 ** Definitions and Macros for creating EaglePlayer Noiseconvertermodules
 **
 **	(C) Copyright 1993-97 by DEFECT
 **	    All Rights Reserved
 **
 **/
#ifndef EAGLEPLAYERCONVERTER_H
#define EAGLEPLAYERCONVERTER_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#define CONVERTERVERSION 1
#define SECURITYMEM 2000

/*------------------------------ Convertertags --------------------------------*/
#define	CV_TagBase		(TAG_USER+0x4543)
#define CV_Convertername	(CV_TagBase)
#define CV_Convert		(CV_TagBase+1)
					/* Ptr to ConvertCode
					 * Input: a0=Adr der Datei
					 *	 d0=Size der Datei
					 * Output:d0=ConvertSize oder NULL
					 *	 d1=Flags
					 *	 d2=MemSize
					 *	 a0=ConvertAdr
					 *	 a1=Formatname
					 *	 a2=SamplePtr (für Protrackerclones) oder NULL
					 */

#define CV_Version		(CV_TagBase+2) /* Version of Converter */
#define CV_RequestConverter	(CV_TagBase+3) /* Requested Version of Noiseconverter */
#define CV_EagleBase		(CV_TagBase+4)
#define CV_Creator		(CV_TagBase+5)
#define CV_Flags		(CV_TagBase+6)
#define CV_Next			(CV_TagBase+7) /* Ptr to next Convertertaglist */

	/* end of converterdefinition in V1 */



/* --- various flags, returned by CV_Convert in d1 */

#define CVB_FreeOriginal 0	/* free original data */
#define CVF_FreeOriginal (1<<0)	/* free original data */

#endif			/* end of EAGLEPLAYERCONVERTER.H */
