#ifndef	MF3D_RESOLUTION_H
#define	MF3D_RESOLUTION_H
/*==============================================================================
 *
 *	File:		MFRSLNTN.H
 *
 *	Function:	Interface file for MFRSLNTN.C.
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
 *		F2N_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"

MF3DErr
MF3D_PushResolution (
		      MF3D_FilePtr ioMF3DFilePtr,	/* In:  MF3D file structure             */
		      MF3DReferenceObjPtr inMF3DRefObjPtr,	/* In:  reference object                */
		      MF3DStorageObjPtr inExternalFilePtr);	/* In:  external storage object */

MF3DErr
MF3D_PopResolution (
		     MF3D_FilePtr inMF3DFilePtr);

#endif
