/*==============================================================================
 *
 *	File:		MF3D.C
 *
 *	Function:	QuickDraw 3D Metafile Read/Write API routines
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
 *		F3A_RWW	All basic types working.
 *		F2S_RWW	BeginGroup changes.
 *		F2R_RWW	Change to simple object theory.
 *		F29_RWW	File created.
 *==============================================================================
 */

#include "mf3d.h"

#include <string.h>		/* memcpy */

#include "mferrors.h"
#include "mfint64.h"
#include "mfstdchk.h"
#include "mfassert.h"
#include "mffile.h"
#include "mfgroups.h"
#include "mfintobj.h"
#include "mfmacros.h"
#include "mfmemory.h"
#include "mfobjtyp.h"
#include "mfprimtv.h"
#include "mfrslntn.h"
#include "mftextrd.h"
#include "mftextwr.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3DOpenInput
 *
 *	Open a metafile for parsing using caller-defined file-handling routines.
 *
 *	Set outMF3DFilePtr to NULL on failure.
 *==============================================================================
 */
MF3DErr
MF3DOpenInput (
		MF3DUserOpenDataPtr inUserDataPtr,	/* In:  user-defined file info  */
		MF3DProcsPtr inUserIOProcsPtr,	/* In:  user-defined I/O procs  */
		MF3D_FilePtr * outMF3DFilePtr)	/* Out: MF3D file structure             */
{
  MF3D_FilePtr fileRecord;
  MF3DUserFilePtr userFilePtr;
  MF3DObjType headerSignature;
  MF3DErr result;
  MF3DBoolean fileOpened;

  result = kMF3DNoErr;

  MF3D_Allocate (fileRecord);

  if (result == kMF3DNoErr)
    {				/*
				 * Because it is unknown at this moment whether we have
				 * a text file or a binary file, open it as binary.
				 */
      result = (*inUserIOProcsPtr->openProc) (kMF3DFormatBinary,
					      inUserDataPtr, &userFilePtr);
    }

  if (result == kMF3DNoErr)
    {
      fileOpened = kMF3DBooleanTrue;
      /* Read object type to determine file type */
      result = (*inUserIOProcsPtr->readProc) (userFilePtr,
		       sizeof (headerSignature), (char *) &headerSignature);
    }

  if (result == kMF3DNoErr)
    {
      if (headerSignature == kMF3DObjMetafile)
	{
	  fileRecord->dataFormat = kMF3DFormatBinary;
	}
      else if (headerSignature == kMF3DObjMetafileSwapped)
	{
	  fileRecord->dataFormat = kMF3DFormatSwappedBinary;
	}
      else
	{			/* Assume text */
	  fileRecord->dataFormat = kMF3DFormatText;
	}
    }

  /* Close and reopen as correct type
   * This is unnecessary for binary files, but has the side effect
   * of resetting the file offset to 0.
   */
  if (result == kMF3DNoErr)
    {
      (*inUserIOProcsPtr->closeProc) (userFilePtr);
      fileOpened = kMF3DBooleanFalse;
      result = (*inUserIOProcsPtr->openProc) (fileRecord->dataFormat,
					      inUserDataPtr, &userFilePtr);
    }

  if (result == kMF3DNoErr)
    {
      fileOpened = kMF3DBooleanTrue;
      result = MF3D_GetPrimitivesAccessor (fileRecord->dataFormat,
					   &fileRecord->primitives);
    }

  if (result == kMF3DNoErr)
    {
      fileRecord->readWrite = MF3D_MetafileRead;
      fileRecord->readBuffer.buf = NULL;
      fileRecord->inContainer = 0;
      fileRecord->userFilePtr = userFilePtr;
      memcpy (&fileRecord->procsRec, inUserIOProcsPtr, sizeof (MF3DProcsRec));
      fileRecord->resStuff.resState = MF3D_NotResolvingReference;
      fileRecord->resStuff.reference = NULL;
      fileRecord->resStuff.parent = NULL;
      SetInt64ToZero (fileRecord->resStuff.returnLoc);
      fileRecord->typeTable.nTypes = 0;
      MF3D_AllocEmpty (fileRecord->typeTable.types);

      result = MF3D_InitGroup (fileRecord);
    }

  /* Prepare the file for reading */
  if (result == kMF3DNoErr)
    result = MF3D_PreprocessFile (fileRecord);

  if (result == kMF3DNoErr)
    {
      *outMF3DFilePtr = fileRecord;
    }
  else
    {
      if (fileOpened == kMF3DBooleanTrue)
	(*inUserIOProcsPtr->closeProc) (userFilePtr);
      if (fileRecord != NULL)
	MF3D_Free (fileRecord);
      *outMF3DFilePtr = NULL;
    }

  return result;
}

