/*==============================================================================
 *
 *	File:		MFTEXTUT.C
 *
 *	Function:	Text utility routines
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
 *		FB8_JRK	Segmentation
 *		Fabio	Changed file name to 8 characters
 *		F3A_RWW	TOC stuff works.
 *		F2G_RWW	File created.
 *==============================================================================
 */
#include "mftextut.h"

#include <ctype.h>			/* tolower */

#include "mfsystyp.h"
#include "mftypes.h"
#include "mfassert.h"
#include "mfmemory.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_CmpStrInsensitive
 *
 *	Case-insensitive comparison of two strings.
 *
 *	Return 0 for a match; nonzero otherwise.
 *==============================================================================
 */
MF3DInt32
MF3D_CmpStrInsensitive(
	const char		*string1,
	const char		*string2)
{
	MF3DInt32		result;

	MFASSERT(string1 != NULL && string2 != NULL);

	while((result = (tolower(*string1) - tolower(*string2))) == 0 &&
			*string1++ != '\0' && *string2++ != '\0')
	{
	}

	return result;
}

/*==============================================================================
 *	MF3D_DuplicateCString
 *
 *	Make a copy of a CStringPtr by allocating another pointer of the same size.
 *==============================================================================
 */
MF3DCStringPtr
MF3D_DuplicateCString(
	MF3DConstCStringPtr	inStringPtr)
{
	MF3DCStringPtr		result;
	MF3DUns32			stringLen;
	MF3DCStringPtr		pdst;
	MF3DConstCStringPtr	psrc;

	if (inStringPtr == NULL)
		return NULL;

	stringLen = CStringLen(inStringPtr);

	result = (MF3DCStringPtr)MF3D_Malloc(stringLen + 1);

	if (result != NULL)
	{	pdst = result;
		psrc = inStringPtr;

		while ((*pdst++ = *psrc++) != 0)
			;
	}

	return result;
}
