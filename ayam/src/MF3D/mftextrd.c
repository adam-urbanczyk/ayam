/*==============================================================================
 *
 *	File:		MFTEXTRD.C
 *
 *	Function:	Internal Read routines
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
 *		F3D_RWW	Changed ReadTextString not to call ScanTextBuffer, which
 *					stripped spaces.
 *		F3A_RWW	TOC stuff works.
 *		F37_RWW	Copy label table so that we can return names with objects.
 *		F2R_RWW	Get rid of dumb curPos field.
 *		F2F_RWW	File created.
 *==============================================================================
 */
#include "mftextrd.h"

#include <ctype.h>			/* isspace			*/
#include <stddef.h>			/* NULL				*/
#include <stdio.h>			/* sscanf			*/

#include "mf3d.h"
#include "mferrors.h"
#include "mfint64.h"
#include "mfsystyp.h"
#include "mfassert.h"
#include "mfmacros.h"
#include "mfintobj.h"
#include "mfmemory.h"
#include "mfobjtyp.h"
#include "mftextst.h"
#include "mftextut.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_ReadSingleChar
 *
 *	Read a single character from a text metafile.
 *	If we have time, make this a buffered read for better performance.
 *==============================================================================
 */
char
MF3D_ReadSingleChar(
	MF3D_FilePtr	inMetafilePtr)
{
	MF3DErr	error;
	char	c;

	error = (*inMetafilePtr->procsRec.readProc)
					(inMetafilePtr->userFilePtr, sizeof(char), &c);
	if (error != kMF3DNoErr)
		return kMF3DEOFChar;				/* assume EOF on error */
	else
		return c;
}

/*==============================================================================
 *	MF3D_ReadUntilCloseParen
 *
 *	Read characters from a file until we get a close parenthesis
 *	that matches the open parenthesis we just read.
 *==============================================================================
 */
MF3DErr
MF3D_ReadUntilCloseParen(
	MF3D_FilePtr	inMetafilePtr)
{
	MF3DUns32	parenCount;
	char		c;

	parenCount = 1;
	do
	{	c = MF3D_ReadSingleChar(inMetafilePtr);
		if (c == kMF3D_CommentLineChar)
		{	/* Entering a comment: read to end of line */
			do
			{	c = MF3D_ReadSingleChar(inMetafilePtr);
			} while (strchr(kMF3D_EndOfLineList, c) == NULL && c != kMF3DEOFChar);
		}
		else if (c == kMF3D_StringBeginChar)
		{	/* Entering a string: read to end of string */
			do
			{	c = MF3D_ReadSingleChar(inMetafilePtr);
				if (c == kMF3D_StringEscapeChar)
				{	/* If we get the escape character,
					 * then ignore the following character.
					 */
					MF3D_ReadSingleChar(inMetafilePtr);
					c = MF3D_ReadSingleChar(inMetafilePtr);
				}
			} while (c != kMF3D_StringEndChar && c != kMF3DEOFChar);
		}
		else if (c == kMF3D_BeginChar)
		{	++parenCount;
		}
		else if (c == kMF3D_EndChar)
		{	--parenCount;
		}
	} while (parenCount > 0 && c != kMF3DEOFChar);

	return (parenCount == 0 ? kMF3DNoErr : kMF3DErrCantParse);
}

/*==============================================================================
 *	MF3D_GetTOCLabels
 *
 *	Broke this out of MF3D_PreprocessTextFile
 *==============================================================================
 */
