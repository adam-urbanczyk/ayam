/*==============================================================================
 *
 *	File:		MFBINRD.C
 *
 *	Function:	Miscellaneous functions needed during binary reads.
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
 *		Fabio	Changed file name to 8 characters
 *		F3A_RWW	Binary TOC.
 *		F2O_RWW	File created.
 *==============================================================================
 */

#include "mfbinrd.h"

#include <string.h>		/* memcpy */

#include "mf3d.h"
#include "mfint64.h"
#include "mferrors.h"
#include "mfmacros.h"
#include "mfmemory.h"
#include "mfobject.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

MF3DErr
MF3D_BinaryReadProc (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DSize inAmtToRead,
		      char *outBuffer)
{
  if ((inMetafilePtr->readBuffer.bufPos + inAmtToRead) >
      inMetafilePtr->readBuffer.bufSize)
    {
      return kMF3DErrCantParse;
    }

  /* FBL_JRK */
  /* Cast below is safe since we must be called with a valid buffer */
  memcpy (outBuffer,
	  &inMetafilePtr->readBuffer.buf[inMetafilePtr->readBuffer.bufPos],
	  (size_t) inAmtToRead);
  inMetafilePtr->readBuffer.bufPos += inAmtToRead;

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_PreprocessBinaryFile
 *
 *	Read TOC into memory.
 *	First, we read the metafile header [NOTE: This is inefficient because it is
 *		also the first thing we will do when the next GetAnObject is called].
 *	Then, we will read in the Table of Contents objects, building the tocStuff
 *		table.
 *==============================================================================
 */
MF3DErr
MF3D_PreprocessBinaryFile (
			    MF3D_FilePtr inMetafilePtr)
{
  MF3DMetafileObjPtr headerPtr;
  MF3DTableOfContentsObjPtr tocPtr;
  MF3DBinaryFilePosition nextTocLoc;
  MF3DUns32 numReferences;
  MF3DErr result;

  numReferences = 0;
  /* tocStuff.numReferences must be initialized before calling IntReadObj */
  inMetafilePtr->tocStuff.numReferences = 0;
  MF3D_AllocEmpty (inMetafilePtr->tocStuff.references);
  inMetafilePtr->tocStuff.refSeed = 1;
  inMetafilePtr->tocStuff.typeSeed = -1;

  /* Note: MF3D_IntReadObject sets headerPtr to NULL if it fails
   * (so it is okay to call dispose later)
   */
  result = MF3D_IntReadObject (inMetafilePtr, (MF3DVoidObjPtr *) & headerPtr);

  if (result == kMF3DNoErr)
    {
      if (headerPtr->objectType != kMF3DObjMetafile)
	result = kMF3DErrCantParse;	/* first obj is not metafile header */
    }

  MFASSERT (result != kMF3DNoErr ||
	    !MF3DIsTextFormat (headerPtr->tocLocation->format));

  /* if we have read a good file header, set file read position   */
  /* to read the first item in the table of contents                              */
  if (result == kMF3DNoErr)
    {
      AssignInt64 (nextTocLoc, headerPtr->tocLocation->location.binary);
    }

  /* If we have a table of contents */
  while (result == kMF3DNoErr && (nextTocLoc.hi != 0 || nextTocLoc.lo != 0))
    {
      MF3DUns32 numNewRefs;

      tocPtr = NULL;

      result = MF3DSeekPosition (inMetafilePtr, nextTocLoc);

      if (result == kMF3DNoErr)
	{
	  result = MF3D_IntReadObject (inMetafilePtr,
				       (MF3DVoidObjPtr *) & tocPtr);
	}

      if (result == kMF3DNoErr)
	{
	  if (tocPtr->objectType != kMF3DObjTableOfContents)
	    result = kMF3DErrCantParse;
	}

      if (result == kMF3DNoErr)
	{
	  MFASSERT (!MF3DIsTextFormat (tocPtr->nextTOC->format));
	  AssignInt64 (nextTocLoc, tocPtr->nextTOC->location.binary);
	  numNewRefs = tocPtr->nEntries;
	  result = MF3D_Reallocate (&inMetafilePtr->tocStuff.references,
				    ((numReferences + numNewRefs) *
			     sizeof (*inMetafilePtr->tocStuff.references)));
	  if (result != kMF3DNoErr)
	    MF3D_Free (inMetafilePtr->tocStuff.references);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_TOCReferencePtr refPtr;
	  MF3D_TOCEntryPtr tocRefPtr;

	  refPtr = &inMetafilePtr->tocStuff.references[numReferences - 1];
	  numReferences += numNewRefs;

	  if (tocPtr->refSeed > inMetafilePtr->tocStuff.refSeed)
	    inMetafilePtr->tocStuff.refSeed = tocPtr->refSeed;
	  if (tocPtr->typeSeed < inMetafilePtr->tocStuff.typeSeed)
	    inMetafilePtr->tocStuff.typeSeed = tocPtr->typeSeed;
	  tocRefPtr = tocPtr->tocEntries;
	  while (numNewRefs > 0)
	    {
	      refPtr->refID = tocRefPtr->refID;
	      MFASSERT (!MF3DIsTextFormat (tocRefPtr->objLocation->format));
	      AssignInt64 (refPtr->ref.location,
			   tocRefPtr->objLocation->location.binary);
	      ++refPtr;
	      ++tocRefPtr;
	      --numNewRefs;
	    }
	  inMetafilePtr->tocStuff.numReferences = numReferences;
	}

      MF3DDisposeObject ((MF3DVoidObjPtr) tocPtr);
    }

  MF3DDisposeObject ((MF3DVoidObjPtr) headerPtr);

  if (result == kMF3DNoErr)
    {				/* Reset file back to zero to start real reading */
      MF3DBinaryFilePosition zero;

      SetInt64ToZero (zero);
      result = MF3DSeekPosition (inMetafilePtr, zero);
    }

  return result;
}

/*==============================================================================
 *	MF3D_GetRefNameB
 *
 *	No reference names yet for binary files
 *	@@@But maybe we will want to create them?
 *==============================================================================
 */
MF3DCStringPtr
MF3D_GetRefNameB (
		   MF3D_FilePtr inMetafilePtr,
		   MF3DReferenceID inRefID)
{
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inRefID);
  return NULL;
}

/*==============================================================================
 *	MF3D_GetBinaryRefID
 *
 *	Return the refID associated with inLocation, 0 if none.
 *==============================================================================
 */
MF3DErr
MF3D_GetBinaryRefID (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DBinaryFilePosition inLocation,
		      MF3DReferenceID * outRefID)
{
  MF3D_TOCReferencePtr refPtr;
  MF3DUns32 refCount;

  *outRefID = 0;
  refCount = inMetafilePtr->tocStuff.numReferences;
  refPtr = inMetafilePtr->tocStuff.references;
  while (refCount > 0)
    {
      if (CompareInt64 (refPtr->ref.location, inLocation) == 0)
	{
	  *outRefID = refPtr->refID;
	  break;		/* ### LOOP EXIT if reference found */
	}
      --refCount;
      ++refPtr;
    }

  return kMF3DNoErr;		/* for now, always no err, even if no match */
}

/*==============================================================================
 *	MF3D_PostprocessBinaryFile
 *
 *	Dispose of TOC structures
 *==============================================================================
 */
MF3DErr
MF3D_PostprocessBinaryFile (
			     MF3D_FilePtr inMetafilePtr)
{
  MF3D_Free (inMetafilePtr->tocStuff.references);
  return kMF3DNoErr;
}
