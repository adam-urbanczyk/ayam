#ifndef	MF3D_GROUPS_H
#define	MF3D_GROUPS_H
/*==============================================================================
 *
 *	File:		MFGROUPS.H
 *
 *	Function:	Interface file for MFGROUPS.C.
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


MF3DErr MF3D_InitGroup (MF3D_FilePtr ioMetafile);
MF3DErr MF3D_PopGroup (MF3D_FilePtr ioMetafile);
MF3DErr MF3D_PushGroup (MF3D_FilePtr ioMetafile);
MF3DErr MF3D_DisposeGroup (MF3D_FilePtr ioMetafile);

#endif
