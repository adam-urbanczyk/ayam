/*==============================================================================
 *
 *	File:		MFOBJECT.C
 *
 *	Function:	Object read/write
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
 *		FB4_JRK	Parse object types we don't understand
 *		FB3_JRK	Added UnknownBinary (ukbn)
 *		Fabio	Changed file name to 8 characters
 *		F3K_RWW	GeneralPolygonHint.
 *		F3G_RWW	User-defined objects.
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2S_RWW	BeginGroup changes.
 *		F2R_RWW	Change to simple object theory.
 *		F2H_RWW	File created.
 *==============================================================================
 */

#include "mfobject.h"

#include <string.h>		/* memcpy */

#include "mf3d.h"
#include "mferrors.h"
#include "mfint64.h"
#include "mfobjcts.h"
#include "mfassert.h"
#include "mfgroups.h"
#include "mfintobj.h"
#include "mfmacros.h"
#include "mfmemory.h"
#include "mfobjtyp.h"
#include "mfprimtv.h"
#include "mfrslntn.h"
#include "mftextst.h"
#include "mftextwr.h"		/* MF3D_WriteNewLine */

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MFOBJECT1__
#endif

/*==============================================================================
 *	MF3D_VerifyObject
 *==============================================================================
 */
#define	MF3D_VerifyObjectType(inObj, inObjType)								\
	MFASSERT(inObj != NULL && inObj->objectType == inObjType);


/*==============================================================================
 *	MF3D_IntReadObject
 *
 *	Moved out of ReadAnObject to support BeginGroup()
 *
 *	Sets *outMF3DObjPtr to NULL on failure.
 *==============================================================================
 */
MF3DErr
MF3D_IntReadObject (
		     MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure                     */
		     MF3DVoidObjPtr * outMF3DObjPtr)	/* Out: metafile object                         */
{
  MF3DObjType objType;
  MF3D_ObjStuffPtr objStuff;
  MF3DReferenceID objRefID;
  MF3DVoidObjPtr object;
  MF3DBoolean objTypeIsValid;
  MF3DErr result;

  objTypeIsValid = kMF3DBooleanFalse;
  result = MF3D_BeginRead (inMF3DFilePtr, &objStuff, &objRefID, &objType);

  if (result == kMF3DNoErr)
    {
      objTypeIsValid = kMF3DBooleanTrue;
      result = (*objStuff->reader) (inMF3DFilePtr, &object);
    }

  /* Special setup for User-created objects */
  /* FB4_JRK */
  /* Unknown objects fail to convert, and we allow them to be parsed */
  if (result == kMF3DNoErr && objStuff->type == kMF3DObjUnknownType)
    {
      ((MF3DUnknownObjPtr) object)->realObjectType = objType;
      result = MF3D_ConvertUserDefinedObjType (inMF3DFilePtr, objType,
			   &(((MF3DUnknownObjPtr) object)->realObjectName));
      objType = kMF3DObjUnknownType;
      if (result == kMF3DErrIllegalObjName)
	result = kMF3DNoErr;
    }

  if (result == kMF3DNoErr && object != NULL)
    {
      object->objectType = objType;
      if (objRefID != 0 && objType != kMF3DObjTableOfContents)
	{
	  MF3D_Allocate (object->refInfo);
	  if (result == kMF3DNoErr)
	    {
	      object->refInfo->refID = objRefID;
	      object->refInfo->refName = MF3D_GetRefName (
						   inMF3DFilePtr, objRefID);
	    }
	}
      else
	object->refInfo = NULL;
    }

  /*
   * The only excuse for this small series of special-case tests
   * is that we cannot seem to think of a reasonable alternative
   * at the moment. One option that would alleviate the IsTextFormat
   * test is to read binary files in smaller chunks (like we read
   * the text files), but since that is not a trivial change and
   * would slow the speed of reading binary files, we opt not to
   * do that for now.
   */
  if (result == kMF3DNoErr)
    {
      if (objType != kMF3DObjContainer)
	{
	  result = MF3D_EndRead (inMF3DFilePtr);
	}
      else if (MF3DIsTextFormat (inMF3DFilePtr->dataFormat))
	{			/* After reading a text container, we still have to dispose
				 * of the readBuffer.
				 */
	  /* We should be the end of the object, free the buffer */
	  MFASSERT (inMF3DFilePtr->readBuffer.buf[
				 inMF3DFilePtr->readBuffer.bufPos] == '\0');
	  MFASSERT (inMF3DFilePtr->readBuffer.bufPos ==
		    inMF3DFilePtr->readBuffer.bufSize);
	  MF3D_Free (inMF3DFilePtr->readBuffer.buf);
	  inMF3DFilePtr->readBuffer.buf = NULL;
	}
    }

  /* JRK */
  /* We no longer get rid of buffer if we get an error reading the object
   * This is now the responsibility of the caller. Some people (notably
   * BeginGroup) call this expecting errors if optional objects aren't present.
   *
   * The code used to be:
   *
   *              if (result != kMF3DNoErr)
   *              {       MF3D_Free(inMF3DFilePtr->readBuffer.buf);
   *                              inMF3DFilePtr->readBuffer.buf = NULL;
   *              }       
   */

  /* Note that we do this anyway in the error case */
  if ((objTypeIsValid == kMF3DBooleanTrue || result == kMF3DNoMoreObjects) &&
      inMF3DFilePtr->resStuff.resState >= MF3D_ResolvingReference)
    {
      if (objTypeIsValid == kMF3DBooleanTrue)
	{
	  switch (objType)
	    {
	    case kMF3DObjBeginGroup:
	    case kMF3DObjContainer:
	      ++inMF3DFilePtr->resStuff.resState;
	      break;
	    case kMF3DObjEndGroup:
	    case kMF3DObjEndContainer:		/* Actually, we will not get this */
	      --inMF3DFilePtr->resStuff.resState;
	      break;
	    }
	}
      if (result == kMF3DNoMoreObjects ||
	  inMF3DFilePtr->resStuff.resState == MF3D_ResolvingReference)
	{
	  MFASSERT (inMF3DFilePtr->resStuff.resState >=
		    MF3D_ResolvingReference);
	  /* Reset back to original file pointer */
	  result = MF3D_PopResolution (inMF3DFilePtr);
	}
    }

  if (result == kMF3DNoErr)
    *outMF3DObjPtr = object;
  else
    *outMF3DObjPtr = NULL;

  return result;
}

/*==============================================================================
 *	MF3D_ObjMetafileReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMetafileReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DMetafileObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->dataFormat = inMetafilePtr->dataFormat;
      result = MF3D_Uns16Read (inMetafilePtr, &returnObj->majorVersion);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns16Read (inMetafilePtr, &returnObj->minorVersion);

  if (result == kMF3DNoErr)
    {
      result = MF3D_FlagsRead (inMetafilePtr, gMF3D_MetafileFlags,
			       &returnObj->flags);
    }

  /* Read the TOC ptr */
  if (result == kMF3DNoErr)
    MF3D_Allocate (returnObj->tocLocation);

  if (result == kMF3DNoErr)
    result = MF3D_RefRead (inMetafilePtr, returnObj->tocLocation);

  if (result == kMF3DNoErr)
    {				/* As read from the Tables of Contents */
      returnObj->refSeed = inMetafilePtr->tocStuff.refSeed;
      returnObj->typeSeed = inMetafilePtr->tocStuff.typeSeed;
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  /* We don't support versions prior to release (1.0) */
  if (result == kMF3DNoErr && returnObj->majorVersion < 1)
    result = kMF3DErrUnsupportedMetafileFormat;

  /* Check for valid version? */
  if (result == kMF3DNoErr &&
      (returnObj->majorVersion > kMF3DVersionMajor ||
       (returnObj->majorVersion == kMF3DVersionMajor &&
	returnObj->minorVersion > kMF3DVersionMinor)))
    {
      result = kMF3DWarnNewerMetafileFormat;
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjMetafileWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMetafileWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3DMetafileObjPtr writeObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_VerifyObjectType (inObj, kMF3DObjMetafile);
  writeObj = (MF3DMetafileObjPtr) inObj;

  if (writeObj->refSeed > inMetafilePtr->tocStuff.refSeed)
    {
      if (writeObj->refSeed > kMF3DMaximumRefSeed)
	result = kMF3DErrIllegalRefID;
      inMetafilePtr->tocStuff.refSeed = writeObj->refSeed;
    }

  if (writeObj->typeSeed < inMetafilePtr->tocStuff.typeSeed)
    {
      if (writeObj->typeSeed < kMF3DMinimumTypeSeed)
	result = kMF3DErrIllegalUserObjectType;
      inMetafilePtr->tocStuff.typeSeed = writeObj->typeSeed;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns16Write (inMetafilePtr, kMF3DVersionMajor);

  if (result == kMF3DNoErr)
    result = MF3D_Uns16Write (inMetafilePtr, kMF3DVersionMinor);

  if (result == kMF3DNoErr)
    {
      result = MF3D_FlagsWrite (inMetafilePtr, gMF3D_MetafileFlags,
				((MF3DMetafileObjPtr) inObj)->flags);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_TOCRefWrite (inMetafilePtr,
				 ((MF3DMetafileObjPtr) inObj)->tocLocation);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjMetafileDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMetafileDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_VerifyObjectType (inObj, kMF3DObjMetafile);

  if (inObj != NULL)
    {
      result = MF3D_PositionReferenceDisposer (
				 ((MF3DMetafileObjPtr) inObj)->tocLocation);
      MF3D_Free (((MF3DMetafileObjPtr) inObj)->tocLocation);
      MF3D_Free (inObj);
    }
  return result;
}

/*==============================================================================
 *	MF3D_ObjContainerReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjContainerReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DContainerObjPtr containerObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (containerObj);

  if (result == kMF3DNoErr)
    {
      ++inMetafilePtr->inContainer;
      *outObj = (MF3DVoidObjPtr) containerObj;
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjContainerWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjContainerWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjContainer);

  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjContainerDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjContainerDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjContainer);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEndContainerWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjEndContainerWriter (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEndContainer);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEndContainerDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjEndContainerDisposer (
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEndContainer);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjBeginGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjBeginGroupReader (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr * outObj)
{
  MF3DBeginGroupObjPtr returnObj;
  MF3DErr result, dispResult;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->group = NULL;
      returnObj->displayState = NULL;
      result = MF3D_IntReadObject (inMetafilePtr, &returnObj->group);
    }

  /* Optional display state */
  if (result == kMF3DNoErr)
    {
      dispResult = MF3D_IntReadObject (inMetafilePtr,
			      (MF3DVoidObjPtr *) & returnObj->displayState);
      if (dispResult == kMF3DNoErr &&
	  returnObj->displayState->objectType !=
	  kMF3DObjDisplayGroupState)
	{
	  result = kMF3DErrWrongObjType;
	}
      if (dispResult != kMF3DErrCantParse)
	result = dispResult;
    }

  if (result == kMF3DNoErr)
    result = MF3D_PushGroup (inMetafilePtr);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjBeginGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjBeginGroupWriter (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr inObj)
{
  MF3DBeginGroupObjPtr beginGroupObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjBeginGroup);
  beginGroupObj = (MF3DBeginGroupObjPtr) inObj;

  if (beginGroupObj->group == NULL)
    return kMF3DErrGroupTypeNotSpecified;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3DWriteAnObject (inMetafilePtr, beginGroupObj->group);

  if (result == kMF3DNoErr && beginGroupObj->displayState != NULL)
    {
      result = MF3DWriteAnObject (inMetafilePtr,
			      (MF3DVoidObjPtr) beginGroupObj->displayState);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjBeginGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjBeginGroupDisposer (
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBeginGroup);

  if (inObj != NULL)
    {
      MF3DDisposeObject (((MF3DBeginGroupObjPtr) inObj)->group);
      MF3DDisposeObject ((MF3DVoidObjPtr)
			 ((MF3DBeginGroupObjPtr) inObj)->displayState);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEndGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjEndGroupReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DEndGroupObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {				/* Do any necessary end group processing */
      result = MF3D_PopGroup (inMetafilePtr);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjEndGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjEndGroupWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEndGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEndGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjEndGroupDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEndGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjReferenceReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjReferenceReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DReferenceObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->refID);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjReferenceWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjReferenceWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjReference);

  return MF3D_Uns32Write (inMetafilePtr, ((MF3DReferenceObjPtr) inObj)->refID);
}

/*==============================================================================
 *	MF3D_ObjReferenceDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjReferenceDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjReference);

  MF3D_Free (inObj);

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTableOfContentsReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTableOfContentsReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DTableOfContentsObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    MF3D_Allocate (returnObj->nextTOC);

  if (result == kMF3DNoErr)
    result = MF3D_RefRead (inMetafilePtr, returnObj->nextTOC);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->refSeed);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Read (inMetafilePtr, &returnObj->typeSeed);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->tocEntryType);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->tocEntrySize);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nEntries);

  if (result == kMF3DNoErr)
    {
      MF3DUns32 entriesLeft;

      entriesLeft = returnObj->nEntries;
      if (entriesLeft == 0)
	{
	  returnObj->tocEntries = NULL;
	}
      else
	{
	  MF3D_TOCEntryPtr curEntryPtr;

	  MF3D_AllocArray (returnObj->tocEntries, entriesLeft);

	  curEntryPtr = returnObj->tocEntries;
	  while (result == kMF3DNoErr && entriesLeft > 0)
	    {
	      result = MF3D_Uns32Read (inMetafilePtr, &curEntryPtr->refID);

	      if (curEntryPtr->refID >= returnObj->refSeed)
		result = kMF3DErrIllegalRefID;

	      if (result == kMF3DNoErr)
		MF3D_Allocate (curEntryPtr->objLocation);

	      if (result == kMF3DNoErr)
		{
		  result = MF3D_RefRead (inMetafilePtr,
					 curEntryPtr->objLocation);
		}

	      if (result == kMF3DNoErr)
		{
		  if (returnObj->tocEntryType >= 1)
		    {
		      result = MF3D_ObjTypeRead (inMetafilePtr, NULL,
						 &curEntryPtr->objType);
		    }
		  else
		    {		/* Not necessary, but looks better */
		      curEntryPtr->objType = 0L;
		    }
		}

	      --entriesLeft;
	      ++curEntryPtr;
	    }

	  if (result != kMF3DNoErr)
	    {
	      MF3D_Free (returnObj->tocEntries);
	    }
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTableOfContentsWriter
 *
 *	Called only from MF3DClose when writing.
 *==============================================================================
 */