/*==============================================================================
 *	MF3DOpenInputStdCFile
 *
 *	Open a metafile for parsing using standard C routines.
 *
 *	This routine just calls MF3DOpenInput with inUserDataPtr = inFilename
 *	and inUserIOProcsPtr set to call fopen, fread, NULL, ftell, fseek,
 *	and fclose.
 *==============================================================================
 */
MF3DErr
MF3DOpenInputStdCFile (
			const char *inFilename,		/* In:  C-string filename                       */
			MF3D_FilePtr * outMF3DFilePtr)	/* Out: MF3D file structure                     */
{
  MF3DProcsRec procsRecord;
  MF3DStdCOpenData stdCOpenData;

  procsRecord.openProc = MF3DStdCOpenHook;
  procsRecord.readProc = MF3DStdCReadHook;
  procsRecord.writeProc = NULL;
  procsRecord.tellProc = MF3DStdCTellHook;
  procsRecord.seekProc = MF3DStdCSeekHook;
  procsRecord.closeProc = MF3DStdCCloseHook;

  stdCOpenData.filename = (char *) inFilename;
  stdCOpenData.permission = kMF3DStdCReadPerm;

  return MF3DOpenInput ((MF3DUserOpenDataPtr) & stdCOpenData, &procsRecord,
			outMF3DFilePtr);
}

/*==============================================================================
 *	MF3DReadAnObject
 *
 *	Retrieve the next metafile object in an open metafile.
 *
 *	Metafile objects are defined in <MFOBJCTS.H>. They can be recognized
 *		by checking the fObjectType field and casting the object pointer
 *		appropriately.
 *
 *	Objects returned by MF3DReadAnObject should be disposed using
 *		MF3DDisposeObject when they are no longer needed. If an error occurs,
 *		NULL will be returned (MF3DDisposeObject ignores NULL parameters).
 *
 *	Returns:
 *		kMF3DNoErr				object was successfully read
 *		kMF3DNoMoreObjects		the end of the file has been reached
 *==============================================================================
 */
