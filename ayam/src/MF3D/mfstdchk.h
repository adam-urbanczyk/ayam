/*==============================================================================
 *
 *	File:		MFSTDCHK.H
 *
 *	Function:	Hooks for standard C I/O routines.
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

/* JRK */
/* It seems that some C environments (such as Visual C++) have trouble
 * when reading files that don't follow native newline conventions, so
 * we open it as binary to skip newline conversion.
 */

#define	kMF3D_StdCTextReadPermStr		"rb"
#define	kMF3D_StdCTextWritePermStr		"w+"
#define	kMF3D_StdCBinaryReadPermStr		"rb"
#define	kMF3D_StdCBinaryWritePermStr	"w+b"

typedef enum MF3DStdCPermissionEnum
  {
    kMF3DStdCReadPerm,
    kMF3DStdCWritePerm
  }
MF3DStdCPermissionEnum;

typedef struct MF3DStdCOpenData
  {
    char *filename;
    MF3DStdCPermissionEnum permission;
  }
MF3DStdCOpenData, *MF3DStdCOpenDataPtr;

MF3DErr MF3DStdCOpenHook (MF3DDataFormatEnum, MF3DUserOpenDataPtr,
			  MF3DUserFilePtr *);
MF3DErr MF3DStdCReadHook (MF3DUserFilePtr, MF3DSize, char *);
MF3DErr MF3DStdCWriteHook (MF3DUserFilePtr, MF3DSize, const char *);
MF3DErr MF3DStdCTellHook (MF3DUserFilePtr, MF3DBinaryFilePosition *);
MF3DErr MF3DStdCSeekHook (MF3DUserFilePtr, MF3DBinaryFilePosition);
MF3DErr MF3DStdCCloseHook (MF3DUserFilePtr);