MF3DErr
MF3D_ObjTableOfContentsWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3DUns32 entriesLeft;
  MF3DErr result;

  MF3D_Unused (inObj);

  result = kMF3DNoErr;

  MF3D_WriteNewLine (inMetafilePtr);

  /* NOTE: This will always write 0 in the binary case and
   * nextTOC> in the text case. In the binary case, we will
   * also trash inMetafilePtr->tocLocation, but we no longer
   * need it because we have already backpatched the header.
   * It also has the side effect of allocating a string that
   * gets placed into inMetafilePtr->tocStuff.tocLabelName.
   */
  result = MF3D_TOCRefWrite (inMetafilePtr, NULL);

  MF3D_Free (inMetafilePtr->tocStuff.tocLabelName);
  inMetafilePtr->tocStuff.tocLabelName = NULL;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr,
				inMetafilePtr->tocStuff.refSeed);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "reference Seed");

  if (result == kMF3DNoErr)
    {
      result = MF3D_Int32Write (inMetafilePtr,
				inMetafilePtr->tocStuff.typeSeed);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "type Seed");

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, kMF3DTocEntryType);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, kMF3DTocEntrySize);

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "tocEntry type/size");

  if (result == kMF3DNoErr)
    {
      entriesLeft = inMetafilePtr->tocStuff.numReferences;
      result = MF3D_Uns32Write (inMetafilePtr, entriesLeft);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "nEntries");

  if (result == kMF3DNoErr)
    {
      MF3D_TOCReferencePtr curRefPtr;

      curRefPtr = inMetafilePtr->tocStuff.references;
      MFASSERT (curRefPtr != NULL || entriesLeft == 0);

      for (; result == kMF3DNoErr && entriesLeft > 0;
	   --entriesLeft, ++curRefPtr)
	{
	  MF3D_ObjStuffPtr ignoredObjStuff;
	  MF3DVoidObj fakeObj;

	  result = MF3D_Uns32Write (inMetafilePtr, curRefPtr->refID);

	  if (result == kMF3DNoErr)
	    result = MF3D_RefWrite (inMetafilePtr, curRefPtr->ref);

	  if (result == kMF3DNoErr)
	    {			/* Fake an object to write the type */
	      MFASSERT ((MF3DInt32) curRefPtr->type > 0);
	      fakeObj.objectType = curRefPtr->type;
	      result = MF3D_ObjTypeWrite (inMetafilePtr, &fakeObj,
					  &ignoredObjStuff);
	    }

	  if (result == kMF3DNoErr)
	    MF3D_WriteNewLine (inMetafilePtr);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTableOfContentsDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTableOfContentsDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3DTableOfContentsObjPtr tocObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjTableOfContents);

  result = kMF3DNoErr;

  tocObj = (MF3DTableOfContentsObjPtr) inObj;

  if (tocObj != NULL)
    {
      MF3DUns32 entriesLeft;

      result = MF3D_PositionReferenceDisposer (tocObj->nextTOC);

      if (result == kMF3DNoErr)
	{
	  MF3D_TOCEntryPtr curEntryPtr;

	  curEntryPtr = tocObj->tocEntries;
	  for (entriesLeft = tocObj->nEntries; entriesLeft > 0; --entriesLeft)
	    {
	      MF3D_PositionReferenceDisposer (curEntryPtr->objLocation);
	      MF3D_Free (curEntryPtr->objLocation);
	      ++curEntryPtr;
	    }

	  MF3D_Free (tocObj->nextTOC);
	  MF3D_Free (tocObj->tocEntries);
	  MF3D_Free (tocObj);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DInt32 typeID;		/* always negative                              */
  MF3DCStringPtr owner;		/* ISO 9070-registered ID               */
  MF3DErr result;

  result = MF3D_Int32Read (inMetafilePtr, &typeID);

  if (result == kMF3DNoErr && typeID >= 0)
    result = kMF3DErrIllegalUserObjectType;

  if (result == kMF3DNoErr)
    result = MF3D_CStringRead (inMetafilePtr, &owner);

  /* Make sure that the type number is not already taken */
  if (result == kMF3DNoErr)
    {
      MF3DUns32 numTypes;
      MF3D_TypeListPtr typeListPtr;

      typeListPtr = inMetafilePtr->typeTable.types;
      for (numTypes = inMetafilePtr->typeTable.nTypes;
	   result == kMF3DNoErr && numTypes > 0;
	   --numTypes, ++typeListPtr)
	{
	  if (typeID == typeListPtr->number)
	    result = kMF3DErrIllegalUserObjectType;
	}
    }

  /* Add it to our table */
  if (result == kMF3DNoErr)
    {
      MF3DUns32 numTypes;

      ++inMetafilePtr->typeTable.nTypes;
      numTypes = inMetafilePtr->typeTable.nTypes;

      result = MF3D_Reallocate (&inMetafilePtr->typeTable.types,
		       numTypes * sizeof (*inMetafilePtr->typeTable.types));

      if (result == kMF3DNoErr)
	{
	  inMetafilePtr->typeTable.types[numTypes - 1].number = typeID;
	  inMetafilePtr->typeTable.types[numTypes - 1].name = owner;

	  if (typeID < inMetafilePtr->tocStuff.typeSeed)
	    inMetafilePtr->tocStuff.typeSeed = typeID - 1;
	}
    }

  /* Force reader to read again */
  *outObj = NULL;

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeWriter
 *
 *	Supposed to be called only by MF3D_TypeObjWrite with inObj being of type
 *	kMF3DObjUnknownType
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3DUnknownObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjUnknownType);
  writeObj = (MF3DUnknownObjPtr) inObj;

  MFASSERT (sizeof (writeObj->realObjectType) == sizeof (MF3DInt32));
  result = MF3D_Int32Write (inMetafilePtr,
			    (MF3DInt32) writeObj->realObjectType);

  if (result == kMF3DNoErr)
    result = MF3D_CStringWrite (inMetafilePtr, writeObj->realObjectName);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjType);
  MF3D_Unused (inObj);
  return kMF3DErrIllegalObjName;
}

/*==============================================================================
 *	MF3D_ObjFaceAttributeSetListReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceAttributeSetListReader (
				     MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj)
{
  MF3DFaceAttributeSetListObjPtr returnObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_PackingEnum,
			      &returnObj->packing);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nIndices);
      nObjects = returnObj->nObjects;
      nIndices = returnObj->nIndices;
    }

  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    {
      MF3DUns32 indicesToRead;
      MF3DUns32Ptr curIndexPtr;

      MF3D_AllocArray (returnObj->indices, nIndices);

      indicesToRead = nIndices;
      curIndexPtr = returnObj->indices;
      while (result == kMF3DNoErr && indicesToRead > 0)
	{
	  result = MF3D_VariableUnsRead (inMetafilePtr, nObjects, curIndexPtr);
	  --indicesToRead;
	  ++curIndexPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->indices);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjFaceAttributeSetListWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceAttributeSetListWriter (
				     MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj)
{
  MF3DFaceAttributeSetListObjPtr writeObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjFaceAttributeSetList);
  writeObj = (MF3DFaceAttributeSetListObjPtr) inObj;

  nObjects = writeObj->nObjects;
  nIndices = writeObj->nIndices;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Uns32Write (inMetafilePtr, nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_PackingEnum,
			       writeObj->packing);
    }

  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, nIndices);

  if (result == kMF3DNoErr)
    {
      MF3DUns32Ptr curIndexPtr;

      if (nIndices > 0)
	MF3D_WriteNewLine (inMetafilePtr);
      curIndexPtr = writeObj->indices;
      for (; result == kMF3DNoErr && nIndices > 0; --nIndices)
	{
	  result = MF3D_VariableUnsWrite (inMetafilePtr, nObjects,
					  *curIndexPtr++);
	}
      MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjFaceAttributeSetListDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceAttributeSetListDisposer (
				       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjFaceAttributeSetList);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DFaceAttributeSetListObjPtr) inObj)->indices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjGeometryAttributeSetListReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeometryAttributeSetListReader (
					 MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj)
{
  MF3DGeometryAttributeSetListObjPtr returnObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_PackingEnum,
			      &returnObj->packing);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nIndices);
      nObjects = returnObj->nObjects;
      nIndices = returnObj->nIndices;
    }

  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    {
      MF3DUns32 indicesToRead;
      MF3DUns32Ptr curIndexPtr;

      MF3D_AllocArray (returnObj->indices, nIndices);

      indicesToRead = nIndices;
      curIndexPtr = returnObj->indices;
      while (result == kMF3DNoErr && indicesToRead > 0)
	{
	  result = MF3D_VariableUnsRead (inMetafilePtr, nObjects, curIndexPtr);
	  --indicesToRead;
	  ++curIndexPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->indices);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjGeometryAttributeSetListWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeometryAttributeSetListWriter (
					 MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj)
{
  MF3DGeometryAttributeSetListObjPtr writeObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjGeometryAttributeSetList);
  writeObj = (MF3DGeometryAttributeSetListObjPtr) inObj;

  nObjects = writeObj->nObjects;
  nIndices = writeObj->nIndices;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Uns32Write (inMetafilePtr, nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_PackingEnum,
			       writeObj->packing);
    }

  nIndices = writeObj->nIndices;
  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, nIndices);

  if (result == kMF3DNoErr)
    {
      MF3DUns32Ptr curIndexPtr;

      if (nIndices > 0)
	MF3D_WriteNewLine (inMetafilePtr);
      curIndexPtr = writeObj->indices;
      for (; result == kMF3DNoErr && nIndices > 0; --nIndices)
	{
	  result = MF3D_VariableUnsWrite (inMetafilePtr, nObjects,
					  *curIndexPtr++);
	}
      MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjGeometryAttributeSetListDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeometryAttributeSetListDisposer (
					   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjGeometryAttributeSetList);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DGeometryAttributeSetListObjPtr) inObj)->indices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjVertexAttributeSetListReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjVertexAttributeSetListReader (
				       MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj)
{
  MF3DVertexAttributeSetListObjPtr returnObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_PackingEnum,
			      &returnObj->packing);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nIndices);
      nObjects = returnObj->nObjects;
      nIndices = returnObj->nIndices;
    }

  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    {
      MF3DUns32 indicesToRead;
      MF3DUns32Ptr curIndexPtr;

      MF3D_AllocArray (returnObj->indices, nIndices);

      indicesToRead = nIndices;
      curIndexPtr = returnObj->indices;
      while (result == kMF3DNoErr && indicesToRead > 0)
	{
	  result = MF3D_VariableUnsRead (inMetafilePtr, nObjects, curIndexPtr);
	  --indicesToRead;
	  ++curIndexPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->indices);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjVertexAttributeSetListWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjVertexAttributeSetListWriter (
				       MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj)
{
  MF3DVertexAttributeSetListObjPtr writeObj;
  MF3DUns32 nObjects;
  MF3DUns32 nIndices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjVertexAttributeSetList);
  writeObj = (MF3DVertexAttributeSetListObjPtr) inObj;

  nObjects = writeObj->nObjects;
  nIndices = writeObj->nIndices;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Uns32Write (inMetafilePtr, nObjects);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_PackingEnum,
			       writeObj->packing);
    }

  nIndices = writeObj->nIndices;
  if (result == kMF3DNoErr && nIndices < 0)
    result = kMF3DErrNIndicesLessThanZero;

  if (result == kMF3DNoErr && nIndices > nObjects)	/* F7K_PAM */
    result = kMF3DErrNIndicesGreaterThanNObjects;

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, nIndices);

  if (result == kMF3DNoErr)
    {
      MF3DUns32Ptr curIndexPtr;

      if (nIndices > 0)
	MF3D_WriteNewLine (inMetafilePtr);
      curIndexPtr = writeObj->indices;
      for (; result == kMF3DNoErr && nIndices > 0; --nIndices)
	{
	  result = MF3D_VariableUnsWrite (inMetafilePtr, nObjects,
					  *curIndexPtr++);
	}
      MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjVertexAttributeSetListDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjVertexAttributeSetListDisposer (
					 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjVertexAttributeSetList);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DVertexAttributeSetListObjPtr) inObj)->indices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCameraPlacementReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraPlacementReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DCameraPlacementObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->pointOfInterest);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->upVector);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraPlacementWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraPlacementWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3DCameraPlacementObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjCameraPlacement);

  writeObj = (MF3DCameraPlacementObjPtr) inObj;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->location);

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Point3DWrite (inMetafilePtr, writeObj->pointOfInterest);
    }

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->upVector);
    }

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (inMetafilePtr);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraPlacementDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraPlacementDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCameraPlacement);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCameraRangeReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraRangeReader (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr * outObj)
{
  MF3DCameraRangeObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->hither);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->yon);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraRangeWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraRangeWriter (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr inObj)
{
  MF3DCameraRangeObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjCameraRange);
  writeObj = (MF3DCameraRangeObjPtr) inObj;

  result = MF3D_Float32Write (inMetafilePtr, writeObj->hither);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->yon);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraRangeDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraRangeDisposer (
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCameraRange);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCameraViewPortReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraViewPortReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DCameraViewPortObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point2DRead (inMetafilePtr, &returnObj->origin);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->height);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraViewPortWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraViewPortWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3DCameraViewPortObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjCameraViewPort);
  writeObj = (MF3DCameraViewPortObjPtr) inObj;

  result = MF3D_Point2DWrite (inMetafilePtr, writeObj->origin);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->height);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCameraViewPortDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCameraViewPortDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCameraViewPort);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjBottomCapAttributeSetReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjBottomCapAttributeSetReader (
				      MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj)
{
  MF3DBottomCapAttributeSetObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjBottomCapAttributeSetWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjBottomCapAttributeSetWriter (
				      MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBottomCapAttributeSet);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjBottomCapAttributeSetDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjBottomCapAttributeSetDisposer (
					MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBottomCapAttributeSet);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCapsReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCapsReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DCapsObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_FlagsRead (inMetafilePtr, gMF3D_CapsFlags,
			       &returnObj->caps);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCapsWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCapsWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCaps);
  return MF3D_FlagsWrite (inMetafilePtr, gMF3D_CapsFlags,
			  ((MF3DCapsObjPtr) inObj)->caps);
}