MF3DErr
MF3DReadAnObject (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure                     */
		   MF3DVoidObjPtr * outMF3DObjPtr)	/* Out: metafile object                         */
{
  MF3DEndContainerObjPtr endContainerObj;
  MF3DErr result;

  if (inMF3DFilePtr == NULL || outMF3DObjPtr == NULL)
    return kMF3DErrInvalidParameter;

  if (inMF3DFilePtr->resStuff.reference != NULL)
    {				/* If we are resolving a reference, jump to the referenced file */
      return MF3DReadAnObject (inMF3DFilePtr->resStuff.reference,
			       outMF3DObjPtr);
    }

  /*
   * Special upfront check for EndContainer
   */
  if (inMF3DFilePtr->readBuffer.buf != NULL)
    {
      result = MF3D_EndRead (inMF3DFilePtr);
      if (result == kMF3DNoErr)
	{			/* We got an end paren; it should be an EndContainer marker */
	  if (inMF3DFilePtr->inContainer == 0)
	    result = kMF3DErrTooManyEndContainers;
	  else
	    {
	      --inMF3DFilePtr->inContainer;
	      MF3D_Allocate (endContainerObj);
	    }
	  if (result == kMF3DNoErr)
	    {
	      endContainerObj->objectType = kMF3DObjEndContainer;
	      endContainerObj->refInfo = NULL;
	      *outMF3DObjPtr = (MF3DVoidObjPtr) endContainerObj;

	      /* If we were resolving a container reference, we may
	       * be able to stop now.
	       */
	      if (inMF3DFilePtr->resStuff.resState >= MF3D_ResolvingReference)
		{
		  --inMF3DFilePtr->resStuff.resState;
		  if (inMF3DFilePtr->resStuff.resState ==
		      MF3D_ResolvingReference)
		    {
		      MFASSERT (inMF3DFilePtr->resStuff.resState >=
				MF3D_ResolvingReference);
		      result = MF3D_PopResolution (inMF3DFilePtr);
		    }
		}
	    }
	  return result;	/* ### INTERNAL EXIT ### */
	}
    }

  /* This case should not happen */
  MFASSERT (!(MF3DIsTextFormat (inMF3DFilePtr->dataFormat) &&
	      inMF3DFilePtr->readBuffer.buf != NULL));
  if (MF3DIsTextFormat (inMF3DFilePtr->dataFormat) &&
      inMF3DFilePtr->readBuffer.buf != NULL)
    {
      return kMF3DErrCantParse;	/* ### INTERNAL EXIT ### */
    }

  /* Now, we want to skip TOC objects.
   * NOTE: The easiest way to do this is to continue reading objects
   * until we get a non-TOC object and a non-Type object.
   * This is not necessarily the _best_ way.
   */
  do
    {
      result = MF3D_IntReadObject (inMF3DFilePtr, outMF3DObjPtr);
      if (*outMF3DObjPtr != NULL)
	{
	  if ((*outMF3DObjPtr)->objectType == kMF3DObjTableOfContents)
	    MF3DDisposeObject (*outMF3DObjPtr);
	  else
	    break;		/* ### NORMAL LOOP EXIT ### */
	}

      /* JRK */
      /* MF3D_IntReadObject does not dispose of readBuffer on error any more.
       * We need to check for it here.
       */
      if (result != kMF3DNoErr)
	{
	  MF3D_Free (inMF3DFilePtr->readBuffer.buf);
	  inMF3DFilePtr->readBuffer.buf = NULL;
	}
    }
  while (result == kMF3DNoErr);

  return result;
}

/*==============================================================================
 *	MF3DDisposeObject
 *
 *	Dispose a metafile object.
 *	If inMF3DObjPtr is NULL, nothing happens.
 *==============================================================================
 */
MF3DErr
MF3DDisposeObject (
		    MF3DVoidObjPtr inMF3DObjPtr)	/* In:  metafile object                         */
{
  MF3D_ObjStuffPtr objStuff;
  MF3DErr result;

  result = kMF3DNoErr;

  if (inMF3DObjPtr != NULL)
    {
      result = MF3D_FindObjectFromType (inMF3DObjPtr->objectType,
					&objStuff);

      if (inMF3DObjPtr->refInfo != NULL)
	{
	  MF3D_Free (inMF3DObjPtr->refInfo->refName);
	  MF3D_Free (inMF3DObjPtr->refInfo);
	}

      if (result == kMF3DNoErr)
	result = (*objStuff->disposer) (inMF3DObjPtr);
    }

  return result;
}

/*==============================================================================
 *	MF3DResolveReference
 *
 *	Set up an MF3D file so that it will read the object pointed to by
 *	a reference object. The next call to MF3DReadAnObject will then read
 *	the referenced object, and the MF3D file will then be reset to read the
 *	object that would have been read if MF3DResolveReference had not been
 *	called.
 *
 *	If MF3DResolveReference resolves to a group object, the entire group will
 *	be read before the MF3D file is reset.
 *
 *	If inStoragePtr is not NULL, the reference is in an external file.
 *==============================================================================
 */