MF3DErr
MF3D_GetTOCLabels(
	MF3D_FilePtr					inMetafilePtr,
	MF3DUns32						inNumTocs,
	MF3DConstBinaryFilePositionPtr	inTocLocations,
	MF3DUns32						*outNumLabels,
	MF3D_TOCReferencePtr			*outTocLabelNames,
	MF3DUns32						*outRefSeed,
	MF3DInt32						*outTypeSeed)
{
	MF3DUns32						numTocs;
	MF3DConstBinaryFilePositionPtr	tocLocations;
	MF3DUns32						numLabels;
	MF3D_TOCReferencePtr			tocLabelNames;
	MF3DUns32						refSeed;
	MF3DInt32						typeSeed;
	MF3DErr							result;

	tocLocations = inTocLocations;

	result = kMF3DNoErr;
	numLabels = 0;
	MF3D_AllocEmpty(tocLabelNames);
	refSeed = 0;
	typeSeed = 0;

	for (numTocs = inNumTocs; numTocs > 0 && result == kMF3DNoErr; --numTocs)
	{	MF3DTableOfContentsObjPtr	tocPtr;
		MF3D_TOCEntryPtr			tocEntryPtr;
		MF3DUns32					numNewLabels;

		tocPtr = NULL;

		/* Set the file pointer to the next TOC and get the obj there */
		result = MF3DSeekPosition(inMetafilePtr, *tocLocations);

		if (result == kMF3DNoErr)
		{	++tocLocations;
			result = MF3D_IntReadObject(inMetafilePtr,
					(MF3DVoidObjPtr *)&tocPtr);
		}

		if (result == kMF3DNoErr)
		{	if (tocPtr->objectType != kMF3DObjTableOfContents)
				result = kMF3DErrCantParse;
		}

		if (result == kMF3DNoErr)
		{	if (tocPtr->refSeed > refSeed)
				refSeed = tocPtr->refSeed;
			if (tocPtr->typeSeed < typeSeed)
				typeSeed = tocPtr->typeSeed;
			numNewLabels = tocPtr->nEntries;
			MFASSERT(numNewLabels >= 0);

			if (numNewLabels > 0)
				result = MF3D_Reallocate(&tocLabelNames,
						(numLabels + numNewLabels) * sizeof(*tocLabelNames));
		}

		if (result == kMF3DNoErr)
		{	/* Make copies of all the labels */
			tocEntryPtr = tocPtr->tocEntries;

			for (; numNewLabels > 0; --numNewLabels)
			{	MF3DCStringPtr	ptr;
	
				MFASSERT(tocEntryPtr != NULL);
				MFASSERT(tocEntryPtr->objLocation != NULL);
				MFASSERT(tocEntryPtr->objLocation->format == kMF3DFormatText);
				ptr = MF3D_DuplicateCString(
						tocEntryPtr->objLocation->location.text);
				if (ptr == NULL)
				{	result = kMF3DErrOutOfMemory;
					break;
				}
				tocLabelNames[numLabels].refID = tocEntryPtr->refID;
				tocLabelNames[numLabels].ref.name = ptr;
				++tocEntryPtr;
				++numLabels;
			}
		}

		/* Done with this TOC object; so dispose it */
		MF3DDisposeObject((MF3DVoidObjPtr)tocPtr);
	}

	*outNumLabels = numLabels;
	*outTocLabelNames = tocLabelNames;
	*outRefSeed = refSeed;
	*outTypeSeed = typeSeed;

	return result;
}

/*==============================================================================
 *	MF3D_ConvertTableLabels
 *
 *	Convert all the labels in ioObjTable to refIDs.
 *	Also frees any matching label names in ioTocLabelNames.
 *==============================================================================
 */
MF3DErr
MF3D_ConvertTableLabels(
	MF3D_FilePtr				inMetafilePtr,
	MF3DUns32					inNumLabels,
	MF3D_TOCReferencePtr		ioTocLabelNames,
	MF3DUns32					inNumObjects,
	MF3D_ObjectTableEntryPtr	ioObjTable)
{
	MF3DUns32					objCount;
	MF3DUns32					labelIndex;
	MF3D_ObjectTableEntryPtr	objTablePtr;
	MF3DUns32					labelLen;
	MF3DCStringPtr				objLabelName;
	MF3DErr						result;

	result = kMF3DNoErr;

	MF3D_AllocEmpty(objLabelName);
	objTablePtr = ioObjTable;
	for (objCount = inNumObjects; objCount > 0; --objCount)
	{	/* Did we mark this object as having a label? */
		if ((labelLen = objTablePtr->objRefID) > 0)
		{	result = MF3DSeekPosition(inMetafilePtr, objTablePtr->objLocation);
			if (result != kMF3DNoErr)
				break;

			result = MF3D_Reallocate(&objLabelName, labelLen);
			if (result != kMF3DNoErr)
				break;

			/* Read the object label name from the metafile */
			--labelLen;			/* get rid of the colon */
			result = (*inMetafilePtr->procsRec.readProc)
					(inMetafilePtr->userFilePtr, labelLen, objLabelName);
			if (result != kMF3DNoErr)
				break;

			/* Replace the colon with EOS */
			objLabelName[labelLen] = '\0';

			/* Compare the label name against all the names in the TOC */
			for (labelIndex = 0; labelIndex < inNumLabels; ++labelIndex)
			{	if (MF3D_CompareLabelNames(objLabelName,
						ioTocLabelNames[labelIndex].ref.name) == 0)
				{	objTablePtr->objRefID =
							ioTocLabelNames[labelIndex].refID;
					/* We no longer free the name because a copy of the ptr
					 * exists in the metafile (tocStuff.references).
					 */
					/* MF3D_Free(ioTocLabelNames[labelIndex].name); */
					ioTocLabelNames[labelIndex].ref.name = NULL;
					break;
				}
			}
			if (labelIndex >= inNumLabels)
			{	/* Did not find the label. Just ignore it. */
				objTablePtr->objRefID = kMF3DUnreferencedLabel;
			}
		}
		++objTablePtr;
	}

	MF3D_Free(objLabelName);

	return result;
}

