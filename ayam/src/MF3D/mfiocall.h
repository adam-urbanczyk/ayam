#ifndef MF3DIOCALLBACK_H
#define	MF3DIOCALLBACK_H
/*==============================================================================
 *
 *	File:		MFIOCALL.H
 *
 *	Function:	File I/O Callback Routines
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
 *		F29_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"

/*==============================================================================
 *	If you are unable to use MF3DOpenInputStdCFile and MF3DOpenOutputStdCFile,
 *	MF3DOpenInput and MF3DOpenOutput require an MF3DProcsPtr which define
 *	six basic I/O routines. You must define the six routines and include
 *	them in an MF3DProcsRec in your call to MF3DOpen.
 *==============================================================================
 */

typedef MF3DErr (*MF3DOpenProcPtr) (MF3DDataFormatEnum, MF3DUserOpenDataPtr,
				    MF3DUserFilePtr *);
typedef MF3DErr (*MF3DReadProcPtr) (MF3DUserFilePtr, MF3DSize, char *);
typedef MF3DErr (*MF3DWriteProcPtr) (MF3DUserFilePtr, MF3DSize, const char *);
typedef MF3DErr (*MF3DTellProcPtr) (MF3DUserFilePtr, MF3DBinaryFilePosition *);
typedef MF3DErr (*MF3DSeekProcPtr) (MF3DUserFilePtr, MF3DBinaryFilePosition);
typedef MF3DErr (*MF3DCloseProcPtr) (MF3DUserFilePtr);

typedef struct MF3DProcsRec
  {
    MF3DOpenProcPtr openProc;
    MF3DReadProcPtr readProc;
    MF3DWriteProcPtr writeProc;
    MF3DTellProcPtr tellProc;
    MF3DSeekProcPtr seekProc;
    MF3DCloseProcPtr closeProc;
  }
MF3DProcsRec, *MF3DProcsPtr;

#endif
