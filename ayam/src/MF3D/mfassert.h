#ifndef	MF3D_ASSERT_H
#define	MF3D_ASSERT_H
/*==============================================================================
 *
 *	File:		MFASSERT.H
 *
 *	Function:	Assert wrapper
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
 *		F2G_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#if defined(DEBUG) && DEBUG > 0
#include <assert.h>

#define	MFASSERT(x)	assert(x)

#else /* DEBUG */

#define	MFASSERT(x)	1

#endif /* DEBUG */

#endif
