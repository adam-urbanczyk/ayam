#ifndef	MF3D_TEXTWRITE_H
#define	MF3D_TEXTWRITE_H
/*==============================================================================
 *
 *	File:		MFTEXTWR.H
 *
 *	Function:	Interface file for MFTEXTWR.C.
 *
 *	Version:	Metafile:	Version 1.0 3DMF files
 *				Package:	Release #3 of this code
 *
 *	Author(s):	Rick Wong (RWW), Duet Development Corp.
 *				John Kelly (JRK), Duet Development Corp.
 *
 *	Copyright:	(c) 1995 by Apple Computer, Inc., all rights reserved.
 *
 *	Change History (most recent first):
 *		FBL_JRK	Lowercase include file names
 *		FB7_JRK	Pragma macros
 *		Fabio	Changed file name to 8 characters
 *		F2K_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"

#define	kMF3D_RawDataBytesPerLine	16

#define	kMaxOutputTextBuffer		1024	/* Should be large enough */
/*==============================================================================
 *	MF3D_WriteProc
 *==============================================================================
 */
#define	MF3D_WriteProc(x,y,z)												\
					(*x->procsRec.writeProc) (x->userFilePtr,y,(char *)z)

/* write new line */
MF3DErr MF3D_WriteNewLine (MF3D_FilePtr inMetafile);

MF3DErr MF3D_OutputText (MF3D_FilePtr inMetafile, const char *inFormatStr,...);

MF3DErr MF3D_WriteTextString (MF3D_FilePtr inMetafilePtr,
			      MF3DCStringPtr inStringPtr);

#endif
