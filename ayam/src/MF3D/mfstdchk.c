/*==============================================================================
 *
 *	File:		MFSTDCHK.C
 *
 *	Function:	Our MF3DProcs which interface to standard C routines.
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
 *		FB8_JRK	Segmentation
 *		Fabio	Changed file name to 8 characters
 *		F31_RWW	Avoid error when reading zero-size objects (binary).
 *		F29_RWW	File created.
 *==============================================================================
 */

#include "mfstdchk.h"

#include <limits.h>
#include <stdio.h>

#include "mferrors.h"
#include "mfint64.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

MF3DErr
MF3DStdCOpenHook(
	MF3DDataFormatEnum	inDataFormat,
	MF3DUserOpenDataPtr	inOpenDataPtr,
	MF3DUserFilePtr *	outFilePtr)
{
	MF3DUserFilePtr		filePtr;
	MF3DStdCOpenDataPtr	stdCOpenDataPtr;
	MF3DErr				result;
	char				*permissionString;

	stdCOpenDataPtr = (MF3DStdCOpenDataPtr)inOpenDataPtr;
	if (MF3DIsTextFormat(inDataFormat))
	{	if (stdCOpenDataPtr->permission == kMF3DStdCReadPerm)
			permissionString = kMF3D_StdCTextReadPermStr;
		else
			permissionString = kMF3D_StdCTextWritePermStr;
	}
	else
	{	if (stdCOpenDataPtr->permission == kMF3DStdCReadPerm)
			permissionString = kMF3D_StdCBinaryReadPermStr;
		else
			permissionString = kMF3D_StdCBinaryWritePermStr;
	}

	filePtr = (MF3DUserFilePtr)fopen(stdCOpenDataPtr->filename,
			permissionString);

	if (filePtr != NULL)
	{	result = kMF3DNoErr;
		*outFilePtr = filePtr;
	}
	else
	{	result = kMF3DErrFailedToOpen;
	}

	return result;
}

MF3DErr
MF3DStdCReadHook(
	MF3DUserFilePtr		inFilePtr,
	MF3DSize			inCharsToRead,
	char *				outBuffer)
{
	size_t	bytesRead;
	MF3DErr	result;

	result = kMF3DNoErr;

	if (inCharsToRead > 0)
	{	bytesRead = fread(outBuffer, sizeof(char), (size_t)inCharsToRead,
				(FILE *)inFilePtr);

		if (bytesRead == 0)
		{	if (feof((FILE *)inFilePtr))
				result = kMF3DErrReadFailedEOF;
			else
				result = kMF3DErrReadFailed;
		}
		else if (bytesRead != (size_t)inCharsToRead)
		{	if (feof((FILE *)inFilePtr))
				result = kMF3DErrReadPartialEOF;
			else
				result = kMF3DErrReadPartial;
		}
	}

	return result;
}

MF3DErr
MF3DStdCWriteHook(
	MF3DUserFilePtr		inFilePtr,
	MF3DSize			inCharsToWrite,
	const char *		inBuffer)
{
	size_t	bytesWritten;
	MF3DErr	result;

	result = kMF3DNoErr;
	bytesWritten = fwrite(inBuffer, sizeof(char), (size_t)inCharsToWrite,
			(FILE *)inFilePtr);

	if (bytesWritten == 0)
	{	if (feof((FILE *)inFilePtr))
			result = kMF3DErrWriteFailedEOF;
		else
			result = kMF3DErrWriteFailed;
	}
	else if (bytesWritten != (size_t)inCharsToWrite)
	{	if (feof((FILE *)inFilePtr))
			result = kMF3DErrWritePartialEOF;
		else
			result = kMF3DErrWritePartial;
	}

	return result;
}

MF3DErr
MF3DStdCTellHook(
	MF3DUserFilePtr				inFilePtr,
	MF3DBinaryFilePosition *	outPositionPtr)
{
	long	filePosition;
	MF3DErr	result;

	result = kMF3DNoErr;

	filePosition = ftell((FILE *)inFilePtr);
	if (filePosition != -1L)
	{	Int32ToUns64(filePosition, *outPositionPtr);
	}
	else
	{	result = kMF3DErrTellFailed;
	}

	return result;
}

MF3DErr
MF3DStdCSeekHook(
	MF3DUserFilePtr			inFilePtr,
	MF3DBinaryFilePosition	inPosition)
{
	MF3DErr					result;
	long					longPos;
	int						fseekResult;
	MF3DBinaryFilePosition	longMax;

	result = kMF3DNoErr;

	/*
	 * Standard C library only allows seeks in the range of a long.
	 * If you really want to seek farther, you will have to rewrite
	 * this yourself.
	 */
	Int32ToUns64(LONG_MAX, longMax);
	if (CompareInt64(inPosition, longMax) > 0)
		result = kMF3DErrSeekOutOfRange;

	if (result == kMF3DNoErr)
	{	Uns64ToUns32(inPosition, longPos);
		fseekResult = fseek((FILE *)inFilePtr, longPos, SEEK_SET);
	}

	if (fseekResult != 0)
		result = kMF3DErrSeekFailed;

	return result;
}

MF3DErr
MF3DStdCCloseHook(
	MF3DUserFilePtr		inFilePtr)
{
	int	closeResult;

	closeResult = fclose((FILE *)inFilePtr);

	return closeResult == 0 ? kMF3DNoErr : kMF3DErrCloseEOF;
}
