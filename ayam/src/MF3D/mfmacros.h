#ifndef	MF3D_MACROS_H
#define	MF3D_MACROS_H
/*==============================================================================
 *
 *	File:		MFMACROS.H
 *
 *	Function:	For oft-repeated code.
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
 *		FB7_JRK	Mac pragmas
 *		Fabio	Changed file name to 8 characters
 *		F2N_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

/* Suppress parameter unused warnings */
#define	MF3D_Unused(x)		if (0) (x)

#define	MF3D_Allocate(ptr)													\
					{	(*(void **)&(ptr)) = MF3D_Malloc(sizeof(*(ptr)));	\
						if ((ptr) == NULL)									\
							result = kMF3DErrOutOfMemory;					\
					}

#define MF3D_AllocArray(ptr, entries)										\
		{	(*(void **)&(ptr)) = MF3D_Malloc((entries) * sizeof(*(ptr)));	\
			if ((ptr) == NULL)												\
				result = kMF3DErrOutOfMemory;								\
		}

#define MF3D_AllocEmpty(ptr)	{	(*(void **)&(ptr)) = MF3D_Malloc(0);	}


#endif
