#ifndef	MF3DERRORS_H
#define	MF3DERRORS_H
/*==============================================================================
 *
 *	File:		MFERRORS.H
 *
 *	Function:	Error codes
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
 *		F26_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#ifndef kBaseMF3DErr
#define	kBaseMF3DErr	12000
#endif

enum
  {
    kMF3DNoErr = 0,

/* 12000: Returned when there are no more objects to be read in the file. */
    kMF3DNoMoreObjects = kBaseMF3DErr,

/* 12001 */
    kMF3DErrInvalidParameter,
/* 12002 */
    kMF3DErrOutOfMemory,
/* 12003 */
    kMF3DErrNoObjectsFound,
/* 12004 */
    kMF3DErrCantParse,
/* 12005 */
    kMF3DErrDidntReadEntireObj,
/* 12006 */
    kMF3DErrObjHasTooManyLabels,
/* 12007 */
    kMF3DErrIllegalObjName,
/* 12008 */
    kMF3DErrUnquotedString,
/* 12009 */
    kMF3DErrRawDataOddNumberOfHexChars,
/* 12010 */
    kMF3DErrIllegalEnum,
/* 12011 */
    kMF3DErrIllegalDataType,
/* 12012 */
    kMF3DErrWrongObjType,
/* 12013 */
    kMF3DErrNotAReferenceObj,
/* 12014 */
    kMF3DErrReferenceNotFound,
/* 12015 */
    kMF3DErrTooManyStorageObjects,
/* 12016: User Type must be negative, unique, and greater than 0x80000000 */
    kMF3DErrIllegalUserObjectType,
/* 12017 */
    kMF3DErrNIndicesLessThanZero,
/* 12018 */
    kMF3DErrNIndicesGreaterThanNObjects,
/* 12019 */
    kMF3DErrNumCornersIsZero,
/* 12020 */
    kMF3DErrNumEdgesIsZero,
/* 12021 */
    kMF3DErrNumFacesIsZero,
/* 12022: Sorry, minimum order is required */
    kMF3DErrOrderTooSmall,
/* 12023 */
    kMF3DErrTooFewPoints,
/* 12024 */
    kMF3DErrNumContoursIsZero,
/* 12025 */
    kMF3DErrTooFewVertices,
/* 12026 */
    kMF3DErrWidthOrHeightIsZero,
/* 12027: Missing EndContainer ')', EndGroup, or Read was interrupted before
 * a matching ) or EndGroup was read.
 */
    kMF3DErrNotEnoughEndGroups,
/* 12028 */
    kMF3DErrTooManyEndGroups,
/* 12029 */
    kMF3DErrTooManyEndContainers,
/* 12030 */
    kMF3DErrContainedObjTooBig,
/* 12031 */
    kMF3DErrCantWrite,
/* 12032: Reference ID must be less than TOC refSeed */
    kMF3DErrIllegalRefID,
/* 12033: Objects must have unique refs */
    kMF3DErrTwoObjectsWithSameRefID,
/* 12034: Objects must have unique refs */
    kMF3DErrTwoObjectsWithSameRefName,
/* 12035: BeginGroup requires a group obj */
    kMF3DErrGroupTypeNotSpecified,

/* MF3DOpenProcPtr errors */
    kMF3DErrFailedToOpen,

/* MF3DReadProcPtr errors */
    kMF3DErrReadFailedEOF,
    kMF3DErrReadFailed,
    kMF3DErrReadPartialEOF,
    kMF3DErrReadPartial,

/* MF3DWriteProcPtr errors */
    kMF3DErrWriteFailedEOF,
    kMF3DErrWriteFailed,
    kMF3DErrWritePartialEOF,
    kMF3DErrWritePartial,

/* MF3DTellProcPtr errors */
    kMF3DErrTellFailed,

/* MF3DSeekProcPtr errors */
    kMF3DErrSeekFailed,
    kMF3DErrSeekOutOfRange,

/* MF3DCloseProcPtr errors */
    kMF3DErrCloseEOF,

/* ### Full 64-bit numbers cannot be written to text files right now. ### */
    kMF3DErrUnsupportedUns64,
    kMF3DErrUnsupportedInt64,

/* We don't support file formats prior to 1.0 */
    kMF3DErrUnsupportedMetafileFormat,

/* Fake error so that I can add more errors without worrying about commas */
    kMF3DTopError
  };

enum
  {
    kMF3DWarnNewerMetafileFormat = kMF3DTopError + 1,
    kMF3DWarnWrongUnknownTypeFormat,

    kMF3DTopWarning
  };

#endif