/*==============================================================================
 *	MF3D_ObjCapsDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCapsDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCaps);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjFaceCapAttributeSetReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceCapAttributeSetReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DFaceCapAttributeSetObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjFaceCapAttributeSetWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceCapAttributeSetWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjFaceCapAttributeSet);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjFaceCapAttributeSetDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjFaceCapAttributeSetDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjFaceCapAttributeSet);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTopCapAttributeSetReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTopCapAttributeSetReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DTopCapAttributeSetObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTopCapAttributeSetWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTopCapAttributeSetWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTopCapAttributeSet);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTopCapAttributeSetDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTopCapAttributeSetDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTopCapAttributeSet);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDisplayGroupStateReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupStateReader (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj)
{
  MF3DDisplayGroupStateObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_FlagsRead (inMetafilePtr, gMF3D_DisplayGroupStateFlags,
			       &returnObj->traversalFlags);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjDisplayGroupStateWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupStateWriter (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDisplayGroupState);

  return MF3D_FlagsWrite (inMetafilePtr, gMF3D_DisplayGroupStateFlags,
		     ((MF3DDisplayGroupStateObjPtr) inObj)->traversalFlags);
}

/*==============================================================================
 *	MF3D_ObjDisplayGroupStateDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupStateDisposer (
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDisplayGroupState);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLightDataReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightDataReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DLightDataObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    MF3D_BooleanRead (inMetafilePtr, &returnObj->isOn);

  if (result == kMF3DNoErr)
    MF3D_Float32Read (inMetafilePtr, &returnObj->intensity);

  if (result == kMF3DNoErr)
    MF3D_RGBColorRead (inMetafilePtr, &returnObj->color);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjLightDataWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightDataWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DLightDataObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjLightData);
  writeObj = (MF3DLightDataObjPtr) inObj;

  result = MF3D_BooleanWrite (inMetafilePtr, writeObj->isOn);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->intensity);

  if (result == kMF3DNoErr)
    result = MF3D_RGBColorWrite (inMetafilePtr, writeObj->color);

  return result;
}

/*==============================================================================
 *	MF3D_ObjLightDataDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightDataDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLightData);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMeshCornersReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshCornersReader (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr * outObj)
{
  MF3DMeshCornersObjPtr returnObj;
  MF3DErr result;
  MF3DUns32 numCorners;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nCorners);

  if (result == kMF3DNoErr)
    {
      numCorners = returnObj->nCorners;
      if (numCorners <= 0)
	result = kMF3DErrNumCornersIsZero;
    }

  if (result == kMF3DNoErr)
    {
      MF3DMeshCornerPtr curCornerPtr;

      MF3D_AllocArray (returnObj->corners, numCorners);

      curCornerPtr = returnObj->corners;
      while (result == kMF3DNoErr && numCorners > 0)
	{
	  MF3DUns32 numFaces;

	  result = MF3D_Uns32Read (inMetafilePtr, &curCornerPtr->vertexIndex);

	  if (result == kMF3DNoErr)
	    result = MF3D_Uns32Read (inMetafilePtr, &curCornerPtr->nFaces);

	  if (result == kMF3DNoErr)
	    {
	      numFaces = curCornerPtr->nFaces;
	      if (numFaces <= 0)
		result = kMF3DErrNumFacesIsZero;
	    }

	  if (result == kMF3DNoErr)
	    {
	      MF3DUns32Ptr curFacePtr;

	      MF3D_AllocArray (curCornerPtr->faces, numFaces);

	      curFacePtr = curCornerPtr->faces;
	      while (result == kMF3DNoErr && numFaces > 0)
		{
		  result = MF3D_Uns32Read (inMetafilePtr, curFacePtr);
		  --numFaces;
		  ++curFacePtr;
		}

	      if (result != kMF3DNoErr)
		{
		  MF3D_Free (curCornerPtr->faces);
		}
	    }

	  --numCorners;
	  ++curCornerPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->corners);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshCornersWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshCornersWriter (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr inObj)
{
  MF3DMeshCornersObjPtr writeObj;
  MF3DUns32 nCorners;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjMeshCorners);
  writeObj = (MF3DMeshCornersObjPtr) inObj;

  result = kMF3DNoErr;

  nCorners = writeObj->nCorners;
  if (nCorners <= 0)
    result = kMF3DErrNumCornersIsZero;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, nCorners);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "nCorners");

  if (result == kMF3DNoErr)
    {
      MF3DUns32 cornerNum;
      MF3DMeshCornerPtr curCornerPtr;

      curCornerPtr = writeObj->corners;
      for (cornerNum = 0;
	   result == kMF3DNoErr && cornerNum < nCorners;
	   ++cornerNum)
	{
	  MF3DUns32 nFaces;

	  result = MF3D_Uns32Write (inMetafilePtr, curCornerPtr->vertexIndex);

	  nFaces = curCornerPtr->nFaces;
	  if (nFaces <= 0)
	    result = kMF3DErrNumFacesIsZero;

	  if (result == kMF3DNoErr)
	    result = MF3D_Uns32Write (inMetafilePtr, nFaces);

	  if (result == kMF3DNoErr)
	    {
	      MF3DUns32Ptr curFacePtr;

	      curFacePtr = curCornerPtr->faces;
	      for (; result == kMF3DNoErr && nFaces > 0; --nFaces)
		{
		  result = MF3D_Uns32Write (inMetafilePtr, *curFacePtr++);
		}
	    }
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, cornerNum);
	  ++curCornerPtr;
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshCornersDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshCornersDisposer (
			      MF3DVoidObjPtr inObj)
{
  MF3DMeshCornersObjPtr mcObj;

  MF3D_VerifyObjectType (inObj, kMF3DObjMeshCorners);

  mcObj = (MF3DMeshCornersObjPtr) inObj;

  if (mcObj != NULL && mcObj->corners != NULL)
    {
      MF3DUns32 numCorners;
      MF3DMeshCornerPtr cornerPtr;

      cornerPtr = mcObj->corners;
      for (numCorners = mcObj->nCorners; numCorners > 0; --numCorners)
	{
	  MF3D_Free (cornerPtr->faces);
	  ++cornerPtr;
	}

      MF3D_Free (mcObj->corners);
    }
  MF3D_Free (mcObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMeshEdgesReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshEdgesReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DMeshEdgesObjPtr returnObj;
  MF3DErr result;
  MF3DUns32 numEdges;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nEdges);

  if (result == kMF3DNoErr)
    {
      numEdges = returnObj->nEdges;
      if (numEdges <= 0)
	result = kMF3DErrNumEdgesIsZero;
    }

  if (result == kMF3DNoErr)
    {
      MF3DMeshEdgePtr curEdgePtr;

      MF3D_AllocArray (returnObj->edges, numEdges);

      curEdgePtr = returnObj->edges;
      while (result == kMF3DNoErr && numEdges > 0)
	{
	  result = MF3D_Uns32Read (inMetafilePtr, &curEdgePtr->vertexIndex1);

	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_Uns32Read (inMetafilePtr,
				       &curEdgePtr->vertexIndex2);
	    }

	  --numEdges;
	  ++curEdgePtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->edges);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshEdgesWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshEdgesWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DMeshEdgesObjPtr writeObj;
  MF3DUns32 nEdges;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjMeshEdges);
  writeObj = (MF3DMeshEdgesObjPtr) inObj;

  result = kMF3DNoErr;

  nEdges = writeObj->nEdges;
  if (nEdges <= 0)
    result = kMF3DErrNumEdgesIsZero;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, nEdges);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "nEdges");

  if (result == kMF3DNoErr)
    {
      MF3DUns32 edgeNum;
      MF3DMeshEdgePtr curEdgePtr;

      curEdgePtr = writeObj->edges;
      for (edgeNum = 0;
	   result == kMF3DNoErr && edgeNum < nEdges;
	   ++edgeNum)
	{
	  result = MF3D_Uns32Write (inMetafilePtr, curEdgePtr->vertexIndex1);

	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_Uns32Write (inMetafilePtr,
					curEdgePtr->vertexIndex2);
	    }

	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, edgeNum);

	  ++curEdgePtr;
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshEdgesDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshEdgesDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMeshEdges);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DMeshEdgesObjPtr) inObj)->edges);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjNURBCurve2DReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurve2DReader (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr * outObj)
{
  MF3DNURBCurve2DObjPtr returnObj;
  MF3DUns32 numPoints;
  MF3DUns32 order;
  MF3DUns32 numKnots;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->order);

  if (result == kMF3DNoErr)
    {
      order = returnObj->order;
      if (order < 2)
	result = kMF3DErrOrderTooSmall;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nPoints);

  if (result == kMF3DNoErr)
    {
      numPoints = returnObj->nPoints;
      numKnots = numPoints + order;
      if (numPoints < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint3DPtr curPointPtr;

      MF3D_AllocArray (returnObj->points, numPoints);

      curPointPtr = returnObj->points;
      while (result == kMF3DNoErr && numPoints > 0)
	{
	  result = MF3D_RationalPoint3DRead (inMetafilePtr, curPointPtr);
	  --numPoints;
	  ++curPointPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->points);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;

      MF3D_AllocArray (returnObj->knots, numKnots);

      curKnotPtr = returnObj->knots;
      while (result == kMF3DNoErr && numKnots > 0)
	{
	  result = MF3D_Float32Read (inMetafilePtr, curKnotPtr);
	  --numKnots;
	  ++curKnotPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->knots);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBCurve2DWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurve2DWriter (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr inObj)
{
  MF3DNURBCurve2DObjPtr writeObj;
  MF3DUns32 numPoints;
  MF3DUns32 order;
  MF3DUns32 numKnots;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjNURBCurve2D);
  writeObj = (MF3DNURBCurve2DObjPtr) inObj;

  result = kMF3DNoErr;

  order = writeObj->order;
  if (order < 2)
    result = kMF3DErrOrderTooSmall;

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, order);

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "order");

  if (result == kMF3DNoErr)
    {
      numPoints = writeObj->nPoints;
      if (numPoints < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, numPoints);

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "nPoints");

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint3DPtr curPointPtr;
      MF3DUns32 pointNum;

      curPointPtr = writeObj->points;
      for (pointNum = 0;
	   result == kMF3DNoErr && pointNum < numPoints;
	   ++pointNum)
	{
	  result = MF3D_RationalPoint3DWrite (inMetafilePtr, *curPointPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, pointNum);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;
      MF3DUns32 knotNum;

      MF3D_CommentStrWrite (inMetafilePtr, "knots");

      numKnots = numPoints + order;

      curKnotPtr = writeObj->knots;
      for (knotNum = 0; result == kMF3DNoErr && knotNum < numKnots; ++knotNum)
	{
	  result = MF3D_Float32Write (inMetafilePtr, *curKnotPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, knotNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBCurve2DDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurve2DDisposer (
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjNURBCurve2D);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DNURBCurve2DObjPtr) inObj)->points);
      MF3D_Free (((MF3DNURBCurve2DObjPtr) inObj)->knots);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjShaderDataReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderDataReader (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr * outObj)
{
  MF3DShaderDataObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_ShaderUVBoundaryEnum,
			      &returnObj->uBounds);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_ShaderUVBoundaryEnum,
			      &returnObj->vBounds);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjShaderDataWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderDataWriter (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr inObj)
{
  MF3DShaderDataObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjShaderData);
  writeObj = (MF3DShaderDataObjPtr) inObj;

  result = MF3D_EnumWrite (inMetafilePtr, gMF3D_ShaderUVBoundaryEnum,
			   writeObj->uBounds);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_ShaderUVBoundaryEnum,
			       writeObj->vBounds);
    }
  return result;
}

/*==============================================================================
 *	MF3D_ObjShaderDataDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderDataDisposer (
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShaderData);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjShaderTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderTransformReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DShaderTransformObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_Matrix4x4Read (inMetafilePtr, returnObj->shaderTransform);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjShaderTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderTransformWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShaderTransform);
  MF3D_WriteNewLine (inMetafilePtr);
  return MF3D_Matrix4x4Write (inMetafilePtr,
		       ((MF3DShaderTransformObj *) inObj)->shaderTransform);
}

/*==============================================================================
 *	MF3D_ObjShaderTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderTransformDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShaderTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjShaderUVTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderUVTransformReader (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj)
{
  MF3DShaderUVTransformObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_Matrix3x3Read (inMetafilePtr, returnObj->matrix);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjShaderUVTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderUVTransformWriter (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShaderUVTransform);
  MF3D_WriteNewLine (inMetafilePtr);
  return MF3D_Matrix3x3Write (inMetafilePtr,
			      ((MF3DShaderUVTransformObj *) inObj)->matrix);
}

/*==============================================================================
 *	MF3D_ObjShaderUVTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjShaderUVTransformDisposer (
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShaderUVTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTrimCurvesReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTrimCurvesReader (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr * outObj)
{
  MF3DTrimCurvesObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTrimCurvesWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTrimCurvesWriter (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTrimCurves);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTrimCurvesDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTrimCurvesDisposer (
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTrimCurves);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjImageClearColorReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageClearColorReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DImageClearColorObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_RGBColorRead (inMetafilePtr, &returnObj->clearColor);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjImageClearColorWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageClearColorWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjImageClearColor);
  return MF3D_RGBColorWrite (inMetafilePtr,
			   ((MF3DImageClearColorObjPtr) inObj)->clearColor);
}

/*==============================================================================
 *	MF3D_ObjImageClearColorDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageClearColorDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjImageClearColor);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjImageDimensionsReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageDimensionsReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DImageDimensionsObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->height);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjImageDimensionsWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageDimensionsWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3DImageDimensionsObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjImageDimensions);
  writeObj = (MF3DImageDimensionsObjPtr) inObj;

  result = MF3D_Uns32Write (inMetafilePtr, writeObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->height);

  return result;
}

/*==============================================================================
 *	MF3D_ObjImageDimensionsDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageDimensionsDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjImageDimensions);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjImageMaskReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageMaskReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DImageMaskObjPtr returnObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->rowBytes);

  if (result == kMF3DNoErr)
    {
      imageSize = returnObj->height * returnObj->rowBytes;

      returnObj->image = MF3D_Malloc (imageSize);
      if (returnObj->image == NULL)
	result = kMF3DErrOutOfMemory;
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataRead (inMetafilePtr, imageSize, returnObj->image);
      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->image);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjImageMaskWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageMaskWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DImageMaskObjPtr writeObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjImageMask);
  writeObj = (MF3DImageMaskObjPtr) inObj;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Uns32Write (inMetafilePtr, writeObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->rowBytes);

  if (result == kMF3DNoErr)
    {
      imageSize = writeObj->height * writeObj->rowBytes;
      result = MF3D_RawDataWrite (inMetafilePtr, imageSize, writeObj->image);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjImageMaskDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjImageMaskDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjImageMask);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DImageMaskObjPtr) inObj)->image);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}


#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MFOBJECT2__
#endif


/*==============================================================================
 *	MF3D_ObjAmbientCoefficientReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientCoefficientReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DAmbientCoefficientObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->ambientCoefficent);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjAmbientCoefficientWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientCoefficientWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAmbientCoefficient);

  return MF3D_Float32Write (inMetafilePtr,
		 ((MF3DAmbientCoefficientObjPtr) inObj)->ambientCoefficent);

}

/*==============================================================================
 *	MF3D_ObjAmbientCoefficientDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientCoefficientDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAmbientCoefficient);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDiffuseColorReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiffuseColorReader (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr * outObj)
{
  MF3DDiffuseColorObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_RGBColorRead (inMetafilePtr, &returnObj->diffuseColor);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjDiffuseColorWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiffuseColorWriter (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDiffuseColor);
  return MF3D_RGBColorWrite (inMetafilePtr,
			     ((MF3DDiffuseColorObjPtr) inObj)->diffuseColor);
}

/*==============================================================================
 *	MF3D_ObjDiffuseColorDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiffuseColorDisposer (
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDiffuseColor);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjHighlightStateReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStateReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DHighlightStateObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanRead (inMetafilePtr, &returnObj->highlighted);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjHighlightStateWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStateWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjHighlightState);
  return MF3D_BooleanWrite (inMetafilePtr,
			    ((MF3DHighlightStateObjPtr) inObj)->highlighted);

}

/*==============================================================================
 *	MF3D_ObjHighlightStateDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStateDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjHighlightState);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjNormalReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjNormalReader (
		       MF3D_FilePtr inMetafilePtr,
		       MF3DVoidObjPtr * outObj)
{
  MF3DNormalObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->normal);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjNormalWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjNormalWriter (
		       MF3D_FilePtr inMetafilePtr,
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjNormal);
  return MF3D_Vector3DWrite (inMetafilePtr, ((MF3DNormalObjPtr) inObj)->normal);
}

/*==============================================================================
 *	MF3D_ObjNormalDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjNormalDisposer (
			 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjNormal);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjShadingUVReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjShadingUVReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DShadingUVObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Param2DRead (inMetafilePtr, &returnObj->shadingUV);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjShadingUVWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjShadingUVWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShadingUV);
  return MF3D_Param2DWrite (inMetafilePtr,
			    ((MF3DShadingUVObjPtr) inObj)->shadingUV);
}

/*==============================================================================
 *	MF3D_ObjShadingUVDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjShadingUVDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjShadingUV);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSpecularColorReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularColorReader (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj)
{
  MF3DSpecularColorObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_RGBColorRead (inMetafilePtr, &returnObj->specularColor);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSpecularColorWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularColorWriter (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSpecularColor);
  return MF3D_RGBColorWrite (inMetafilePtr,
			  ((MF3DSpecularColorObjPtr) inObj)->specularColor);
}

/*==============================================================================
 *	MF3D_ObjSpecularColorDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularColorDisposer (
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSpecularColor);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSpecularControlReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularControlReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DSpecularControlObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->specularControl);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSpecularControlWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularControlWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSpecularControl);
  return MF3D_Float32Write (inMetafilePtr,
		      ((MF3DSpecularControlObjPtr) inObj)->specularControl);
}

/*==============================================================================
 *	MF3D_ObjSpecularControlDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpecularControlDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSpecularControl);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSurfaceTangentReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceTangentReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DSurfaceTangentObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->paramU);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->paramV);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSurfaceTangentWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceTangentWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3DSurfaceTangentObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjSurfaceTangent);
  writeObj = (MF3DSurfaceTangentObjPtr) inObj;

  result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->paramU);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->paramV);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSurfaceTangentDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceTangentDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSurfaceTangent);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSurfaceUVReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceUVReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DSurfaceUVObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Param2DRead (inMetafilePtr, &returnObj->surfaceUV);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSurfaceUVWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceUVWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSurfaceUV);
  return MF3D_Param2DWrite (inMetafilePtr,
			    ((MF3DSurfaceUVObjPtr) inObj)->surfaceUV);
}

/*==============================================================================
 *	MF3D_ObjSurfaceUVDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSurfaceUVDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSurfaceUV);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTransparencyColorReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTransparencyColorReader (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj)
{
  MF3DTransparencyColorObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_RGBColorRead (inMetafilePtr, &returnObj->transparency);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTransparencyColorWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTransparencyColorWriter (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTransparencyColor);
  return MF3D_RGBColorWrite (inMetafilePtr,
		       ((MF3DTransparencyColorObjPtr) inObj)->transparency);
}

/*==============================================================================
 *	MF3D_ObjTransparencyColorDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTransparencyColorDisposer (
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTransparencyColor);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererInteractiveReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererInteractiveReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DInteractiveRendererObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRendererInteractiveWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererInteractiveWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererInteractive);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererInteractiveDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererInteractiveDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererInteractive);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererGenericReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererGenericReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DGenericRendererObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRendererGenericWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererGenericWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererGeneric);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererGenericDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererGenericDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererGeneric);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererWireFrameReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererWireFrameReader (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj)
{
  MF3DWireFrameObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRendererWireFrameWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererWireFrameWriter (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererWireFrame);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererWireFrameDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererWireFrameDisposer (
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererWireFrame);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererZBufferReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererZBufferReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DZBufferObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRendererZBufferWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererZBufferWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererZBuffer);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRendererZBufferDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRendererZBufferDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRendererZBuffer);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjAttributeSetReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjAttributeSetReader (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr * outObj)
{
  MF3DAttributeSetObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjAttributeSetWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjAttributeSetWriter (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAttributeSet);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjAttributeSetDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjAttributeSetDisposer (
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAttributeSet);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjOrthographicCameraReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrthographicCameraReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DOrthographicCameraObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->left);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->top);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->right);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->bottom);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjOrthographicCameraWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrthographicCameraWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3DOrthographicCameraObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjOrthographicCamera);
  writeObj = (MF3DOrthographicCameraObjPtr) inObj;

  result = MF3D_Float32Write (inMetafilePtr, writeObj->left);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->top);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->right);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->bottom);

  return result;
}

/*==============================================================================
 *	MF3D_ObjOrthographicCameraDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrthographicCameraDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjOrthographicCamera);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjViewAngleAspectCameraReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewAngleAspectCameraReader (
				      MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj)
{
  MF3DViewAngleAspectCameraObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->fieldOfView);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->aspectRatioXtoY);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjViewAngleAspectCameraWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewAngleAspectCameraWriter (
				      MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj)
{
  MF3DViewAngleAspectCameraObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjViewAngleAspectCamera);
  writeObj = (MF3DViewAngleAspectCameraObjPtr) inObj;

  result = MF3D_Float32Write (inMetafilePtr, writeObj->fieldOfView);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->aspectRatioXtoY);

  return result;
}

/*==============================================================================
 *	MF3D_ObjViewAngleAspectCameraDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewAngleAspectCameraDisposer (
					MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjViewAngleAspectCamera);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjViewPlaneCameraReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewPlaneCameraReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DViewPlaneCameraObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->viewPlane);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->halfWidthAtViewPlane);

  if (result == kMF3DNoErr)
    {
      result = MF3D_Float32Read (inMetafilePtr,
				 &returnObj->halfHeightAtViewPlane);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->centerXOnViewPlane);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->centerYOnViewPlane);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjViewPlaneCameraWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewPlaneCameraWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3DViewPlaneCameraObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjViewPlaneCamera);
  writeObj = (MF3DViewPlaneCameraObjPtr) inObj;

  result = MF3D_Float32Write (inMetafilePtr, writeObj->viewPlane);

  if (result == kMF3DNoErr)
    {
      result = MF3D_Float32Write (inMetafilePtr,
				  writeObj->halfWidthAtViewPlane);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_Float32Write (inMetafilePtr,
				  writeObj->halfHeightAtViewPlane);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->centerXOnViewPlane);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->centerYOnViewPlane);

  return result;
}

/*==============================================================================
 *	MF3D_ObjViewPlaneCameraDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewPlaneCameraDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjViewPlaneCamera);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjBoxReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjBoxReader (
		    MF3D_FilePtr inMetafilePtr,
		    MF3DVoidObjPtr * outObj)
{
  MF3DBoxObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->box = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->box);

      returnObj->box->orientation.x = vector.x;
      returnObj->box->orientation.y = vector.y;
      returnObj->box->orientation.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->box->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->box->minorAxis);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Point3DRead (inMetafilePtr, &returnObj->box->origin);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjBoxWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjBoxWriter (
		    MF3D_FilePtr inMetafilePtr,
		    MF3DVoidObjPtr inObj)
{
  MF3DBoxObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjBox);
  writeObj = (MF3DBoxObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->box != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->box->orientation);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->box->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->box->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->box->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjBoxDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjBoxDisposer (
		      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBox);

  if (inObj != NULL)
    {
      if (((MF3DBoxObjPtr) inObj)->box != NULL)
	MF3D_Free (((MF3DBoxObjPtr) inObj)->box);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjConeReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjConeReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DConeObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->cone = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->cone);

      returnObj->cone->orientation.x = vector.x;
      returnObj->cone->orientation.y = vector.y;
      returnObj->cone->orientation.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->cone->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->cone->minorAxis);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Point3DRead (inMetafilePtr, &returnObj->cone->origin);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjConeWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjConeWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3DConeObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjCone);
  writeObj = (MF3DConeObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->cone != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->cone->orientation);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->cone->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->cone->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->cone->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjConeDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjConeDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCone);

  if (inObj != NULL)
    {
      if (((MF3DConeObjPtr) inObj)->cone != NULL)
	MF3D_Free (((MF3DConeObjPtr) inObj)->cone);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCylinderReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCylinderReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DCylinderObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->cylinder = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->cylinder);

      returnObj->cylinder->orientation.x = vector.x;
      returnObj->cylinder->orientation.y = vector.y;
      returnObj->cylinder->orientation.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->cylinder->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->cylinder->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Point3DRead (inMetafilePtr,
				     &returnObj->cylinder->origin);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MFOBJECT3__
#endif


/*==============================================================================
 *	MF3D_ObjCylinderWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCylinderWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3DCylinderObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjCylinder);
  writeObj = (MF3DCylinderObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->cylinder != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr,
				   writeObj->cylinder->orientation);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->cylinder->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->cylinder->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr,
				      writeObj->cylinder->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjCylinderDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCylinderDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCylinder);

  if (inObj != NULL)
    {
      if (((MF3DCylinderObjPtr) inObj)->cylinder != NULL)
	MF3D_Free (((MF3DCylinderObjPtr) inObj)->cylinder);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDiskReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiskReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DDiskObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->disk = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->disk);

      returnObj->disk->majorRadius.x = vector.x;
      returnObj->disk->majorRadius.y = vector.y;
      returnObj->disk->majorRadius.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->disk->minorRadius);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Point3DRead (inMetafilePtr, &returnObj->disk->origin);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjDiskWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiskWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3DDiskObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjDisk);
  writeObj = (MF3DDiskObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->disk != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr,
				   writeObj->disk->majorRadius);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->disk->minorRadius);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr,
				      writeObj->disk->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjDiskDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjDiskDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDisk);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DDiskObjPtr) inObj)->disk);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEllipseReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipseReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DEllipseObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->ellipse = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->ellipse);

      returnObj->ellipse->majorAxis.x = vector.x;
      returnObj->ellipse->majorAxis.y = vector.y;
      returnObj->ellipse->majorAxis.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->ellipse->minorAxis);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Point3DRead (inMetafilePtr, &returnObj->ellipse->origin);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjEllipseWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipseWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3DEllipseObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjEllipse);
  writeObj = (MF3DEllipseObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->ellipse != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr,
				   writeObj->ellipse->majorAxis);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->ellipse->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr,
				      writeObj->ellipse->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjEllipseDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipseDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEllipse);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DEllipseObjPtr) inObj)->ellipse);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjEllipsoidReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipsoidReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DEllipsoidObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->ellipsoid = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->ellipsoid);

      returnObj->ellipsoid->orientation.x = vector.x;
      returnObj->ellipsoid->orientation.y = vector.y;
      returnObj->ellipsoid->orientation.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->ellipsoid->majorRadius);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->ellipsoid->minorRadius);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Point3DRead (inMetafilePtr,
				     &returnObj->ellipsoid->origin);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjEllipsoidWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipsoidWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DEllipsoidObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjEllipsoid);
  writeObj = (MF3DEllipsoidObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->ellipsoid != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr,
				   writeObj->ellipsoid->orientation);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->ellipsoid->majorRadius);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->ellipsoid->minorRadius);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr,
				      writeObj->ellipsoid->origin);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjEllipsoidDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjEllipsoidDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjEllipsoid);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DEllipsoidObjPtr) inObj)->ellipsoid);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjGeneralPolygonHintReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonHintReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DGeneralPolygonHintObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_GeneralPolygonHintEnum,
			      &returnObj->shapeHint);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjGeneralPolygonHintWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonHintWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjGeneralPolygonHint);
  return MF3D_FlagsWrite (inMetafilePtr, gMF3D_GeneralPolygonHintEnum,
			  ((MF3DGeneralPolygonHintObjPtr) inObj)->shapeHint);
}

/*==============================================================================
 *	MF3D_ObjGeneralPolygonHintDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonHintDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjGeneralPolygonHint);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjGeneralPolygonReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DGeneralPolygonObjPtr returnObj;
  MF3DUns32 numContours;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nContours);

  if (result == kMF3DNoErr)
    {
      numContours = returnObj->nContours;
      if (numContours <= 0)
	result = kMF3DErrNumContoursIsZero;
    }

  /* Read polygon data */

  if (result == kMF3DNoErr)
    {
      MF3DPolygonDataPtr curContourPtr;

      MF3D_AllocArray (returnObj->polygons, numContours);

      curContourPtr = returnObj->polygons;
      while (result == kMF3DNoErr && numContours > 0)
	{
	  MF3DUns32 numVertices;

	  result = MF3D_Uns32Read (inMetafilePtr, &curContourPtr->nVertices);

	  if (result == kMF3DNoErr)
	    {
	      numVertices = curContourPtr->nVertices;
	      if (numVertices <= 2)
		result = kMF3DErrTooFewVertices;
	    }

	  if (result == kMF3DNoErr)
	    {
	      MF3DPoint3DPtr curVertexPtr;

	      MF3D_AllocArray (curContourPtr->vertices, numVertices);

	      curVertexPtr = curContourPtr->vertices;
	      while (result == kMF3DNoErr && numVertices > 0)
		{
		  result = MF3D_Point3DRead (inMetafilePtr, curVertexPtr);
		  --numVertices;
		  ++curVertexPtr;
		}

	      if (result != kMF3DNoErr)
		MF3D_Free (curContourPtr->vertices);
	    }

	  --numContours;
	  ++curContourPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->polygons);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjGeneralPolygonWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3DGeneralPolygonObjPtr writeObj;
  MF3DUns32 numContours;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjGeneralPolygon);
  writeObj = (MF3DGeneralPolygonObjPtr) inObj;

  result = kMF3DNoErr;

  numContours = writeObj->nContours;
  if (numContours <= 0)
    result = kMF3DErrNumContoursIsZero;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, numContours);
    }

  /* Write polygon data */

  if (result == kMF3DNoErr)
    {
      MF3DPolygonDataPtr curContourPtr;
      MF3DUns32 contourNum;

      MF3D_CommentStrWrite (inMetafilePtr, "nContours");

      curContourPtr = writeObj->polygons;
      for (contourNum = 0;
	   result == kMF3DNoErr && contourNum < numContours;
	   ++contourNum)
	{
	  MF3DUns32 numVertices;

	  numVertices = curContourPtr->nVertices;
	  if (numVertices <= 2)
	    result = kMF3DErrTooFewVertices;

	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_Uns32Write (inMetafilePtr,
					curContourPtr->nVertices);
	    }

	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, contourNum);

	  if (result == kMF3DNoErr)
	    {
	      MF3DPoint3DPtr curVertexPtr;
	      MF3DUns32 vertexNum;

	      curVertexPtr = curContourPtr->vertices;
	      for (vertexNum = 0;
		   result == kMF3DNoErr && vertexNum < numVertices;
		   ++vertexNum)
		{
		  result = MF3D_Point3DWrite (inMetafilePtr, *curVertexPtr++);
		  if (result == kMF3DNoErr)
		    MF3D_WriteNewLine (inMetafilePtr);
		}
	    }
	  ++curContourPtr;
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjGeneralPolygonDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjGeneralPolygonDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3DGeneralPolygonObjPtr polygonObj;

  MF3D_VerifyObjectType (inObj, kMF3DObjGeneralPolygon);

  polygonObj = (MF3DGeneralPolygonObjPtr) inObj;

  if (polygonObj != NULL)
    {
      if (polygonObj->polygons != NULL)
	{
	  MF3DUns32 numContours;
	  MF3DPolygonDataPtr contourPtr;

	  contourPtr = polygonObj->polygons;
	  for (numContours = polygonObj->nContours; numContours > 0;
	       --numContours)
	    {
	      MF3D_Free (contourPtr->vertices);
	      ++contourPtr;
	    }
	  MF3D_Free (polygonObj->polygons);
	}

      MF3D_Free (polygonObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLineReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjLineReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DLineObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->start);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->end);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjLineWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjLineWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3DLineObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjLine);
  writeObj = (MF3DLineObjPtr) inObj;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->start);

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Point3DWrite (inMetafilePtr, writeObj->end);
    }

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (inMetafilePtr);

  return result;
}

