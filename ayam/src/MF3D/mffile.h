#ifndef	MF3D_FILE_H
#define	MF3D_FILE_H
/*==============================================================================
 *
 *	File:		MFFILE.H
 *
 *	Function:	Miscellaneous metafile routines.
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
 *		F3F_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"

MF3DErr
MF3D_TypeObjWrite (
		    MF3D_FilePtr inMetafilePtr,
		    MF3DVoidObjPtr inMF3DObjPtr);

MF3DErr
MF3D_CloseReadBuffer (
		       MF3D_FilePtr inMetafilePtr);

MF3DErr
MF3D_BackpatchTOCLocation (
			    MF3D_FilePtr inMF3DFilePtr);

MF3DErr
MF3D_DisposeTOCStuff (
		       MF3D_FilePtr inMetafilePtr);

#endif