/*==============================================================================
 *	MF3D_PreprocessTextFile
 *
 *	We determined that we need to read the whole text file to get the TOC.
 *	As long as we are reading the text file, we might as well store object
 *	locations because they will come in handy when we actually read the objects.
 *
 *	Recipe for spaghetti code:
 *		Write source code on a deadline and then realize halfway through that
 *		some basic assumptions are not true. [In other words, my apologies to
 *		the reader; but nobody is supposed to be reading this code anyway.]
 *
 *	Returns:
 *		kMF3DNoErr					if preprocess succeeded
 *		kMF3DErrOutOfMemory			if unable to create objTable
 *		kMF3DErrCantParse			if file format is incorrect
 *		kMF3DErrObjHasTooManyLabels	if two or more labels for a single object
 *		kMF3DErrIllegalObjName		if object name was way too long or was not
 *										followed by an open parenthesis
 *		any error returned by		MF3DTellPosition()
 *==============================================================================
 */
MF3DErr
MF3D_PreprocessTextFile(
	MF3D_FilePtr	inMetafilePtr)
{
	MF3DUns32					numObjects;
	MF3D_ObjectTableEntryPtr	objTable;
	MF3DBinaryFilePosition		location;
	MF3DUns32					numTocs;
	MF3DBinaryFilePositionPtr	tocLocations;
	MF3DUns32					numLabels, labelIndex;
	MF3D_TOCReferencePtr		tocLabelNames;
	MF3DUns32					refSeed;
	MF3DInt32					typeSeed;
	MF3DUns32					containerDepth;
	MF3DErr						result;
	/* XXXX */
	signed char						c;
	/* XXXX */
	MF3DBinaryFilePosition	firstObj;


	MFASSERT(inMetafilePtr != NULL);

	/* We have not yet found any objects */
	numObjects = 0;
	MF3D_AllocEmpty(objTable);
	containerDepth = 0;	/* Contents objects whose end parens we have to match */

	/* We have not yet read any of the Table of Contents */
	numTocs = 0;
	MF3D_AllocEmpty(tocLocations);

	/* We have no label table yet */
	numLabels = 0;
	tocLabelNames = NULL;

	/* We start outside any object (root level) */

	/* Somewhat lame, but we have to keep calling tell until we hit
	 * something (we want to know where we were when we hit something;
	 * not where we are after we hit it).
	 */
	while ((result = MF3DTellPosition(inMetafilePtr, &location)) == kMF3DNoErr)
	{	/* Read until there is nothing more to read */
		if ((c = MF3D_ReadSingleChar(inMetafilePtr)) == kMF3DEOFChar)
			break;					/* ### REAL LOOP EXIT ### */

		/* Check for end of contents object */
		if (c == kMF3D_EndChar)
		{	if (containerDepth == 0)
			{	result = kMF3DErrCantParse;
				goto PreprocessTextFileAbort;
			}
			--containerDepth;
		}
		/* F7K_PAM added this 'else' to handle reading of comments */
		else if (c == kMF3D_CommentLineChar)
		{       /* Entering a comment: read to end of line */
			do
			{	c = MF3D_ReadSingleChar(inMetafilePtr);
			} while (strchr(kMF3D_EndOfLineList, c) == NULL && c != kMF3DEOFChar);
		}
		/* Look for a text label or object name */
		else if (!isspace(c))
		{	unsigned int	objNameCharPos;
			char			objName[kMF3D_MaxObjNameLength + 1];

			if (numObjects % kMF3D_ObjTableChunk == 0)
			{	result = MF3D_Reallocate(&objTable,
						(numObjects + kMF3D_ObjTableChunk) * sizeof(*objTable));
				if (result != kMF3DNoErr)
					goto PreprocessTextFileAbort;
			}

			/* Store location of beginning of object */
			AssignInt64(objTable[numObjects].objLocation, location);
			objTable[numObjects].objRefID = 0;

			objNameCharPos = 0;

			do
			{	if (objNameCharPos < kMF3D_MaxObjNameLength)
					objName[objNameCharPos++] = c;

				c = MF3D_ReadSingleChar(inMetafilePtr);

				if (isspace(c) || c == kMF3D_BeginChar)
				{	/* Object header name has ended */
					if (objNameCharPos >= kMF3D_MaxObjNameLength)
					{	result = kMF3DErrIllegalObjName;
						goto PreprocessTextFileAbort;
					}
					objName[objNameCharPos] = '\0';
					/* Make sure we actually get the open paren */
					while (c != kMF3D_BeginChar)
					{	if (!isspace(c) || c == kMF3DEOFChar)
						{	result = kMF3DErrIllegalObjName;
							goto PreprocessTextFileAbort;
						}
						c = MF3D_ReadSingleChar(inMetafilePtr);
					}
					/* Is this a table of contents object? */
					if (MF3D_CompareObjNames(objName, kMF3DObjTableOfContentsText) == 0)
					{	/* We found a TOC object. We will want to find
						 * this later; so save the index.
						 */
						result = MF3D_Reallocate(&tocLocations,
								(numTocs + 1) * sizeof(*tocLocations));
						if (result != kMF3DNoErr)
							goto PreprocessTextFileAbort;
						AssignInt64(tocLocations[numTocs], location);
						++numTocs;

						/* Skip to the matching close paren */
						result = MF3D_ReadUntilCloseParen(inMetafilePtr);
						break;				/* ### LOOP EXIT ### */
					}
					else if (MF3D_CompareObjNames(objName,
							kMF3DObjContainerText) == 0)
					{	/* We found a Container object.
						 * Ignore it. AND read no further.
						 */
						++containerDepth;
						break;				/* ### LOOP EXIT ### */
					}
					else
					{	/* We have a non-table of contents object.
						 * Just read until we get the matching end paren.
						 */
						result = MF3D_ReadUntilCloseParen(inMetafilePtr);
						break;				/* ### LOOP EXIT ### */
					}
					if (result != kMF3DNoErr)
						goto PreprocessTextFileAbort;
				}
				else if (c == kMF3D_LabelChar)
				{	/* First time through, we store labels as lengths */
					MF3DBinaryFilePosition	endLabelLoc;

					result = MF3DTellPosition(inMetafilePtr, &endLabelLoc);

					if (objTable[numObjects].objRefID != 0)
						result = kMF3DErrObjHasTooManyLabels;

					if (result != kMF3DNoErr)
						goto PreprocessTextFileAbort;

					/* length of label, including colon */
					objTable[numObjects].objRefID = SubtractUns64(endLabelLoc,
							location);

					/* reset name pointer and continue looking */
					objNameCharPos = 0;
					do
					{	c = MF3D_ReadSingleChar(inMetafilePtr);

						/* FB4_JRK */
						if (c == kMF3D_CommentLineChar)
						{       /* Entering a comment: read to end of line */
							do
							{	c = MF3D_ReadSingleChar(inMetafilePtr);
							} while (strchr(kMF3D_EndOfLineList, c) == NULL && c != kMF3DEOFChar);
						}
					} while (isspace(c));
				}
				else if (c == kMF3DEOFChar)
				{	result = kMF3DErrCantParse;
					goto PreprocessTextFileAbort;
				}
			} while (1);

			++numObjects;
		}

	}



	if (numObjects == 0)
		result = kMF3DErrNoObjectsFound;

	if (result == kMF3DNoErr && containerDepth > 0)
		result = kMF3DErrCantParse;

	/* Readjust the object table from its chunky size */
	if (result == kMF3DNoErr)
	{	result = MF3D_Reallocate(&objTable, (numObjects + 1) * sizeof(*objTable));
		if (result == kMF3DNoErr)
		{	/* Store the file length here */
			AssignInt64(objTable[numObjects].objLocation, location);
			objTable[numObjects].objRefID = 0;

			inMetafilePtr->objTable.numObjects = numObjects;
			inMetafilePtr->objTable.objects = objTable;
		}
	}

	/* Okay, if we get this far, we have read through the entire file
	 * successfully!
	 *
	 * Now, we can use all our fancy read routines because we have
	 * all the object sizes.
	 *
	 * So, read in all the TableOfContents objects and convert all
	 * the object labels to refIDs.
	 */
	if (result == kMF3DNoErr)
	{	result = MF3D_GetTOCLabels(inMetafilePtr, numTocs, tocLocations,
				&numLabels, &tocLabelNames, &refSeed, &typeSeed);
	}

	/* Save the label table info because we will need it for the
	 * names later.
	 * NOTE: We can probably rewrite to eliminate MF3D_ConvertTableLabels now,
	 * if we want to.
	 */
	if (result == kMF3DNoErr)
	{	MF3DSize				tocLabelTableSize;
		MF3D_TOCReferencePtr	tempPtr;

		inMetafilePtr->tocStuff.refSeed = refSeed;
		inMetafilePtr->tocStuff.typeSeed = typeSeed;
		inMetafilePtr->tocStuff.numReferences = numLabels;
		tocLabelTableSize = numLabels *
				sizeof(*inMetafilePtr->tocStuff.references);	
		tempPtr = (MF3D_TOCReferencePtr)MF3D_Malloc(tocLabelTableSize);
		if (tempPtr == NULL)
		{	result = kMF3DErrOutOfMemory;
		}
		else
		{	inMetafilePtr->tocStuff.references = tempPtr;
			memcpy(inMetafilePtr->tocStuff.references, tocLabelNames,
					(size_t)tocLabelTableSize);
		}
	}

	/* Now, go through objTable and change the label lengths to actual
	 * ref IDs!
	 */
	if (result == kMF3DNoErr)
	{	result = MF3D_ConvertTableLabels(inMetafilePtr, numLabels,
				tocLabelNames, numObjects, objTable);
	}

PreprocessTextFileAbort:
	/* Free any TOC labels that did not find matches */
	for (labelIndex = 0; labelIndex < numLabels; ++labelIndex)
	{	MF3D_Free(tocLabelNames[labelIndex].ref.name);
	}

	MF3D_Free(tocLabelNames);
	MF3D_Free(tocLocations);

	if (result != kMF3DNoErr)
	{	/* Free the objTable only if we are quitting */
		MF3D_Free(objTable);
	}
	else
	{	
	  /* XXXX */
		MF3D_Free(inMetafilePtr->readBuffer.buf);
		inMetafilePtr->readBuffer.buf = NULL;

		/* Reset the file pointer to the beginning */
		AssignInt64(firstObj, objTable[0].objLocation);

		result = MF3DSeekPosition(inMetafilePtr, firstObj);
	}

	return result;
}