/*==============================================================================
 *	MF3D_ObjLineDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjLineDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLine);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMarkerReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMarkerReader (
		       MF3D_FilePtr inMetafilePtr,

		       MF3DVoidObjPtr * outObj)
{
  MF3DMarkerObjPtr returnObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->rowBytes);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Read (inMetafilePtr, &returnObj->xOffset);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Read (inMetafilePtr, &returnObj->yOffset);

  if (result == kMF3DNoErr)
    {
      imageSize = returnObj->height * returnObj->rowBytes;
      if (imageSize == 0)
	result = kMF3DErrWidthOrHeightIsZero;
    }

  if (result == kMF3DNoErr)
    {
      returnObj->data = MF3D_Malloc (imageSize);
      if (returnObj->data == NULL)
	result = kMF3DErrOutOfMemory;
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataRead (inMetafilePtr, imageSize, returnObj->data);
      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->data);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjMarkerWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMarkerWriter (
		       MF3D_FilePtr inMetafilePtr,
		       MF3DVoidObjPtr inObj)
{
  MF3DMarkerObjPtr writeObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjMarker);
  writeObj = (MF3DMarkerObjPtr) inObj;

  result = kMF3DNoErr;

  imageSize = writeObj->height * writeObj->rowBytes;
  if (imageSize == 0)
    result = kMF3DErrWidthOrHeightIsZero;

  if (result == kMF3DNoErr)
    result = MF3D_Point3DWrite (inMetafilePtr, writeObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->rowBytes);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Write (inMetafilePtr, writeObj->xOffset);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Write (inMetafilePtr, writeObj->yOffset);

  if (result == kMF3DNoErr)
    result = MF3D_RawDataWrite (inMetafilePtr, imageSize, writeObj->data);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMarkerDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMarkerDisposer (
			 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMarker);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DMarkerObjPtr) inObj)->data);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMeshReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshReader (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr * outObj)
{
  MF3DMeshObjPtr returnObj;
  MF3DUns32 numVertices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nVertices);

  if (result == kMF3DNoErr)
    {
      numVertices = returnObj->nVertices;
      if (numVertices <= 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;

      MF3D_AllocArray (returnObj->vertices, numVertices);

      curVertexPtr = returnObj->vertices;
      while (result == kMF3DNoErr && numVertices > 0)
	{
	  result = MF3D_Point3DRead (inMetafilePtr, curVertexPtr);
	  --numVertices;
	  ++curVertexPtr;
	}

      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->vertices);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nFaces);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nContours);

  if (result == kMF3DNoErr)
    {
      MF3DUns32 numMeshFaces;

      numMeshFaces = returnObj->nFaces + returnObj->nContours;

      if (numMeshFaces == 0)
	{
	  returnObj->faces = NULL;
	}
      else
	{
	  MF3DMeshFacePtr curMeshFacePtr;

	  MF3D_AllocArray (returnObj->faces, numMeshFaces);

	  curMeshFacePtr = returnObj->faces;
	  while (result == kMF3DNoErr && numMeshFaces > 0)
	    {
	      MF3DUns32 numFaceVertexIndices;

	      result = MF3D_Uns32Read (inMetafilePtr,
				       &curMeshFacePtr->nFaceVertexIndices);

	      if (result == kMF3DNoErr)
		{
		  numFaceVertexIndices = curMeshFacePtr->nFaceVertexIndices;
		  if (numFaceVertexIndices <= 2)
		    result = kMF3DErrTooFewVertices;
		}

	      if (result == kMF3DNoErr)
		{
		  MF3DUns32Ptr curVertexPtr;

		  MF3D_AllocArray (curMeshFacePtr->faceVertexIndices,
				   numFaceVertexIndices);

		  curVertexPtr = curMeshFacePtr->faceVertexIndices;
		  while (result == kMF3DNoErr && numFaceVertexIndices > 0)
		    {
		      result = MF3D_Uns32Read (inMetafilePtr, curVertexPtr);
		      --numFaceVertexIndices;
		      ++curVertexPtr;
		    }

		  if (result != kMF3DNoErr)
		    MF3D_Free (curMeshFacePtr->faceVertexIndices);
		}

	      --numMeshFaces;
	      ++curMeshFacePtr;
	    }

	  if (result != kMF3DNoErr)
	    MF3D_Free (returnObj->faces);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshWriter (
		     MF3D_FilePtr inMetafilePtr,
		     MF3DVoidObjPtr inObj)
{
  MF3DMeshObjPtr meshObj;
  MF3DUns32 nVertices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjMesh);
  meshObj = (MF3DMeshObjPtr) inObj;

  result = kMF3DNoErr;

  MF3D_WriteNewLine (inMetafilePtr);

  if (result == kMF3DNoErr)
    {
      nVertices = meshObj->nVertices;
      if (nVertices <= 2)
	result = kMF3DErrTooFewVertices;
    }

  result = MF3D_Uns32Write (inMetafilePtr, nVertices);

  if (result == kMF3DNoErr)
    {
      MF3DUns32 vertexNum;
      MF3DPoint3DPtr curVertexPtr;

      MF3D_CommentStrWrite (inMetafilePtr, "nVertices");

      curVertexPtr = meshObj->vertices;
      for (vertexNum = 0;
	   result == kMF3DNoErr && vertexNum < nVertices;
	   ++vertexNum)
	{
	  result = MF3D_Point3DWrite (inMetafilePtr, *curVertexPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, vertexNum);
	}
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, meshObj->nFaces);

  if (result == kMF3DNoErr)
    {
      MF3D_CommentStrWrite (inMetafilePtr, "nFaces");
      result = MF3D_Uns32Write (inMetafilePtr, meshObj->nContours);
    }

  if (result == kMF3DNoErr)
    {
      MF3DUns32 faceNum, nMeshFaces;
      MF3DMeshFacePtr curMeshFacePtr;

      MF3D_CommentStrWrite (inMetafilePtr, "nContours");

      nMeshFaces = meshObj->nFaces + meshObj->nContours;
      curMeshFacePtr = meshObj->faces;
      for (faceNum = 0;
	   result == kMF3DNoErr && faceNum < nMeshFaces;
	   ++faceNum)
	{
	  MF3DUns32 faceVertexNum, nFaceVertexIndices;
	  MF3DUns32Ptr curFaceVertexPtr;

	  nFaceVertexIndices = curMeshFacePtr->nFaceVertexIndices;
	  if (nFaceVertexIndices <= 2)
	    result = kMF3DErrTooFewVertices;
	  else
	    result = MF3D_Uns32Write (inMetafilePtr, nFaceVertexIndices);

	  curFaceVertexPtr = curMeshFacePtr->faceVertexIndices;
	  for (faceVertexNum = 0;
	       result == kMF3DNoErr && faceVertexNum < nFaceVertexIndices;
	       ++faceVertexNum)
	    {
	      result = MF3D_Uns32Write (inMetafilePtr, *curFaceVertexPtr++);
	    }
	  MF3D_CommentNumWrite (inMetafilePtr, faceNum);
	  ++curMeshFacePtr;
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjMeshDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMeshDisposer (
		       MF3DVoidObjPtr inObj)
{
  MF3DMeshObjPtr meshObj;

  MF3D_VerifyObjectType (inObj, kMF3DObjMesh);

  meshObj = (MF3DMeshObjPtr) inObj;

  if (meshObj != NULL)
    {
      if (meshObj->faces != NULL)
	{
	  MF3DUns32 numMeshFaces;
	  MF3DMeshFacePtr curMeshFacePtr;

	  curMeshFacePtr = meshObj->faces;
	  for (numMeshFaces = meshObj->nFaces + meshObj->nContours;
	       numMeshFaces > 0; --numMeshFaces)
	    {
	      MF3D_Free (curMeshFacePtr->faceVertexIndices);
	      ++curMeshFacePtr;
	    }
	  MF3D_Free (meshObj->faces);
	}
      MF3D_Free (meshObj->vertices);
      MF3D_Free (meshObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjNURBCurveReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurveReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DNURBCurveObjPtr returnObj;
  MF3DUns32 order;
  MF3DUns32 numPoints;
  MF3DUns32 numKnots;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->order);

  if (result == kMF3DNoErr)
    {
      order = returnObj->order;
      if (order < 2)
	result = kMF3DErrOrderTooSmall;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nPoints);

  if (result == kMF3DNoErr)
    {
      numPoints = returnObj->nPoints;
      numKnots = numPoints + order;
      if (numPoints < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint4DPtr curPointPtr;

      MF3D_AllocArray (returnObj->points, numPoints);

      curPointPtr = returnObj->points;
      while (result == kMF3DNoErr && numPoints > 0)
	{
	  result = MF3D_RationalPoint4DRead (inMetafilePtr, curPointPtr);
	  --numPoints;
	  ++curPointPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->points);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;

      MF3D_AllocArray (returnObj->knots, numKnots);

      curKnotPtr = returnObj->knots;
      while (result == kMF3DNoErr && numKnots > 0)
	{
	  result = MF3D_Float32Read (inMetafilePtr, curKnotPtr);
	  --numKnots;
	  ++curKnotPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->knots);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBCurveWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurveWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DNURBCurveObjPtr writeObj;
  MF3DUns32 order;
  MF3DUns32 numPoints;
  MF3DUns32 numKnots;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjNURBCurve);
  writeObj = (MF3DNURBCurveObjPtr) inObj;

  result = kMF3DNoErr;

  numPoints = writeObj->nPoints;
  order = writeObj->order;

  if (order < 2)
    result = kMF3DErrOrderTooSmall;

  if (result == kMF3DNoErr)
    {
      numKnots = numPoints + order;
      if (numPoints < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, order);
    }

  if (result == kMF3DNoErr)
    {
      MF3D_CommentStrWrite (inMetafilePtr, "order");
      result = MF3D_Uns32Write (inMetafilePtr, numPoints);
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "nPoints");

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint4DPtr curPointPtr;
      MF3DUns32 pointNum;

      curPointPtr = writeObj->points;
      for (pointNum = 0;
	   result == kMF3DNoErr && pointNum < numPoints;
	   ++pointNum)
	{
	  result = MF3D_RationalPoint4DWrite (inMetafilePtr, *curPointPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, pointNum);
	}
    }

  if (result == kMF3DNoErr)
    MF3D_CommentStrWrite (inMetafilePtr, "knots");

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;
      MF3DUns32 knotNum;

      curKnotPtr = writeObj->knots;
      for (knotNum = 0;
	   result == kMF3DNoErr && knotNum < numKnots;
	   ++knotNum)
	{
	  result = MF3D_Float32Write (inMetafilePtr, *curKnotPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, knotNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBCurveDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBCurveDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjNURBCurve);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DNURBCurveObjPtr) inObj)->points);
      MF3D_Free (((MF3DNURBCurveObjPtr) inObj)->knots);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjNURBPatchReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBPatchReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DNurbPatchObjPtr returnObj;
  MF3DUns32 uOrder;
  MF3DUns32 vOrder;
  MF3DUns32 numColumns;
  MF3DUns32 numRows;
  MF3DUns32 numPoints;
  MF3DUns32 numUKnots;
  MF3DUns32 numVKnots;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->uOrder);
  if (result == kMF3DNoErr)
    {
      uOrder = returnObj->uOrder;
      if (uOrder < 2)
	result = kMF3DErrOrderTooSmall;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->vOrder);
  if (result == kMF3DNoErr)
    {
      vOrder = returnObj->vOrder;
      if (vOrder < 2)
	result = kMF3DErrOrderTooSmall;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->numColumns);
  if (result == kMF3DNoErr)
    {
      numColumns = returnObj->numColumns;
      if (numColumns < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->numRows);
  if (result == kMF3DNoErr)
    {
      numRows = returnObj->numRows;
      if (numRows < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint4DPtr curPointPtr;

      numPoints = numColumns * numRows;
      numUKnots = uOrder + numColumns;
      numVKnots = vOrder + numRows;

      MF3D_AllocArray (returnObj->points, numPoints);

      curPointPtr = returnObj->points;
      while (result == kMF3DNoErr && numPoints > 0)
	{
	  result = MF3D_RationalPoint4DRead (inMetafilePtr, curPointPtr);
	  --numPoints;
	  ++curPointPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->points);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;

      MF3D_AllocArray (returnObj->uKnots, numUKnots);

      curKnotPtr = returnObj->uKnots;
      while (result == kMF3DNoErr && numUKnots > 0)
	{
	  result = MF3D_Float32Read (inMetafilePtr, curKnotPtr);
	  --numUKnots;
	  ++curKnotPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->uKnots);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;

      MF3D_AllocArray (returnObj->vKnots, numVKnots);

      curKnotPtr = returnObj->vKnots;
      while (result == kMF3DNoErr && numVKnots > 0)
	{
	  result = MF3D_Float32Read (inMetafilePtr, curKnotPtr);
	  --numVKnots;
	  ++curKnotPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->vKnots);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBPatchWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBPatchWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DNurbPatchObjPtr writeObj;
  MF3DUns32 uOrder;
  MF3DUns32 vOrder;
  MF3DUns32 numColumns;
  MF3DUns32 numRows;
  MF3DUns32 numPoints;
  MF3DUns32 numUKnots;
  MF3DUns32 numVKnots;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjNURBPatch);
  writeObj = (MF3DNurbPatchObjPtr) inObj;

  result = kMF3DNoErr;

  uOrder = writeObj->uOrder;
  if (uOrder < 2)
    result = kMF3DErrOrderTooSmall;

  if (result == kMF3DNoErr)
    {
      vOrder = writeObj->vOrder;
      if (vOrder < 2)
	result = kMF3DErrOrderTooSmall;
    }

  if (result == kMF3DNoErr)
    {
      numColumns = writeObj->numColumns;
      if (numColumns < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      numRows = writeObj->numRows;
      if (numRows < 2)
	result = kMF3DErrTooFewPoints;
    }

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, uOrder);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, vOrder);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, numColumns);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, numRows);

  if (result == kMF3DNoErr)
    {
      MF3DRationalPoint4DPtr curPointPtr;
      MF3DUns32 pointNum;

      MF3D_WriteNewLine (inMetafilePtr);

      numPoints = numColumns * numRows;
      numUKnots = uOrder + numColumns;
      numVKnots = vOrder + numRows;

      curPointPtr = writeObj->points;
      for (pointNum = 0;
	   result == kMF3DNoErr && pointNum < numPoints;
	   ++pointNum)
	{
	  result = MF3D_RationalPoint4DWrite (inMetafilePtr, *curPointPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, pointNum);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;
      MF3DUns32 knotNum;

      MF3D_WriteNewLine (inMetafilePtr);
      MF3D_CommentStrWrite (inMetafilePtr, "uKnots");
      curKnotPtr = writeObj->uKnots;
      for (knotNum = 0;
	   result == kMF3DNoErr && knotNum < numUKnots;
	   ++knotNum)
	{
	  result = MF3D_Float32Write (inMetafilePtr, *curKnotPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, knotNum);
	}
    }

  if (result == kMF3DNoErr)
    {
      MF3DKnotPtr curKnotPtr;
      MF3DUns32 knotNum;

      MF3D_WriteNewLine (inMetafilePtr);
      MF3D_CommentStrWrite (inMetafilePtr, "vKnots");
      curKnotPtr = writeObj->vKnots;
      for (knotNum = 0;
	   result == kMF3DNoErr && knotNum < numVKnots;
	   ++knotNum)
	{
	  result = MF3D_Float32Write (inMetafilePtr, *curKnotPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, knotNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjNURBPatchDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjNURBPatchDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjNURBPatch);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DNurbPatchObjPtr) inObj)->points);
      MF3D_Free (((MF3DNurbPatchObjPtr) inObj)->uKnots);
      MF3D_Free (((MF3DNurbPatchObjPtr) inObj)->vKnots);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPointReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointReader (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr * outObj)
{
  MF3DPointObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->point);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjPointWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointWriter (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPoint);
  return MF3D_Point3DWrite (inMetafilePtr,
			    ((MF3DPointObjPtr) inObj)->point);
}

/*==============================================================================
 *	MF3D_ObjPointDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointDisposer (
			MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPoint);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPolygonReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolygonReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DPolygonObjPtr returnObj;
  MF3DUns32 numVertices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nVertices);

  if (result == kMF3DNoErr)
    {
      numVertices = returnObj->nVertices;
      if (numVertices <= 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;

      MF3D_AllocArray (returnObj->vertices, numVertices);

      curVertexPtr = returnObj->vertices;
      while (result == kMF3DNoErr && numVertices > 0)
	{
	  result = MF3D_Point3DRead (inMetafilePtr, curVertexPtr);
	  --numVertices;
	  ++curVertexPtr;
	}

      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->vertices);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjPolygonWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolygonWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3DPolygonObjPtr writeObj;
  MF3DUns32 numVertices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjPolygon);
  writeObj = (MF3DPolygonObjPtr) inObj;

  result = kMF3DNoErr;

  numVertices = writeObj->nVertices;
  if (numVertices <= 2)
    result = kMF3DErrTooFewVertices;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, numVertices);
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;
      MF3DUns32 vertexNum;

      MF3D_CommentStrWrite (inMetafilePtr, "nVertices");
      curVertexPtr = writeObj->vertices;
      for (vertexNum = 0;
	   result == kMF3DNoErr && vertexNum < numVertices;
	   ++vertexNum)
	{
	  result = MF3D_Point3DWrite (inMetafilePtr, *curVertexPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, vertexNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjPolygonDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolygonDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPolygon);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DPolygonObjPtr) inObj)->vertices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPolyLineReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolyLineReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DPolyLineObjPtr returnObj;
  MF3DUns32 numVertices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->nVertices);

  if (result == kMF3DNoErr)
    {
      numVertices = returnObj->nVertices;
      if (numVertices <= 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;

      MF3D_AllocArray (returnObj->vertices, numVertices);

      curVertexPtr = returnObj->vertices;
      while (result == kMF3DNoErr && numVertices > 0)
	{
	  result = MF3D_Point3DRead (inMetafilePtr, curVertexPtr);
	  --numVertices;
	  ++curVertexPtr;
	}

      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->vertices);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjPolyLineWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolyLineWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3DPolyLineObjPtr writeObj;
  MF3DUns32 numVertices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjPolyLine);
  writeObj = (MF3DPolyLineObjPtr) inObj;

  result = kMF3DNoErr;

  numVertices = writeObj->nVertices;
  if (numVertices <= 2)
    result = kMF3DErrTooFewVertices;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, writeObj->nVertices);
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;
      MF3DUns32 vertexNum;

      MF3D_CommentStrWrite (inMetafilePtr, "nVertices");
      curVertexPtr = writeObj->vertices;
      for (vertexNum = 0;
	   result == kMF3DNoErr && vertexNum < numVertices;
	   ++vertexNum)
	{
	  result = MF3D_Point3DWrite (inMetafilePtr, *curVertexPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, vertexNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjPolyLineDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPolyLineDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPolyLine);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DPolyLineObjPtr) inObj)->vertices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTorusReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTorusReader (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr * outObj)
{
  MF3DTorusObjPtr returnObj;
  MF3DVector3D vector;
  MF3DErr numberExists;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->torus = NULL;

      numberExists = MF3D_Vector3DRead (inMetafilePtr, &vector);
    }

  if (result == kMF3DNoErr && numberExists == kMF3DNoErr)
    {
      MF3D_Allocate (returnObj->torus);

      returnObj->torus->orientation.x = vector.x;
      returnObj->torus->orientation.y = vector.y;
      returnObj->torus->orientation.z = vector.z;

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->torus->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  result = MF3D_Vector3DRead (inMetafilePtr,
				      &returnObj->torus->minorAxis);
	}

      if (result == kMF3DNoErr)
	result = MF3D_Point3DRead (inMetafilePtr, &returnObj->torus->origin);

      if (result == kMF3DNoErr)
	result = MF3D_Float32Read (inMetafilePtr, &returnObj->torus->ratio);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjTorusWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTorusWriter (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr inObj)
{
  MF3DTorusObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjTorus);
  writeObj = (MF3DTorusObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->torus != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Vector3DWrite (inMetafilePtr,
				   writeObj->torus->orientation);

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->torus->majorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Vector3DWrite (inMetafilePtr,
				       writeObj->torus->minorAxis);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Point3DWrite (inMetafilePtr,
				      writeObj->torus->origin);
	}

      if (result == kMF3DNoErr)
	{
	  MF3D_WriteNewLine (inMetafilePtr);
	  result = MF3D_Float32Write (inMetafilePtr,
				      writeObj->torus->ratio);
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (inMetafilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTorusDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTorusDisposer (
			MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTorus);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DTorusObjPtr) inObj)->torus);
      MF3D_Free (inObj);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTriangleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriangleReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DTriangleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->vertex1);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->vertex2);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->vertex3);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjTriangleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriangleWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3DTriangleObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjTriangle);
  writeObj = (MF3DTriangleObjPtr) inObj;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->vertex1);

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Point3DWrite (inMetafilePtr, writeObj->vertex2);
    }

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Point3DWrite (inMetafilePtr, writeObj->vertex3);
    }

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (inMetafilePtr);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTriangleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriangleDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTriangle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTriGridReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriGridReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DTriGridObjPtr returnObj;
  MF3DUns32 numRows, numColumns;
  MF3DUns32 numVertices;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->numRows);
  if (result == kMF3DNoErr)
    {
      numRows = returnObj->numRows;
      if (numRows < 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->numColumns);
  if (result == kMF3DNoErr)
    {
      numColumns = returnObj->numColumns;
      if (numColumns < 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;

      numVertices = numColumns * numRows;

      MF3D_AllocArray (returnObj->vertices, numVertices);

      curVertexPtr = returnObj->vertices;
      while (result == kMF3DNoErr && numVertices > 0)
	{
	  result = MF3D_Point3DRead (inMetafilePtr, curVertexPtr);
	  --numVertices;
	  ++curVertexPtr;
	}

      if (result != kMF3DNoErr)
	{
	  MF3D_Free (returnObj->vertices);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjTriGridWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriGridWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3DTriGridObjPtr writeObj;
  MF3DUns32 numRows, numColumns;
  MF3DUns32 numVertices;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjTriGrid);
  writeObj = (MF3DTriGridObjPtr) inObj;

  result = kMF3DNoErr;

  numRows = writeObj->numRows;
  if (numRows < 2)
    result = kMF3DErrTooFewVertices;

  if (result == kMF3DNoErr)
    {
      numColumns = writeObj->numColumns;
      if (numColumns < 2)
	result = kMF3DErrTooFewVertices;
    }

  if (result == kMF3DNoErr)
    {
      MF3D_CommentStrWrite (inMetafilePtr, "numRows");
      result = MF3D_Uns32Write (inMetafilePtr, numRows);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, numColumns);

  if (result == kMF3DNoErr)
    {
      MF3DPoint3DPtr curVertexPtr;
      MF3DUns32 vertexNum;

      numVertices = numColumns * numRows;

      MF3D_CommentStrWrite (inMetafilePtr, "numColumns");

      curVertexPtr = writeObj->vertices;
      for (vertexNum = 0;
	   result == kMF3DNoErr && vertexNum < numVertices;
	   ++vertexNum)
	{
	  result = MF3D_Point3DWrite (inMetafilePtr, *curVertexPtr++);
	  if (result == kMF3DNoErr)
	    MF3D_CommentNumWrite (inMetafilePtr, vertexNum);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTriGridDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTriGridDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTriGrid);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DTriGridObjPtr) inObj)->vertices);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjGroupReader (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr * outObj)
{
  MF3DGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjGroupWriter (
		      MF3D_FilePtr inMetafilePtr,
		      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjGroupDisposer (
			MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDisplayGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupReader (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr * outObj)
{
  MF3DDisplayGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjDisplayGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupWriter (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDisplayGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDisplayGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjDisplayGroupDisposer (
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDisplayGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjIOProxyDisplayGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjIOProxyDisplayGroupReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DIOProxyDisplayGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}


/*==============================================================================
 *	MF3D_ObjIOProxyDisplayGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjIOProxyDisplayGroupWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjIOProxyDisplayGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjIOProxyDisplayGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjIOProxyDisplayGroupDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjIOProxyDisplayGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjOrderedDisplayGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrderedDisplayGroupReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DOrderedDisplayGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjOrderedDisplayGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrderedDisplayGroupWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjOrderedDisplayGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjOrderedDisplayGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrderedDisplayGroupDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjOrderedDisplayGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjInfoGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjInfoGroupReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DInfoGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjInfoGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjInfoGroupWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjInfoGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjInfoGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjInfoGroupDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjInfoGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLightGroupReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightGroupReader (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr * outObj)
{
  MF3DLightGroupObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjLightGroupWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightGroupWriter (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLightGroup);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLightGroupDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjLightGroupDisposer (
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLightGroup);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjAmbientLightReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientLightReader (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr * outObj)
{
  MF3DAmbientLightObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjAmbientLightWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientLightWriter (
			     MF3D_FilePtr inMetafilePtr,
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAmbientLight);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjAmbientLightDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjAmbientLightDisposer (
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjAmbientLight);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjDirectionalLightReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjDirectionalLightReader (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj)
{
  MF3DDirectionalLightObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->direction);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanRead (inMetafilePtr, &returnObj->castsShadows);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjDirectionalLightWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjDirectionalLightWriter (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj)
{
  MF3DDirectionalLightObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjDirectionalLight);
  writeObj = (MF3DDirectionalLightObjPtr) inObj;

  result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->direction);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanWrite (inMetafilePtr, writeObj->castsShadows);

  return result;
}

/*==============================================================================
 *	MF3D_ObjDirectionalLightDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjDirectionalLightDisposer (
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjDirectionalLight);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPointLightReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointLightReader (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr * outObj)
{
  MF3DPointLightObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c0);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c1);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c2);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanRead (inMetafilePtr, &returnObj->castsShadows);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPointLightWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointLightWriter (
			   MF3D_FilePtr inMetafilePtr,
			   MF3DVoidObjPtr inObj)
{
  MF3DPointLightObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjPointLight);
  writeObj = (MF3DPointLightObjPtr) inObj;

  result = kMF3DNoErr;

  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c0);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c1);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c2);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanWrite (inMetafilePtr, writeObj->castsShadows);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPointLightDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPointLightDisposer (
			     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPointLight);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSpotLightReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpotLightReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DSpotLightObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->orientation);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanRead (inMetafilePtr, &returnObj->castsShadows);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c0);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c1);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->attenuation.c2);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->hotAngle);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->outerAngle);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_FallOffEnum,
			      &returnObj->fallOff);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSpotLightWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpotLightWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3DSpotLightObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjSpotLight);
  writeObj = (MF3DSpotLightObjPtr) inObj;

  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->location);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->orientation);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanWrite (inMetafilePtr, writeObj->castsShadows);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c0);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c1);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->attenuation.c2);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->hotAngle);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->outerAngle);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_FallOffEnum,
			       writeObj->fallOff);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjSpotLightDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSpotLightDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSpotLight);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLambertIlluminationReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjLambertIlluminationReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DLambertIlluminationObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjLambertIlluminationWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjLambertIlluminationWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLambertIllumination);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjLambertIlluminationDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjLambertIlluminationDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjLambertIllumination);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPhongIlluminationReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPhongIlluminationReader (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj)
{
  MF3DPhongIlluminationObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPhongIlluminationWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPhongIlluminationWriter (
				  MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPhongIllumination);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPhongIlluminationDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPhongIlluminationDisposer (
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPhongIllumination);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTextureShaderReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTextureShaderReader (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj)
{
  MF3DTextureShaderObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTextureShaderWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTextureShaderWriter (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTextureShader);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTextureShaderDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTextureShaderDisposer (
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTextureShader);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}


#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MFOBJECT4__
#endif


/*==============================================================================
 *	MF3D_ObjBackfacingStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjBackfacingStyleReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DBackfacingStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_BackfacingEnum,
			      &returnObj->backfacing);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjBackfacingStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjBackfacingStyleWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBackfacingStyle);
  return MF3D_EnumWrite (inMetafilePtr, gMF3D_BackfacingEnum,
			 ((MF3DBackfacingStyleObjPtr) inObj)->backfacing);
}

/*==============================================================================
 *	MF3D_ObjBackfacingStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjBackfacingStyleDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjBackfacingStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjFillStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjFillStyleReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DFillStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_FillStyleEnum,
			      &returnObj->fillStyle);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjFillStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjFillStyleWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjFillStyle);
  return MF3D_EnumWrite (inMetafilePtr, gMF3D_FillStyleEnum,
			 ((MF3DFillStyleObjPtr) inObj)->fillStyle);
}

/*==============================================================================
 *	MF3D_ObjFillStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjFillStyleDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjFillStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjHighlightStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStyleReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DHighlightStyleObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjHighlightStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStyleWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjHighlightStyle);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjHighlightStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjHighlightStyleDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjHighlightStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjInterpolationStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjInterpolationStyleReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DInterpolationStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_InterpolationStyleEnum,
			      &returnObj->interpolationStyle);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjInterpolationStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjInterpolationStyleWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjInterpolationStyle);
  return MF3D_EnumWrite (inMetafilePtr, gMF3D_InterpolationStyleEnum,
		((MF3DInterpolationStyleObjPtr) inObj)->interpolationStyle);
}

/*==============================================================================
 *	MF3D_ObjInterpolationStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjInterpolationStyleDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjInterpolationStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjOrientationStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrientationStyleReader (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj)
{
  MF3DOrientationStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_OrientationStyleEnum,
			      &returnObj->orientationStyle);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjOrientationStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrientationStyleWriter (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjOrientationStyle);
  return MF3D_EnumWrite (inMetafilePtr, gMF3D_OrientationStyleEnum,
		    ((MF3DOrientationStyleObjPtr) inObj)->orientationStyle);
}

/*==============================================================================
 *	MF3D_ObjOrientationStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjOrientationStyleDisposer (
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjOrientationStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPickIDStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPickIDStyleReader (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr * outObj)
{
  MF3DPickIDStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->id);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPickIDStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPickIDStyleWriter (
			    MF3D_FilePtr inMetafilePtr,
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPickIDStyle);
  return MF3D_Uns32Write (inMetafilePtr, ((MF3DPickIDStyleObjPtr) inObj)->id);
}

/*==============================================================================
 *	MF3D_ObjPickIDStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPickIDStyleDisposer (
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPickIDStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPickPartsStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPickPartsStyleReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DPickPartsStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_FlagsRead (inMetafilePtr, gMF3D_PickPartsFlags,
			       &returnObj->pickParts);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPickPartsStyleWriter
 *==============================================================================
 */
