#ifndef MF3DTYPES_H
#define	MF3DTYPES_H
/*==============================================================================
 *
 *	File:		MFTYPES.H
 *
 *	Function:	External typedefs used by API routines.
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
 *		Fabio	Changed file name to 8 characters
 *		F2R_RWW	No more complex objects.
 *		F26_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mfsystyp.h"

/*==============================================================================
 *	Basic Types
 *==============================================================================
 */

typedef MF3DUns32 MF3DSize;

typedef struct MF3DPoint2D
  {
    MF3DFloat32 x;
    MF3DFloat32 y;
  }
MF3DPoint2D, *MF3DPoint2DPtr;	/* 2-D point                            */
typedef struct MF3DPoint3D
  {
    MF3DFloat32 x;
    MF3DFloat32 y;
    MF3DFloat32 z;
  }
MF3DPoint3D, *MF3DPoint3DPtr;	/* 3-D point                            */
typedef struct MF3DRationalPoint3D
  {
    MF3DFloat32 x;
    MF3DFloat32 y;
    MF3DFloat32 w;
  }
MF3DRationalPoint3D, *MF3DRationalPoint3DPtr;	/* 3-D rational point           */
typedef struct MF3DRationalPoint4D
  {
    MF3DFloat32 x;
    MF3DFloat32 y;
    MF3DFloat32 z;
    MF3DFloat32 w;
  }
MF3DRationalPoint4D, *MF3DRationalPoint4DPtr;	/* 4-D rational point           */

typedef struct MF3DVector3D
  {
    MF3DFloat32 x;
    MF3DFloat32 y;
    MF3DFloat32 z;
  }
MF3DVector3D, *MF3DVector3DPtr;	/* 3-D vector                           */

typedef struct MF3DParam2D
  {
    MF3DFloat32 u;
    MF3DFloat32 v;
  }
MF3DParam2D, *MF3DParam2DPtr;	/* 2-D parametrization          */

typedef struct MF3DRGBColor
  {
    MF3DFloat32 red;
    MF3DFloat32 green;
    MF3DFloat32 blue;
  }
MF3DRGBColor, *MF3DRGBColorPtr;	/* RGB color                            */

typedef MF3DFloat32 MF3DMatrix3x3[3][3];
typedef MF3DFloat32 MF3DMatrix4x4[4][4];
typedef const MF3DFloat32 MF3DConstMatrix3x3[3][3];
typedef const MF3DFloat32 MF3DConstMatrix4x4[4][4];

typedef void *MF3DRawDataPtr;	/* pointer to unknown data      */
typedef const void *MF3DConstRawDataPtr;	/* pointer to constant data     */

/*==============================================================================
 *	Flags and Enums
 *==============================================================================
 */
enum
  {
    kMF3DBooleanFalse,
    kMF3DBooleanTrue
  };

typedef MF3DUns32 MF3DLabelValue;
typedef MF3DLabelValue MF3DEnumData, MF3DFlagData;
typedef MF3DEnumData MF3DBoolean;

/*==============================================================================
 *	System type pointers
 *==============================================================================
 */
typedef MF3DUns8 *MF3DUns8Ptr;
typedef MF3DUns16 *MF3DUns16Ptr;
typedef MF3DUns32 *MF3DUns32Ptr;
typedef MF3DUns64 *MF3DUns64Ptr;
typedef MF3DInt8 *MF3DInt8Ptr;
typedef MF3DInt16 *MF3DInt16Ptr;
typedef MF3DInt32 *MF3DInt32Ptr;
typedef MFINT64 *MF3DInt64Ptr;

/*==============================================================================
 *	External Types
 *==============================================================================
 */

typedef MF3DInt32 MF3DObjType;	/* object type                                          */

typedef MF3DInt32 MF3DErr;	/* error result                                         */
typedef void *MF3DUserOpenDataPtr;	/* user-defined data for file-open  */
typedef void *MF3DUserFilePtr;	/* user-defined file (e.g. FILE *)      */

typedef enum
  {
    kMF3DFormatBinary,		/* IEEE binary                                          */
    kMF3DFormatSwappedBinary,	/* IEEE swapped binary                          */
    kMF3DFormatText		/* US English 7-bit ASCII                       */
  }
MF3DDataFormatEnum;		/* storage format                                       */

#define	MF3DIsTextFormat(dataformat)		((dataformat) == kMF3DFormatText)

/*==============================================================================
 * MF3DPositionReference:
 *	For binary files, an unsigned offset relative to the metafile header
 *	For text files, a textname pointing to a resolution label somewhere
 *		in the metafile
 *==============================================================================
 */
typedef MF3DUns64 MF3DBinaryFilePosition, *MF3DBinaryFilePositionPtr;
typedef const MF3DUns64 *MF3DConstBinaryFilePositionPtr;
typedef MF3DCStringPtr MF3DTextFilePosition;

typedef union MF3DFilePosition
  {
    MF3DBinaryFilePosition binary;
    MF3DTextFilePosition text;
  }
MF3DFilePosition;

typedef struct MF3DPositionReference
  {
    MF3DDataFormatEnum format;
    MF3DFilePosition location;
  }
MF3DPositionReference, *MF3DPositionReferencePtr;
typedef const MF3DPositionReference *MF3DConstPositionReferencePtr;

/*==============================================================================
 *	Object Reference Info
 *==============================================================================
 */
typedef MF3DUns32 MF3DReferenceID;
#define	kMF3DUnreferencedLabel	0xFFFFFFFF

typedef struct MF3DReferenceInfo
  {
    MF3DReferenceID refID;	/* reference ID                         */
    MF3DCStringPtr refName;	/* (opt) name of reference      */
  }
MF3DReferenceInfo, *MF3DReferenceInfoPtr;

/*==============================================================================
 *	Object Type Macros
 *
 *	MF3DReadAnObject() will return an object whose objectType field contains
 *	a four-character constant specifying what type of object it is.
 *==============================================================================
 */
#define	MF3DFourCharacterConstant(a,b,c,d) 			\
			((const MF3DUns32)		 				\
			((const MF3DUns32) (a) << 24) | 		\
			((const MF3DUns32) (b) << 16) |			\
			((const MF3DUns32) (c) << 8)  | 		\
			((const MF3DUns32) (d)))

#define	MF3DObjectType(a,b,c,d)						\
			((MF3DObjType) MF3DFourCharacterConstant(a,b,c,d))

/*==============================================================================
 *	Standard Object Header
 *==============================================================================
 */
#define	MF3DObjHeader														\
	MF3DObjType					objectType;		/* Object ID			*/	\
	MF3DReferenceInfoPtr		refInfo;		/* (opt) Ref num & name	*/	\

#endif
