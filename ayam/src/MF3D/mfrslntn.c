/*==============================================================================
 *
 *	File:		MFRSLNTN.C
 *
 *	Function:	Routines for saving information around reference resolution.
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
 *		F3H_RWW	Make the resolution stuff work with external files.
 *		F2N_RWW	File created.
 *==============================================================================
 */

#include "mfrslntn.h"

#include <string.h>

#include "mf3d.h"
#include "mferrors.h"
#include "mfint64.h"
#include "mftypes.h"
#include "mfassert.h"
#include "mfintobj.h"
#include "mfmacros.h"
#include "mfmemory.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_PushResolution
 *
 *	Resolve a reference
 *==============================================================================
 */
MF3DErr
MF3D_PushResolution(
	MF3D_FilePtr		ioMF3DFilePtr,	 	/* In:  MF3D file structure		*/
	MF3DReferenceObjPtr	inMF3DRefObjPtr, 	/* In:  reference object		*/
	MF3DStorageObjPtr	inExternalFilePtr)	/* In:	external storage object	*/
{
	MF3D_FilePtr			newFilePtr;
	MF3DUns32				resState;
	MF3DBinaryFilePosition	returnLoc;
	MF3DErr					result;

	result = kMF3DNoErr;

	MFASSERT(ioMF3DFilePtr->resStuff.reference == NULL);

	if (inMF3DRefObjPtr->refID == 0)
	{	if (inExternalFilePtr == NULL)
			result = kMF3DErrIllegalRefID;
		else
			resState = MF3D_ResolvingFile;
	}
	else
	{	resState = MF3D_ResolvingReference;
	}

	if (result == kMF3DNoErr)
	{	SetInt64ToZero(returnLoc);
		if (inExternalFilePtr != NULL)
		{	/* Got outside object */
			result = MF3DOpenInputStdCFile(
					inExternalFilePtr->pathName, &newFilePtr);
		}
		else
		{	/* Duplicate current file for reference */
			MF3D_Allocate(newFilePtr);

			if (result == kMF3DNoErr)
				result = MF3DTellPosition(ioMF3DFilePtr, &returnLoc);

			if (result == kMF3DNoErr)
				memcpy(newFilePtr, ioMF3DFilePtr, sizeof(*newFilePtr));

			
		}
	}

	if (result == kMF3DNoErr && resState == MF3D_ResolvingReference)
	{	MF3DUns32					objIndex, numObjTableEntries;
		MF3D_ObjectTableEntryPtr	objTableEntryPtr;

		numObjTableEntries = newFilePtr->objTable.numObjects;
		objTableEntryPtr = newFilePtr->objTable.objects;
		for (objIndex = 0;
				objTableEntryPtr->objRefID != inMF3DRefObjPtr->refID;
				++objIndex, ++objTableEntryPtr)
		{	if (objIndex >= numObjTableEntries)
			{	result = kMF3DErrReferenceNotFound;
				break;
			}
		}

		if (result == kMF3DNoErr)
		{	result = MF3DSeekPosition(newFilePtr,
					objTableEntryPtr->objLocation);
		}

		if (result != kMF3DNoErr)
		{	if (inExternalFilePtr != NULL)
				MF3DClose(newFilePtr);
			else
				MF3D_Free(newFilePtr);
		}
	}

	if (result == kMF3DNoErr)
	{	MFASSERT(ioMF3DFilePtr->resStuff.reference == NULL);

		newFilePtr->resStuff.parent = ioMF3DFilePtr;
		newFilePtr->resStuff.resState = resState;
		newFilePtr->resStuff.reference = NULL;
		newFilePtr->readBuffer.buf = NULL;
		newFilePtr->inContainer = 0;
		newFilePtr->typeTable.nTypes = 0;
		MF3D_AllocEmpty(newFilePtr->typeTable.types);
		/* NOTE: returnLoc is only valid for internal references */
		AssignInt64(newFilePtr->resStuff.returnLoc, returnLoc);
		ioMF3DFilePtr->resStuff.reference = newFilePtr;
	}

	return result;
}

/*==============================================================================
 *	MF3D_PopResolution
 *
 *	Close inMF3DFilePtr.
 *==============================================================================
 */
MF3DErr
MF3D_PopResolution(
	MF3D_FilePtr	inMF3DFilePtr)
{
	MF3D_FilePtr			parentFilePtr;
	MF3DBinaryFilePosition	zero;
	MF3DErr					result;

	result = kMF3DNoErr;

	parentFilePtr = inMF3DFilePtr->resStuff.parent;
	MFASSERT(parentFilePtr != NULL);

	if (inMF3DFilePtr->resStuff.reference != NULL)
		result = MF3D_PopResolution(inMF3DFilePtr->resStuff.reference);

	parentFilePtr->resStuff.reference = NULL;

	/* Was it an external reference? */
	SetInt64ToZero(zero);
	if (CompareInt64(inMF3DFilePtr->resStuff.returnLoc, zero) == 0)
	{	result = MF3DClose(inMF3DFilePtr);
	}
	else
	{	/* NOTE: returnLoc is only valid for internal references */
		result = MF3DSeekPosition(inMF3DFilePtr,
				inMF3DFilePtr->resStuff.returnLoc);
	}

	if (inMF3DFilePtr->resStuff.resState > MF3D_ResolvingFile)
		result = kMF3DErrNotEnoughEndGroups;

	return result;
}