MF3DErr
MF3DResolveReference (
		       MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure             */
		       MF3DReferenceObjPtr inMF3DRefObjPtr,	/* In:  reference object                */
		       MF3DStorageObjPtr inExternalFilePtr)	/* In:  external storage object */
{
  MF3DErr result;

  if (inMF3DFilePtr == NULL || inMF3DRefObjPtr == NULL)
    return kMF3DErrInvalidParameter;

  result = kMF3DNoErr;

  if (inMF3DRefObjPtr->objectType != kMF3DObjReference)
    result = kMF3DErrNotAReferenceObj;

  if (result == kMF3DNoErr)
    {
      result = MF3D_PushResolution (inMF3DFilePtr, inMF3DRefObjPtr,
				    inExternalFilePtr);
    }

  return result;
}

/*==============================================================================
 *	MF3DClose
 *
 *	Close the metafile.
 *
 *	If inMF3DFilePtr is NULL, nothing happens.
 *==============================================================================
 */
MF3DErr
MF3DClose (
	    MF3D_FilePtr inMF3DFilePtr)		/* In:  MF3D file structure                     */
{
  MF3D_TypeListPtr tempPtr;
  MF3DUns32 numUserDefinedTypes;
  MF3DErr result, closeResult;

  if (inMF3DFilePtr == NULL)
    return kMF3DNoErr;

  result = kMF3DNoErr;

  /* Free the user types table */
  for (numUserDefinedTypes = inMF3DFilePtr->typeTable.nTypes,
       tempPtr = inMF3DFilePtr->typeTable.types;
       numUserDefinedTypes > 0;
       --numUserDefinedTypes, ++tempPtr)
    {
      MF3D_Free (tempPtr->name);
    }
  MF3D_Free (inMF3DFilePtr->typeTable.types);

  if (inMF3DFilePtr->readWrite == MF3D_MetafileRead)
    {
      MF3D_DisposeGroup (inMF3DFilePtr);
      MF3D_PostprocessFile (inMF3DFilePtr);
      /* Close any files we opened to resolve a reference */
      if (inMF3DFilePtr->resStuff.reference != NULL)
	result = MF3D_PopResolution (inMF3DFilePtr->resStuff.reference);

      closeResult = MF3D_CloseReadBuffer (inMF3DFilePtr);

      if (result == kMF3DNoErr)
	result = closeResult;
    }
  else if (inMF3DFilePtr->readWrite == MF3D_MetafileWrite)
    {
      MF3DVoidObj fakeTOCObj;

      if (inMF3DFilePtr->tocStuff.refSeed > 1 ||
	  inMF3DFilePtr->tocStuff.typeSeed < -1)
	{
	  result = MF3D_BackpatchTOCLocation (inMF3DFilePtr);

	  if (result == kMF3DNoErr)
	    {			/* Fake a TOC object to force it to get written */
	      fakeTOCObj.objectType = kMF3DObjTableOfContents;
	      fakeTOCObj.refInfo = NULL;
	      result = MF3DWriteAnObject (inMF3DFilePtr, &fakeTOCObj);
	    }
	}
      MF3D_DisposeTOCStuff (inMF3DFilePtr);
    }

  closeResult =
    (*inMF3DFilePtr->procsRec.closeProc) (inMF3DFilePtr->userFilePtr);

  MF3D_Free (inMF3DFilePtr);

  if (result == kMF3DNoErr)
    result = closeResult;

  return result;
}

/*==============================================================================
 *	MF3DOpenOutput
 *
 *	Open a metafile for writing using caller-defined file-handling routines.
 *
 *	Set *outMF3DFilePtr to NULL on error.
 *==============================================================================
 */