/*==============================================================================
 *	MF3D_GetRefNameT
 *
 *	Get reference name
 *
 *	Allocates and returns CStringPtr or NULL if no match found.
 *==============================================================================
 */
MF3DCStringPtr
MF3D_GetRefNameT(
	MF3D_FilePtr		inMetafilePtr,
	MF3DReferenceID		inRefID)
{
	MF3DUns32				refIndex;
	MF3D_TOCReferencePtr	labelTablePtr;
	MF3DCStringPtr			result;

	result = NULL;

	labelTablePtr = inMetafilePtr->tocStuff.references;
	MFASSERT(inMetafilePtr->tocStuff.numReferences == 0 ||
			labelTablePtr != NULL);

	for (refIndex = inMetafilePtr->tocStuff.numReferences;
			refIndex > 0; --refIndex)
	{	if (labelTablePtr->refID == inRefID)
		{	result = MF3D_DuplicateCString(labelTablePtr->ref.name);
			break;
		}
		++labelTablePtr;
	}

	return result;
}

/*==============================================================================
 *	MF3D_PostprocessTextFile
 *
 *	Dispose of TOC structures
 *==============================================================================
 */
MF3DErr
MF3D_PostprocessTextFile(
	MF3D_FilePtr		inMetafilePtr)
{
	MF3DUns32				refIndex;
	MF3D_TOCReferencePtr	labelTablePtr;
	MF3DBinaryFilePosition	zero;

	/* Do not close things if we were just resolving a reference in this file */
	SetInt64ToZero(zero);
	if (CompareInt64(inMetafilePtr->resStuff.returnLoc, zero) == 0)
	{	labelTablePtr = inMetafilePtr->tocStuff.references;
		MFASSERT(inMetafilePtr->tocStuff.numReferences == 0 ||
				labelTablePtr != NULL);
	
		for (refIndex = inMetafilePtr->tocStuff.numReferences;
				refIndex > 0; --refIndex)
		{	MF3D_Free(labelTablePtr->ref.name);
			++labelTablePtr;
		}

		MF3D_Free(inMetafilePtr->tocStuff.references);
		MF3D_Free(inMetafilePtr->objTable.objects);
	}

	return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_CompareObjNames
 *
 *	Match the name of an object. Return 0 if it matches; nonzero otherwise.
 *==============================================================================
 */
MF3DInt32
MF3D_CompareObjNames(
	const char		*obj1,
	const char		*obj2)
{
	return MF3D_CompareLabelNames(obj1, obj2);
}

/*==============================================================================
 *	MF3D_CompareLabelNames
 *
 *	Match the name of a label. Return 0 if it matches; nonzero otherwise.
 *==============================================================================
 */
MF3DInt32
MF3D_CompareLabelNames(
	const char		*label1,
	const char		*label2)
{
	MFASSERT(label1 != NULL || label2 != NULL);

	if (label1 == NULL || label2 == NULL)
		return 1;
	return MF3D_CmpStrInsensitive(label1, label2);
}

/*==============================================================================
 *	MF3D_ScanTextBuffer
 *
 *	Basically we just need something that does fscanf for one argument.
 *	After reading the argument, skip spaces and comments that follow.
 *==============================================================================
 */
MF3DErr
MF3D_ScanTextBuffer(
	MF3D_FilePtr	inMetafilePtr,
	const char		*inFormatStr,
	void			*outBuffer)
{
	char		*bufPtr;
	MF3DUns32	bufPos;
	int			numCharsRead;
	char		formatStr[kMF3D_MaxFormatLength + 2];
	MF3DErr		result;

	MFASSERT(inMetafilePtr->readBuffer.buf != NULL);
	MFASSERT(strlen(inFormatStr) <= kMF3D_MaxFormatLength);

	result = kMF3DNoErr;

	bufPos = inMetafilePtr->readBuffer.bufPos;
	bufPtr = inMetafilePtr->readBuffer.buf;

	/* Add %n to the format string, so that we can tell how many characters
	 * get read.
	 */
	strcpy(formatStr, inFormatStr);
	strcat(formatStr, "%n");
	numCharsRead = 0;
	sscanf(&bufPtr[bufPos], formatStr, outBuffer, &numCharsRead);

	bufPos += numCharsRead;
	MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);

	if (numCharsRead <= 0)
		result = kMF3DErrCantParse;

	MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);
	inMetafilePtr->readBuffer.bufPos = bufPos;

	if (result == kMF3DNoErr)
		result = MF3D_SkipWhitespace(inMetafilePtr);

	return result;
}