MF3DErr

MF3D_ObjPickPartsStyleWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPickPartsStyle);
  return MF3D_FlagsWrite (inMetafilePtr, gMF3D_PickPartsFlags,
			  ((MF3DPickPartsStyleObjPtr) inObj)->pickParts);
}

/*==============================================================================
 *	MF3D_ObjPickPartsStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPickPartsStyleDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPickPartsStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjReceiveShadowsStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjReceiveShadowsStyleReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DReceiveShadowsStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_BooleanRead (inMetafilePtr, &returnObj->receiveShadows);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjReceiveShadowsStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjReceiveShadowsStyleWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjReceiveShadowsStyle);
  return MF3D_BooleanWrite (inMetafilePtr,
		   ((MF3DReceiveShadowsStyleObjPtr) inObj)->receiveShadows);
}

/*==============================================================================
 *	MF3D_ObjReceiveShadowsStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjReceiveShadowsStyleDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjReceiveShadowsStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjSubdivisionStyleReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjSubdivisionStyleReader (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj)
{
  MF3DSubdivisionStyleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_SubdivisionMethodEnum,
			      &returnObj->subdivisionMethod);
    }

  if (result == kMF3DNoErr)
    {
      if (returnObj->subdivisionMethod == MF3DSubdivisionConstant)
	{
	  result = MF3D_Uns32Read (inMetafilePtr,
			       &returnObj->subdivisionData.constant.value1);
	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_Uns32Read (inMetafilePtr,
			       &returnObj->subdivisionData.constant.value2);
	    }
	}
      else
	{
	  MFASSERT (returnObj->subdivisionMethod ==
		    MF3DSubdivisionWorldSpace ||
		    returnObj->subdivisionMethod ==
		    MF3DSubdivisionScreenSpace);

	  result = MF3D_Float32Read (inMetafilePtr,
				  &returnObj->subdivisionData.space.value1);
	}
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjSubdivisionStyleWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjSubdivisionStyleWriter (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj)
{
  MF3DSubdivisionStyleObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjSubdivisionStyle);
  writeObj = (MF3DSubdivisionStyleObjPtr) inObj;

  result = MF3D_EnumWrite (inMetafilePtr, gMF3D_SubdivisionMethodEnum,
			   writeObj->subdivisionMethod);

  if (result == kMF3DNoErr)
    {
      if (writeObj->subdivisionMethod == MF3DSubdivisionConstant)
	{
	  result = MF3D_Uns32Write (inMetafilePtr,
				 writeObj->subdivisionData.constant.value1);
	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_Uns32Write (inMetafilePtr,
				 writeObj->subdivisionData.constant.value2);
	    }
	}
      else
	{
	  MFASSERT (writeObj->subdivisionMethod ==
		    MF3DSubdivisionWorldSpace ||
		    writeObj->subdivisionMethod ==
		    MF3DSubdivisionScreenSpace);

	  result = MF3D_Float32Write (inMetafilePtr,
				    writeObj->subdivisionData.space.value1);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjSubdivisionStyleDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjSubdivisionStyleDisposer (
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjSubdivisionStyle);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMatrixTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMatrixTransformReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DMatrixObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Matrix4x4Read (inMetafilePtr, returnObj->matrix);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMatrixTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMatrixTransformWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMatrixTransform);
  MF3D_WriteNewLine (inMetafilePtr);
  return MF3D_Matrix4x4Write (inMetafilePtr,
			      ((MF3DMatrixObj *) inObj)->matrix);
}

/*==============================================================================
 *	MF3D_ObjMatrixTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMatrixTransformDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMatrixTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjQuaternionTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjQuaternionTransformReader (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj)
{
  MF3DQuaternionObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_QuaternionRead (inMetafilePtr, &returnObj->quaternion);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjQuaternionTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjQuaternionTransformWriter (
				    MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjQuaternionTransform);
  return MF3D_QuaternionWrite (inMetafilePtr,
			       ((MF3DQuaternionObjPtr) inObj)->quaternion);
}

/*==============================================================================
 *	MF3D_ObjQuaternionTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjQuaternionTransformDisposer (
				      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjQuaternionTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRotateTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateTransformReader (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj)
{
  MF3DRotateObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_EnumRead (inMetafilePtr, gMF3D_AxisEnum, &returnObj->axis);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->radians);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateTransformWriter (
				MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj)
{
  MF3DRotateObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjRotateTransform);
  writeObj = (MF3DRotateObjPtr) inObj;

  result = MF3D_EnumWrite (inMetafilePtr, gMF3D_AxisEnum, writeObj->axis);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->radians);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateTransformDisposer (
				  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRotateTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutAxisTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutAxisTransformReader (
					 MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj)
{
  MF3DRotateAboutAxisObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->origin);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->orientation);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->radians);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutAxisTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutAxisTransformWriter (
					 MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj)
{
  MF3DRotateAboutAxisObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjRotateAboutAxisTransform);
  writeObj = (MF3DRotateAboutAxisObjPtr) inObj;

  result = MF3D_Point3DWrite (inMetafilePtr, writeObj->origin);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DWrite (inMetafilePtr, writeObj->orientation);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->radians);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutAxisTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutAxisTransformDisposer (
					   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRotateAboutAxisTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutPointTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutPointTransformReader (
					  MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj)
{
  MF3DRotateAboutPointObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_EnumRead (inMetafilePtr, gMF3D_AxisEnum, &returnObj->axis);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (inMetafilePtr, &returnObj->radians);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DRead (inMetafilePtr, &returnObj->origin);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutPointTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutPointTransformWriter (
					  MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj)
{
  MF3DRotateAboutPointObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjRotateAboutPointTransform);
  writeObj = (MF3DRotateAboutPointObjPtr) inObj;

  result = MF3D_EnumWrite (inMetafilePtr, gMF3D_AxisEnum, writeObj->axis);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (inMetafilePtr, writeObj->radians);

  if (result == kMF3DNoErr)
    result = MF3D_Point3DWrite (inMetafilePtr, writeObj->origin);

  return result;
}

/*==============================================================================
 *	MF3D_ObjRotateAboutPointTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjRotateAboutPointTransformDisposer (
					    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjRotateAboutPointTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjScaleTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjScaleTransformReader (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj)
{
  MF3DScaleObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->scale);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjScaleTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjScaleTransformWriter (
			       MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjScaleTransform);
  return MF3D_Vector3DWrite (inMetafilePtr,
			     ((MF3DScaleObjPtr) inObj)->scale);
}

/*==============================================================================
 *	MF3D_ObjScaleTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjScaleTransformDisposer (
				 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjScaleTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjTranslateTransformReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjTranslateTransformReader (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj)
{
  MF3DTranslateObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Vector3DRead (inMetafilePtr, &returnObj->translate);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTranslateTransformWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjTranslateTransformWriter (
				   MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTranslateTransform);
  return MF3D_Vector3DWrite (inMetafilePtr,
			     ((MF3DTranslateObjPtr) inObj)->translate);
}

/*==============================================================================
 *	MF3D_ObjTranslateTransformDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjTranslateTransformDisposer (
				     MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjTranslateTransform);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjMacintoshPathReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjMacintoshPathReader (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj)
{
  MF3DMacintoshPathObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_CStringRead (inMetafilePtr, &returnObj->pathName);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjMacintoshPathWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjMacintoshPathWriter (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMacintoshPath);
  return MF3D_CStringWrite (inMetafilePtr,
			    ((MF3DMacintoshPathObjPtr) inObj)->pathName);
}

/*==============================================================================
 *	MF3D_ObjMacintoshPathDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjMacintoshPathDisposer (
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjMacintoshPath);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DMacintoshPathObjPtr) inObj)->pathName);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjUnixPathReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnixPathReader (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr * outObj)
{
  MF3DUnixPathObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_CStringRead (inMetafilePtr, &returnObj->pathName);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnixPathWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnixPathWriter (
			 MF3D_FilePtr inMetafilePtr,
			 MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjUnixPath);
  return MF3D_CStringWrite (inMetafilePtr,
			    ((MF3DUnixPathObjPtr) inObj)->pathName);
}

/*==============================================================================
 *	MF3D_ObjUnixPathDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnixPathDisposer (
			   MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjUnixPath);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DUnixPathObjPtr) inObj)->pathName);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjCStringReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjCStringReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DCStringObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_CStringRead (inMetafilePtr, &returnObj->cString);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjCStringWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjCStringWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCString);
  return MF3D_CStringWrite (inMetafilePtr,
			    ((MF3DCStringObjPtr) inObj)->cString);
}

/*==============================================================================
 *	MF3D_ObjCStringDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjCStringDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjCString);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DCStringObjPtr) inObj)->cString);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjUnicodeReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnicodeReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DUnicodeObjPtr returnObj;
  MF3DUns32 rawDataSize;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->length);

  if (result == kMF3DNoErr)
    {
      rawDataSize = returnObj->length;
      if (rawDataSize == 0)
	{
	  returnObj->unicode = NULL;
	}
      else
	{
	  returnObj->unicode = MF3D_Malloc (rawDataSize);
	  if (returnObj->unicode == NULL)
	    result = kMF3DErrOutOfMemory;
	}
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataRead (inMetafilePtr, rawDataSize,
				 returnObj->unicode);
      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->unicode);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnicodeWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnicodeWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3DUnicodeObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjUnicode);
  writeObj = (MF3DUnicodeObjPtr) inObj;

  result = kMF3DNoErr;

  if (writeObj->unicode != NULL)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, writeObj->length);

      if (result == kMF3DNoErr)
	{
	  result = MF3D_RawDataWrite (inMetafilePtr, writeObj->length,
				      writeObj->unicode);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnicodeDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnicodeDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjUnicode);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DUnicodeObjPtr) inObj)->unicode);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjPixmapTextureReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjPixmapTextureReader (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj)
{
  MF3DPixmapTextureObjPtr returnObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->width);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->rowBytes);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->pixelSize);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_PixelTypeEnum,
			      &returnObj->pixelType);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_EndianEnum,
			      &returnObj->bitOrder);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_EndianEnum,
			      &returnObj->byteOrder);
    }

  if (result == kMF3DNoErr)
    {
      imageSize = returnObj->rowBytes * returnObj->height;
      if (imageSize == 0)
	result = kMF3DErrWidthOrHeightIsZero;
    }

  if (result == kMF3DNoErr)
    {
      returnObj->image = MF3D_Malloc (imageSize);
      if (returnObj->image == NULL)
	result = kMF3DErrOutOfMemory;
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataRead (inMetafilePtr, imageSize, returnObj->image);
      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->image);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjPixmapTextureWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjPixmapTextureWriter (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr inObj)
{
  MF3DPixmapTextureObjPtr writeObj;
  MF3DUns32 imageSize;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjPixmapTexture);
  writeObj = (MF3DPixmapTextureObjPtr) inObj;

  result = kMF3DNoErr;

  imageSize = writeObj->rowBytes * writeObj->height;
  if (imageSize == 0)
    result = kMF3DErrWidthOrHeightIsZero;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, writeObj->width);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->height);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->rowBytes);

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Uns32Write (inMetafilePtr, writeObj->pixelSize);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_PixelTypeEnum,
			       writeObj->pixelType);
    }

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_EndianEnum,
			       writeObj->bitOrder);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_EndianEnum,
			       writeObj->byteOrder);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataWrite (inMetafilePtr, imageSize, writeObj->image);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjPixmapTextureDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjPixmapTextureDisposer (
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjPixmapTexture);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DPixmapTextureObjPtr) inObj)->image);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjViewHintsReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewHintsReader (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr * outObj)
{
  MF3DViewHintsObjPtr returnObj;
  MF3DErr result;

  MF3D_Unused (inMetafilePtr);
  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjViewHintsWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewHintsWriter (
			  MF3D_FilePtr inMetafilePtr,
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjViewHints);
  MF3D_Unused (inMetafilePtr);
  MF3D_Unused (inObj);
  /* Nothing to write! */
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjViewHintsDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjViewHintsDisposer (
			    MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjViewHints);

  MF3D_Free (inObj);
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjUnknownBinaryReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownBinaryReader (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj)
{
  MF3DUnknownBinaryObjPtr returnObj;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    result = MF3D_Int32Read (inMetafilePtr, &returnObj->realObjectType);

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Read (inMetafilePtr, &returnObj->realObjectSize);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumRead (inMetafilePtr, gMF3D_EndianEnum,
			      &returnObj->realObjectOrder);
    }

  if (result == kMF3DNoErr)
    {
      returnObj->realObjectData = MF3D_Malloc (returnObj->realObjectSize);
      if (returnObj->realObjectData == NULL)
	result = kMF3DErrOutOfMemory;
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataRead (inMetafilePtr, returnObj->realObjectSize,
				 returnObj->realObjectData);
      if (result != kMF3DNoErr)
	MF3D_Free (returnObj->realObjectData);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnknownBinaryWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownBinaryWriter (
			      MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr inObj)
{
  MF3DUnknownBinaryObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjUnknownBinary);
  writeObj = (MF3DUnknownBinaryObjPtr) inObj;

  result = kMF3DNoErr;

  if (result == kMF3DNoErr)
    {
      MF3D_WriteNewLine (inMetafilePtr);
      result = MF3D_Int32Write (inMetafilePtr, writeObj->realObjectType);
    }

  if (result == kMF3DNoErr)
    result = MF3D_Uns32Write (inMetafilePtr, writeObj->realObjectSize);

  if (result == kMF3DNoErr)
    {
      result = MF3D_EnumWrite (inMetafilePtr, gMF3D_EndianEnum,
			       writeObj->realObjectOrder);
    }

  if (result == kMF3DNoErr)
    {
      result = MF3D_RawDataWrite (inMetafilePtr, writeObj->realObjectSize,
				  writeObj->realObjectData);
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnknownBinaryDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownBinaryDisposer (
				MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjUnknownBinary);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DUnknownBinaryObjPtr) inObj)->realObjectData);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ObjUnknownReader
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownReader (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr * outObj)
{
  MF3DUnknownObjPtr returnObj;
  char *bufPtr;
  MF3DUns32 bufferEnd;
  MF3DUns32 bufPos;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (returnObj);

  if (result == kMF3DNoErr)
    {
      returnObj->dataFormat = inMetafilePtr->dataFormat;
      returnObj->realObjectName = NULL;

      bufPtr = inMetafilePtr->readBuffer.buf;
      bufPos = inMetafilePtr->readBuffer.bufPos;
      bufferEnd = inMetafilePtr->readBuffer.bufSize;

      if (MF3DIsTextFormat (inMetafilePtr->dataFormat))
	{			/* Do not include trailing ')' in text files */
	  do
	    {
	    }
	  while (bufPtr[--bufferEnd] != kMF3D_EndChar);
	}

      returnObj->size = bufferEnd - bufPos;

      inMetafilePtr->readBuffer.bufPos = bufferEnd;


      if (returnObj->size == 0)
	returnObj->data = NULL;
      else
	{
	  returnObj->data = MF3D_Malloc (returnObj->size);
	  if (returnObj->data == NULL)
	    result = kMF3DErrOutOfMemory;
	}
    }

  /* FBL_JRK */
  /* Copy the contents, whatever they may be */
  /* Cast below is safe since we must be called with a valid buffer */
  if (result == kMF3DNoErr)
    {
      memcpy (returnObj->data, &bufPtr[bufPos], (size_t) returnObj->size);
    }

  if (result == kMF3DNoErr)
    *outObj = (MF3DVoidObjPtr) returnObj;
  else
    MF3D_Free (returnObj);

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnknownWriter
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownWriter (
			MF3D_FilePtr inMetafilePtr,
			MF3DVoidObjPtr inObj)
{
  MF3DUnknownObjPtr writeObj;
  MF3DErr result;

  MF3D_VerifyObjectType (inObj, kMF3DObjUnknownType);
  writeObj = (MF3DUnknownObjPtr) inObj;

  MF3D_WriteNewLine (inMetafilePtr);
  result = MF3D_WriteProc (inMetafilePtr, writeObj->size, writeObj->data);

  if (inMetafilePtr->dataFormat != writeObj->dataFormat)
    result = kMF3DWarnWrongUnknownTypeFormat;

  return result;
}

/*==============================================================================
 *	MF3D_ObjUnknownDisposer
 *==============================================================================
 */
MF3DErr
MF3D_ObjUnknownDisposer (
			  MF3DVoidObjPtr inObj)
{
  MF3D_VerifyObjectType (inObj, kMF3DObjUnknownType);

  if (inObj != NULL)
    {
      MF3D_Free (((MF3DUnknownObjPtr) inObj)->realObjectName);
      MF3D_Free (((MF3DUnknownObjPtr) inObj)->data);
      MF3D_Free (inObj);
    }
  return kMF3DNoErr;
}
