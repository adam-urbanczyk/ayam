/*==============================================================================
 *
 *	File:		MFFILE.C
 *
 *	Function:	Miscellaneous metafile routines.
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
 *		F3F_RWW	File created.
 *==============================================================================
 */

#include "mffile.h"

#include "mf3d.h"
#include "mferrors.h"
#include "mfobjcts.h"
#include "mftypes.h"
#include "mfassert.h"
#include "mfintobj.h"
#include "mfmemory.h"
#include "mfprimtv.h"
#include "mftextst.h"
#include "mftextut.h"
#include "mftextwr.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_TypeObjWrite
 *
 *	If this is a new user-defined type, write a Type object
 *==============================================================================
 */
MF3DErr
MF3D_TypeObjWrite (
		    MF3D_FilePtr inMetafilePtr,
		    MF3DVoidObjPtr inMF3DObjPtr)
{
  MF3DUnknownObjPtr object;
  MF3DInt32 objType;
  MF3DBoolean foundYet;
  MF3DErr result;

  object = (MF3DUnknownObjPtr) inMF3DObjPtr;
  MFASSERT (object->objectType == kMF3DObjUnknownType);

  MFASSERT (sizeof (objType) == sizeof (object->realObjectType));
  objType = (MF3DInt32) (object->realObjectType);

  result = kMF3DNoErr;

  if (objType <= kMF3DMinimumTypeSeed)
    result = kMF3DErrIllegalUserObjectType;

  /* Is it a user-defined type? */
  if (result == kMF3DNoErr && objType < 0)
    {
      MF3DUns32 numTypes;
      MF3D_TypeListPtr typeListPtr;

      typeListPtr = inMetafilePtr->typeTable.types;
      foundYet = kMF3DBooleanFalse;
      for (numTypes = inMetafilePtr->typeTable.nTypes;
	   foundYet == kMF3DBooleanFalse && numTypes > 0 &&
	   result == kMF3DNoErr;
	   --numTypes, ++typeListPtr)
	{
	  if (objType == typeListPtr->number)
	    {
	      if (MF3D_CmpStrInsensitive (typeListPtr->name,
				(const char *) object->realObjectName) == 0)
		{
		  foundYet = kMF3DBooleanTrue;	/* type already in table */
		}
	      else
		{
		  result = kMF3DErrIllegalUserObjectType;	/* type with    */
		  /* different name already in table      */
		}
	    }
	}
    }

  if (result == kMF3DNoErr && foundYet == kMF3DBooleanFalse)
    {
      MF3DUns32 numTypes;

      /* Add this type to the table */
      ++inMetafilePtr->typeTable.nTypes;
      numTypes = inMetafilePtr->typeTable.nTypes;
      result = MF3D_Reallocate (&inMetafilePtr->typeTable.types,
		       numTypes * sizeof (*inMetafilePtr->typeTable.types));
      if (result == kMF3DNoErr)
	{
	  inMetafilePtr->typeTable.types[numTypes - 1].number = objType;
	  inMetafilePtr->typeTable.types[numTypes - 1].name =
	    MF3D_DuplicateCString (object->realObjectName);

	  if (objType < inMetafilePtr->tocStuff.typeSeed)
	    inMetafilePtr->tocStuff.typeSeed = objType - 1;
	}

      /* Output the Type object */
      if (result == kMF3DNoErr)
	{
	  MF3DVoidObj fakeObj;
	  MF3D_ObjStuffPtr objStuff;

	  fakeObj.objectType = kMF3DObjType;
	  result = MF3D_BeginWrite (inMetafilePtr, &fakeObj, &objStuff);

	  if (result == kMF3DNoErr)
	    result = (*objStuff->writer) (inMetafilePtr, inMF3DObjPtr);

	  if (result == kMF3DNoErr)
	    result = MF3D_EndWrite (inMetafilePtr, &fakeObj);

	  if (result == kMF3DNoErr)
	    MF3D_WriteNewLine (inMetafilePtr);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_CloseReadBuffer
 *
 *	Verify that ReadBuffer is completely popped
 *==============================================================================
 */
MF3DErr
MF3D_CloseReadBuffer (
		       MF3D_FilePtr inMetafilePtr)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (inMetafilePtr->readBuffer.buf != NULL)
    {
      MF3D_Free (inMetafilePtr->readBuffer.buf);
      result = kMF3DErrNotEnoughEndGroups;

      while (inMetafilePtr->readBuffer.saveSize != NULL)
	{
	  MF3D_SaveBufferPtr sizeStack;

	  sizeStack = inMetafilePtr->readBuffer.saveSize;
	  inMetafilePtr->readBuffer.bufSize = sizeStack->bufSize;
	  inMetafilePtr->readBuffer.saveSize = sizeStack->next;
	  MF3D_Free (sizeStack);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_BackpatchTOCLocation
 *
 *	Patch the location of the TOC into the metafile header
 *==============================================================================
 */
MF3DErr
MF3D_BackpatchTOCLocation (
			    MF3D_FilePtr inMF3DFilePtr)
{
  MF3DErr result, seekResult;

  result = kMF3DNoErr;

  /* Backpatch TOC location only for binary files */
  if (!MF3DIsTextFormat (inMF3DFilePtr->dataFormat))
    {
      MF3DBinaryFilePosition curLocation;

      result = MF3DTellPosition (inMF3DFilePtr, &curLocation);

      if (result == kMF3DNoErr)
	{
	  result = MF3DSeekPosition (inMF3DFilePtr,
				     inMF3DFilePtr->tocLocation);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Uns64Write (inMF3DFilePtr, curLocation);

      seekResult = MF3DSeekPosition (inMF3DFilePtr, curLocation);
      if (result == kMF3DNoErr)
	result = seekResult;
    }
  else
    {
      MFASSERT (inMF3DFilePtr->tocStuff.tocLabelName != NULL);
      MF3D_ValidateWriteSize (inMF3DFilePtr, strlen (kMF3D_LabelCharStr " ") +
			      strlen (inMF3DFilePtr->tocStuff.tocLabelName));
      result = MF3D_OutputText (inMF3DFilePtr, "%s%s ",
		  inMF3DFilePtr->tocStuff.tocLabelName, kMF3D_LabelCharStr);
      MF3D_WriteNewLine (inMF3DFilePtr);

      MF3D_Free (inMF3DFilePtr->tocStuff.tocLabelName);
      inMF3DFilePtr->tocStuff.tocLabelName = NULL;
    }

  return result;
}

/*==============================================================================
 *	MF3D_DisposeTOCStuff
 *
 *	Dispose Table of Contents stuff
 *==============================================================================
 */
MF3DErr
MF3D_DisposeTOCStuff (
		       MF3D_FilePtr inMetafilePtr)
{
  MF3DUns32 entriesLeft;
  MF3D_TOCReferencePtr curRefPtr;

  entriesLeft = inMetafilePtr->tocStuff.numReferences;

  if (MF3DIsTextFormat (inMetafilePtr->dataFormat))
    {
      curRefPtr = inMetafilePtr->tocStuff.references;
      MFASSERT (curRefPtr != NULL || entriesLeft == 0);
      for (; entriesLeft > 0; --entriesLeft, ++curRefPtr)
	MF3D_Free (curRefPtr->ref.name);
    }

  MF3D_Free (inMetafilePtr->tocStuff.references);
  /* If there was no Table of Contents, tocLabelName has not been
   * disposed.
   */
  MF3D_Free (inMetafilePtr->tocStuff.tocLabelName);

  return kMF3DNoErr;
}