/*==============================================================================
 *	MF3D_SkipWhitespace
 *
 *	Read and ignore whitespace from text buffer
 *==============================================================================
 */
MF3DErr
MF3D_SkipWhitespace(
	MF3D_FilePtr	inMetafilePtr)
{
	char		*bufPtr;
	MF3DUns32	bufPos;
	int			numCharsRead;
	char		commentChar[2];

	MFASSERT(inMetafilePtr->readBuffer.buf != NULL);

	bufPos = inMetafilePtr->readBuffer.bufPos;
	bufPtr = inMetafilePtr->readBuffer.buf;

	/* Skip over any whitespace */
	numCharsRead = 0;	/* init to 0 in case there is no whitespace */
	sscanf(&bufPtr[bufPos], kMF3D_IngoreWSNFormat, &numCharsRead);
	bufPos += numCharsRead;
	MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);

	/* If the next character is the comment character, skip the rest
	 * of the line.
	 */
	while (sscanf(&bufPtr[bufPos],
				kMF3D_CommentLineNFormat,	/* grab one # char & numChars read */
				&commentChar, &numCharsRead) > 0)
	{	bufPos += numCharsRead;
		numCharsRead = 0;
		sscanf(&bufPtr[bufPos],
				kMF3D_IgnoreEOLNFormat,		/* ignore rest to EOL	*/
				&numCharsRead);
		bufPos += numCharsRead;
		MFASSERT(numCharsRead > 0);			/* should have been a newline */
		numCharsRead = 0;
		sscanf(&bufPtr[bufPos],
				kMF3D_IngoreWSNFormat,		/* skip whitespace		*/
				&numCharsRead);
		bufPos += numCharsRead;
	}

	MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);
	inMetafilePtr->readBuffer.bufPos = bufPos;

	return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_ReadOpenParen
 *
 *	Read an open paren character and return
 *==============================================================================
 */
