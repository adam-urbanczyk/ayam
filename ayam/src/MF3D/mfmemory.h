#ifndef MF3D_MEMORY_H
#define	MF3D_MEMORY_H
/*==============================================================================
 *
 *	File:		MFMEMORY.H
 *
 *	Function:	Memory allocation routines.
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
 *		FBL_JRK	C++ Compatibility
 *		FBL_JRK	Lowercase include file names
 *		FB7_JRK	Pragma macros
 *		FAK_JRK	Added error debugging when DEBUG > 0
 *		Fabio	Changed file name to 8 characters
 *		F3L_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif


#if defined(DEBUG) && DEBUG > 1
#include <stdio.h>
#endif
#include <stdlib.h>

#include "mfsystyp.h"		/* MF3DUns32      */
#include "mftypes.h"		/* MF3DErr                */

#if defined(DEBUG) && DEBUG > 1
#ifdef MF3D_MEMORY_C
int gMallocCount = 0;
#else
extern int gMallocCount;
#endif
#endif

#if defined(DEBUG) && DEBUG > 0
#ifdef MF3D_MEMORY_C
MF3DUns32 gMallocSize = 0;
#else
extern MF3DUns32 gMallocSize;
#endif
#endif

/* Low-level procs which allocate memory, similar to malloc, realloc, and free */
void *MF3D_Malloc (MF3DUns32 size);
void *MF3D_Realloc (void *ptr, MF3DUns32 size);
void MF3D_Free (void *ptr);

/* MF3D procs which return error codes */
MF3DErr MF3D_Reallocate (void **ptr, MF3DUns32 size);

#if !defined(MF3D_MEMORY_C) && defined(DEBUG) && DEBUG > 1
#define MF3D_Malloc(x)	(printf("File \"%s\"; Line %d\n", __FILE__, __LINE__), MF3D_Malloc(x))
#endif

#endif