MF3DErr
MF3DOpenOutput (
		 MF3DDataFormatEnum inMF3DDataFormat,	/* In:  binary or text                  */
		 MF3DUserOpenDataPtr inUserDataPtr,	/* In:  user-defined file info  */
		 MF3DProcsPtr inUserIOProcsPtr,		/* In:  user-defined I/O procs  */
		 MF3D_FilePtr * outMF3DFilePtr)		/* Out: MF3D file structure             */
{
  MF3D_FilePtr fileRecord;
  MF3DUserFilePtr userFilePtr;
  MF3DBoolean fileOpened;
  MF3DErr result;

  result = kMF3DNoErr;

  MF3D_Allocate (fileRecord);

  fileOpened = kMF3DBooleanFalse;
  if (result == kMF3DNoErr)
    {
      memcpy (&fileRecord->procsRec, inUserIOProcsPtr, sizeof (MF3DProcsRec));

      result = (*fileRecord->procsRec.openProc) (inMF3DDataFormat,
					       inUserDataPtr, &userFilePtr);
    }

  if (result == kMF3DNoErr)
    {
      fileOpened = kMF3DBooleanTrue;
      result = MF3D_GetPrimitivesAccessor (inMF3DDataFormat,
					   &fileRecord->primitives);
    }

  if (result == kMF3DNoErr)
    {
      fileRecord->dataFormat = inMF3DDataFormat;
      fileRecord->readWrite = MF3D_MetafileWrite;
      fileRecord->userFilePtr = userFilePtr;
      fileRecord->inContainer = 0;
      SetInt64ToZero (fileRecord->tocLocation);
      fileRecord->writeStack = NULL;
      fileRecord->indent = 0;
      fileRecord->tocStuff.tocLabelName = NULL;
      fileRecord->tocStuff.refSeed = 1;
      fileRecord->tocStuff.typeSeed = -1;
      fileRecord->tocStuff.numReferences = 0;
      MF3D_AllocEmpty (fileRecord->tocStuff.references);
      fileRecord->tocStuff.needToUpdate = kMF3DBooleanFalse;
      fileRecord->typeTable.nTypes = 0;
      MF3D_AllocEmpty (fileRecord->typeTable.types);
    }

  if (result == kMF3DNoErr)
    {
      *outMF3DFilePtr = fileRecord;
    }
  else
    {
      if (fileOpened)
	(inUserIOProcsPtr->closeProc) (userFilePtr);
      if (fileRecord != NULL)
	MF3D_Free (fileRecord);
      *outMF3DFilePtr = NULL;
    }

  return result;
}

/*==============================================================================
 *	MF3DOpenOutputStdCFile
 *
 *	Open a metafile for writing using standard C routines.
 *
 *	This routine just calls MF3DOpenOutput with inUserDataPtr = inFilename
 *	and inUserIOProcsPtr set to call fopen, fread, fwrite, ftell, fseek,
 *	and fclose.
 *==============================================================================
 */
MF3DErr 
MF3DOpenOutputStdCFile (
			 MF3DDataFormatEnum inMF3DDataFormat,	/* In:  binary or text                  */
			 const char *inFilename,	/* In:  C-string filename               */
			 MF3D_FilePtr * outMF3DFilePtr)		/* Out: MF3D file structure             */
{
  MF3DProcsRec procsRecord;
  MF3DStdCOpenData stdCOpenData;

  procsRecord.openProc = MF3DStdCOpenHook;
  procsRecord.readProc = MF3DStdCReadHook;
  procsRecord.writeProc = MF3DStdCWriteHook;
  procsRecord.tellProc = MF3DStdCTellHook;
  procsRecord.seekProc = MF3DStdCSeekHook;
  procsRecord.closeProc = MF3DStdCCloseHook;

  stdCOpenData.filename = (char *) inFilename;
  stdCOpenData.permission = kMF3DStdCWritePerm;

  return MF3DOpenOutput (inMF3DDataFormat, (MF3DUserOpenDataPtr) & stdCOpenData,
			 &procsRecord, outMF3DFilePtr);
}