MF3DErr
MF3D_ReadOpenParen(
	MF3D_FilePtr	inMetafilePtr)
{
	char	openParen[2];

	return MF3D_ScanTextBuffer(inMetafilePtr, kMF3D_BeginCharFormat, openParen);
}

/*==============================================================================
 *	MF3D_SkipText
 *
 *	Skip over text (used for skipping over resolution labels).
 *	###NOTE### will return an error if not least two characters of text
 *==============================================================================
 */
MF3DErr
MF3D_SkipText(
	MF3D_FilePtr	inMetafilePtr)
{
	/* ScanTextBuffer expects at least one void * parameter */
	char	junkChar[2];

	return MF3D_ScanTextBuffer(inMetafilePtr, kMF3D_IgnoreTextFormat, junkChar);
}

/*==============================================================================
 *	MF3D_ReadObjectStuff
 *
 *	Read the object name and get the objStuff.
 *==============================================================================
 */
MF3DErr
MF3D_ReadObjectStuff(
	MF3D_FilePtr		inMetafilePtr,
	MF3D_ObjStuffPtr	*outObjectStuffPtr,
	MF3DObjType			*outObjectType)
{
	char		objName[kMF3D_MaxObjNameLength];
	MF3DErr		result;

	result = MF3D_ScanTextBuffer(inMetafilePtr, kMF3D_NameFormat, objName);

	if (result == kMF3DNoErr)
	{	result = MF3D_FindObjectFromName(objName, outObjectStuffPtr,
				outObjectType);
	}

	return result;
}

