#ifndef	MF3D_BINARYREAD_H
#define	MF3D_BINARYREAD_H
/*==============================================================================
 *
 *	File:		MFBINRD.H
 *
 *	Function:	Binary reading utilities.
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
 *		F3A_RWW	Binary TOC.
 *		F2O_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"

/*==============================================================================
 *	MF3D_BinaryReadProc
 *==============================================================================
 */
MF3DErr MF3D_BinaryReadProc (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DSize inAmtToRead,
			      char *outBuffer);

MF3DErr
MF3D_PreprocessBinaryFile (
			    MF3D_FilePtr inMetafilePtr);

MF3DCStringPtr
MF3D_GetRefNameB (
		   MF3D_FilePtr inMetafilePtr,
		   MF3DReferenceID inRefID);

MF3DErr
MF3D_GetBinaryRefID (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DBinaryFilePosition inLocation,
		      MF3DReferenceID * outRefID);

MF3DErr
MF3D_PostprocessBinaryFile (
			     MF3D_FilePtr inMetafilePtr);

#endif