/*==============================================================================
 *	MF3DWriteAnObject
 *
 *	Write a metafile object to an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DWriteAnObject (
		    MF3D_FilePtr inMF3DFilePtr,		/* In:  MF3D file structure             */
		    MF3DVoidObjPtr inMF3DObjPtr)	/* In:  metafile object                 */
{
  MF3D_ObjStuffPtr objStuff;
  MF3DBinaryFilePosition location;
  MF3DErr beginResult, writerResult, endResult;

  if (inMF3DFilePtr == NULL || inMF3DObjPtr == NULL)
    return kMF3DErrInvalidParameter;

  beginResult = kMF3DNoErr;

  /* If the last object was a labeled container, set the object type
   * of that label to this (the root) object.
   */
  if (inMF3DFilePtr->tocStuff.needToUpdate == kMF3DBooleanTrue)
    {
      inMF3DFilePtr->tocStuff.references[
			   inMF3DFilePtr->tocStuff.numReferences - 1].type =
	inMF3DObjPtr->objectType;
      inMF3DFilePtr->tocStuff.needToUpdate = kMF3DBooleanFalse;
    }

  if (inMF3DObjPtr->refInfo != NULL)
    {
      beginResult = MF3DTellPosition (inMF3DFilePtr, &location);
      if (beginResult == kMF3DNoErr)
	{
	  beginResult = MF3D_LabelWrite (inMF3DFilePtr,
		 inMF3DObjPtr->refInfo, location, inMF3DObjPtr->objectType);
	}
    }

  if (beginResult == kMF3DNoErr &&
      inMF3DObjPtr->objectType == kMF3DObjUnknownType)
    {				/* User is trying to write a new type */
      beginResult = MF3D_TypeObjWrite (inMF3DFilePtr, inMF3DObjPtr);
    }

  if (beginResult == kMF3DNoErr)
    beginResult = MF3D_BeginWrite (inMF3DFilePtr, inMF3DObjPtr, &objStuff);

  if (beginResult == kMF3DNoErr)
    writerResult = (*objStuff->writer) (inMF3DFilePtr, inMF3DObjPtr);

  /* Write the closing paren, even if writer failed */
  if (beginResult == kMF3DNoErr)
    endResult = MF3D_EndWrite (inMF3DFilePtr, inMF3DObjPtr);

  /* Ignore error when writing newline here */
  if (beginResult == kMF3DNoErr)
    MF3D_WriteNewLine (inMF3DFilePtr);

  return beginResult == kMF3DNoErr ?
    (writerResult == kMF3DNoErr ? endResult : writerResult) :
    beginResult;
}

/*==============================================================================
 *	MF3DTellPosition
 *
 *	Retrieve the position of an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DTellPosition (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure     */
		   MF3DBinaryFilePosition * outMF3DPosPtr)	/* Out: file offset                     */
{
  MF3DErr result;
  MF3DBinaryFilePosition position;

  if (inMF3DFilePtr == NULL)
    return kMF3DErrInvalidParameter;

  result = (*inMF3DFilePtr->procsRec.tellProc) (
				     inMF3DFilePtr->userFilePtr, &position);

  if (result == kMF3DNoErr)
    *outMF3DPosPtr = position;

  return result;
}

/*==============================================================================
 *	MF3DSeekPosition
 *
 *	Set the position of an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DSeekPosition (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure             */
		   MF3DBinaryFilePosition inMF3DPosition)	/* In:  file offset                             */
{
  if (inMF3DFilePtr == NULL)
    return kMF3DErrInvalidParameter;

  return (*inMF3DFilePtr->procsRec.seekProc) (
				inMF3DFilePtr->userFilePtr, inMF3DPosition);
}