/*==============================================================================
 *	MF3D_ReadTextString
 *
 *	Read a text string
 *	*outStringPtr only valid if kMF3DNoErr returned.
 *==============================================================================
 */
MF3DErr
MF3D_ReadTextString(
	MF3D_FilePtr		inMetafilePtr,
	MF3DCStringPtr		*outStringPtr)
{
	MF3DErr				result;
	MF3D_BuildString	tempStr;
	char				*bufPtr;
	char				c;
	MF3DUns32			bufPos;
	int					numCharsRead;
	

	result = kMF3DNoErr;

	bufPos = inMetafilePtr->readBuffer.bufPos;
	bufPtr = inMetafilePtr->readBuffer.buf;

	numCharsRead = 0;

	sscanf(&(bufPtr[bufPos]), kMF3D_StringBeginNFormat, &c, &numCharsRead);
	/* XXXX */
	bufPtr = inMetafilePtr->readBuffer.buf;

	if (numCharsRead <= 0)
		result = kMF3DErrUnquotedString;

	if (result == kMF3DNoErr)
	{	bufPos += numCharsRead;
		MFASSERT(c == kMF3D_StringBeginChar);
		MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);

		result = MF3D_BuildString_New(&tempStr);
	}

	/* Scan the text buffer, copying characters until we hit the end quote */
	while (result == kMF3DNoErr)
	{	numCharsRead = 0;
		sscanf(&bufPtr[bufPos], "%c%n", &c, &numCharsRead);
		if (numCharsRead <= 0)
			result = kMF3DErrCantParse;

		if (result == kMF3DNoErr)
		{	bufPos += numCharsRead;
			MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);

			if (c == kMF3D_StringEndChar)
			{	MF3D_BuildString_EndString(&tempStr);
				/* Copy the string pointer. Do NOT delete. */
				*outStringPtr = tempStr.str;
				break;		/* ### NORMAL EXIT IS HERE ### */
			}
			else if (c == kMF3D_StringEscapeChar)
			{	sscanf(&bufPtr[bufPos], "%c%n", &c, &numCharsRead);
				if (numCharsRead <= 0)
					result = kMF3DErrCantParse;
				else
					bufPos += numCharsRead;

				if (result == kMF3DNoErr)
				{	switch (c)
					{	case 'a':	c = '\a'; break;
						case 'b':	c = '\b'; break;
						case 'f':	c = '\f'; break;
						case 'n':	c = '\n'; break;
						case 'r':	c = '\r'; break;
						case 't':	c = '\t'; break;
						case 'v':	c = '\v'; break;
						case '0':	/* NOTE: ### \123 NOT SUPPORTED ### */
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						case 'x':	/* NOTE: ### \x0F NOT SUPPORTED ### */
						default:	break;
					}
					MF3D_BuildString_AddChar(&tempStr, c);
				}
			}
			else
			{	MF3D_BuildString_AddChar(&tempStr, c);
			}
		}

		if (result != kMF3DNoErr)
		{	/* Should already be free */
			MFASSERT(tempStr.str == NULL);
		}
	}

	MFASSERT(bufPos <= inMetafilePtr->readBuffer.bufSize);
	inMetafilePtr->readBuffer.bufPos = bufPos;

	if (result == kMF3DNoErr)
		result = MF3D_SkipWhitespace(inMetafilePtr);

	return result;
}

/*==============================================================================
 *	MF3D_ReadTextLabel
 *
 *	Read an unquoted text string
 *	*outStringPtr only valid if kMF3DNoErr returned.
 *==============================================================================
 */
MF3DErr
MF3D_ReadTextLabel(
	MF3D_FilePtr		inMetafilePtr,
	MF3DCStringPtr		*outStringPtr)
{
	MF3DErr				result;
	MF3D_BuildString	tempStr;

	result = MF3D_BuildString_New(&tempStr);

	/* Scan the text buffer, copying characters until we hit a right arrow */
	while (result == kMF3DNoErr)
	{	char		c;

		result = MF3D_ScanTextBuffer(inMetafilePtr,
			"%c", &c);
		if (result == kMF3DNoErr)
		{	if (c == kMF3D_FilePtrChar)
			{	MF3D_BuildString_EndString(&tempStr);
				/* Copy the string pointer. Do NOT delete. */
				*outStringPtr = tempStr.str;
				break;		/* ### NORMAL EXIT IS HERE ### */
			}
			else
			{	MF3D_BuildString_AddChar(&tempStr, c);
			}
		}

		if (result != kMF3DNoErr)
		{	/* Should already be free */
			MFASSERT(tempStr.str == NULL);
		}
	}

	return result;
}

