#ifndef MF3D_READ_H
#define	MF3D_READ_H
/*==============================================================================
 *
 *	File:		MFTEXTRD.H
 *
 *	Function:	Internal Text Read equates and prototypes
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
 *		F3A_RWW	TOC stuff works.
 *		F2K_RWW	Moved a bunch of equates to MFTEXTST.H
 *		F2E_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"

#include "mfintobj.h"
#include "mfobjtyp.h"

/* Arbitrarily large maximum length of an object name.
 * In practice, we know they are much smaller.
 */
#define	kMF3D_MaxObjNameLength	1024

/* Amount to grow ObjTable during preprocessing (must be power of 2) */
#define	kMF3D_ObjTableChunk		32

/* Temporary buffer for building strings of unknown length */
typedef struct MF3D_BuildString
  {
    MF3DCStringPtr str;
    MF3DUns32 pos;
  }
MF3D_BuildString, *MF3D_BuildStringPtr;

/* Amount to grow BuildString buffer (must be power of 2) */
#define	kMF3D_StringBufferChunk		1024

/* NumToString conversion */
#define	kMF3D_MaxDigitsInUns32		10

/*==============================================================================
 *	Preprocessing functions
 *==============================================================================
 */
char MF3D_ReadSingleChar (MF3D_FilePtr inMetafilePtr);
MF3DErr MF3D_ReadUntilCloseParen (MF3D_FilePtr inMetafilePtr);
MF3DErr MF3D_GetTOCLabels (MF3D_FilePtr inMetafilePtr,
			   MF3DUns32 inNumTocs,
			   MF3DConstBinaryFilePositionPtr inTocLocations,
			   MF3DUns32 * outNumLabels,
			   MF3D_TOCReferencePtr * outTocLabelNames,
			   MF3DUns32 * outRefSeed, MF3DInt32 * outTypeSeed);
MF3DErr MF3D_ConvertTableLabels (MF3D_FilePtr inMetafilePtr,
				 MF3DUns32 inNumLabels,
				 MF3D_TOCReferencePtr ioTocLabelNames,
				 MF3DUns32 inNumObjects,
				 MF3D_ObjectTableEntryPtr ioObjTable);
MF3DErr MF3D_PreprocessTextFile (MF3D_FilePtr inMetafilePtr);
MF3DCStringPtr MF3D_GetRefNameT (MF3D_FilePtr inMetafilePtr,
				 MF3DReferenceID inRefID);
MF3DErr MF3D_PostprocessTextFile (MF3D_FilePtr inMetafilePtr);

/*==============================================================================
 *	Comparison functions
 *==============================================================================
 */
MF3DInt32 MF3D_CompareObjNames (const char *obj1, const char *obj2);
MF3DInt32 MF3D_CompareLabelNames (const char *label1, const char *label2);

/*==============================================================================
 *	Scan functions
 *==============================================================================
 */
MF3DErr MF3D_ScanTextBuffer (MF3D_FilePtr inMetafilePtr,
			     const char *inFormatStr, void *outBuffer);

MF3DErr MF3D_SkipWhitespace (MF3D_FilePtr inMetafilePtr);
MF3DErr MF3D_ReadOpenParen (MF3D_FilePtr inMetafilePtr);
MF3DErr MF3D_SkipText (MF3D_FilePtr inMetafilePtr);
MF3DErr MF3D_ReadObjectStuff (MF3D_FilePtr inMetafilePtr,
			      MF3D_ObjStuffPtr * outObjectStuffPtr,
			      MF3DObjType * outObjectType);
MF3DErr MF3D_ReadTextString (MF3D_FilePtr inMetafilePtr,
			     MF3DCStringPtr * outStringPtr);
MF3DErr MF3D_ReadTextLabel (MF3D_FilePtr inMetafilePtr,
			    MF3DCStringPtr * outStringPtr);

/*==============================================================================
 *	BuildString functions
 *==============================================================================
 */
MF3DErr MF3D_BuildString_New (MF3D_BuildString * outStringPtr);
MF3DErr MF3D_BuildString_AddChar (MF3D_BuildString * ioStringPtr,
				  char inAddChar);
MF3DErr MF3D_BuildString_EndString (MF3D_BuildString * ioStringPtr);
MF3DErr MF3D_BuildString_Delete (MF3D_BuildString * ioStringPtr);

/*==============================================================================
 *	Conversion functions
 *==============================================================================
 */
void MF3D_NumToString (MF3DUns32 inNumber,
		       MF3DCStringPtr outString);
MF3DUns8 MF3D_TextToHex (char inChar);

#endif