/*==============================================================================
 *	MF3D_BuildString_New
 *	MF3D_BuildString_AddChar
 *	MF3D_BuildString_EndString
 *	MF3D_BuildString_Delete
 *
 *	Temporary routines to dynamically build a string of unknown length.
 *==============================================================================
 */
MF3DErr
MF3D_BuildString_New(
	MF3D_BuildString	*outStringPtr)
{
	MF3D_AllocEmpty(outStringPtr->str);
	outStringPtr->pos = 0;
	return kMF3DNoErr;
}

MF3DErr
MF3D_BuildString_AddChar(
	MF3D_BuildString	*ioStringPtr,
	char				inAddChar)
{
	MF3DErr	result;

	result = kMF3DNoErr;
	if ((ioStringPtr->pos % kMF3D_StringBufferChunk) == 0)
		result = MF3D_Reallocate(&ioStringPtr->str,
				ioStringPtr->pos + kMF3D_StringBufferChunk);

	if (result == kMF3DNoErr)
		ioStringPtr->str[ioStringPtr->pos++] = inAddChar;
	else
		MF3D_BuildString_Delete(ioStringPtr);

	return result;
}

MF3DErr
MF3D_BuildString_EndString(
	MF3D_BuildString	*ioStringPtr)
{
	MF3DErr	result;

	result = MF3D_BuildString_AddChar(ioStringPtr, '\0');

	if (result == kMF3DNoErr)
	{	ioStringPtr->str = (MF3DCStringPtr)MF3D_Realloc(ioStringPtr->str,
				ioStringPtr->pos);

		/* pointer should still be valid because we never realloc more */
		MFASSERT(ioStringPtr->str != NULL);
	}

	return result;
}

MF3DErr
MF3D_BuildString_Delete(
	MF3D_BuildString	*ioStringPtr)
{
	MF3D_Free(ioStringPtr->str);
	ioStringPtr->str = NULL;
	return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_NumToString
 *
 *	Convert an unsigned number to its decimal string equivalent
 *==============================================================================
 */
void
MF3D_NumToString(
	MF3DUns32		inNumber,
	MF3DCStringPtr	outString)
{
	MF3DUns32	n, mod;
	char		revString[kMF3D_MaxDigitsInUns32 + 1];
	int			i;

	n = inNumber;
	revString[i = kMF3D_MaxDigitsInUns32] = '\0';

	while (n > 0)
	{	MFASSERT(i > 0);

		mod = n - ((n/10) * 10);

		/* Make no assumptions about character ordering */
		switch (mod)
		{	case 0:	revString[--i] = '0'; break;
			case 1:	revString[--i] = '1'; break;
			case 2:	revString[--i] = '2'; break;
			case 3:	revString[--i] = '3'; break;
			case 4:	revString[--i] = '4'; break;
			case 5:	revString[--i] = '5'; break;
			case 6:	revString[--i] = '6'; break;
			case 7:	revString[--i] = '7'; break;
			case 8:	revString[--i] = '8'; break;
			case 9:	revString[--i] = '9'; break;
			default: MFASSERT(0); break;
		}
		n /= 10;
	}

	strcpy(outString, &revString[i]);
}

/*==============================================================================
 *	MF3D_TextToHex
 *
 *	Convert a text character to its hexadecimal equivalent
 *==============================================================================
 */
MF3DUns8
MF3D_TextToHex(
	char inChar)
{
	/* Make no assumptions about character ordering */
	switch(inChar)
	{	case '0':	return(0);
		case '1':	return(1);
		case '2':	return(2);
		case '3':	return(3);
		case '4':	return(4);
		case '5':	return(5);
		case '6':	return(6);
		case '7':	return(7);
		case '8':	return(8);
		case '9':	return(9);
		case 'A':
		case 'a':	return(10);
		case 'B':
		case 'b':	return(11);
		case 'C':
		case 'c':	return(12);
		case 'D':
		case 'd':	return(13);
		case 'E':
		case 'e':	return(14);
		case 'F':
		case 'f':	return(15);
		default:	MFASSERT(0); return(0);
	}
}
