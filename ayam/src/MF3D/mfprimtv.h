#ifndef	MF3D_PRIMITIVES_H
#define	MF3D_PRIMITIVES_H
/*==============================================================================
 *
 *	File:		MFPRIMTV.H
 *
 *	Function:	Primitive I/O routines
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
 *		Fabio	Changed file name to 8 characters
 *		F3A_RWW	TOC stuff works.
 *		F2D_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"
#include "mflabels.h"
#include "mfobjtyp.h"

MF3DErr MF3D_GetPrimitivesAccessor (MF3DDataFormatEnum inDataFormat,
				    MF3D_IOPrimitivesTable * outPrimitives);

/*==============================================================================
 *	Validation routines
 *
 *	do nothing at the moment
 *==============================================================================
 */
#define	MF3D_ValidateReadSize(metafile, size)
#define	MF3D_ValidateWriteSize(metafile, size)

/*==============================================================================
 *	I/O Dispatchers
 *==============================================================================
 */
/* integers */
MF3DErr MF3D_Uns8Read (MF3D_FilePtr metafile, MF3DUns8 * data);
MF3DErr MF3D_Uns8Write (MF3D_FilePtr metafile, MF3DUns8 data);
MF3DErr MF3D_Uns16Read (MF3D_FilePtr metafile, MF3DUns16 * data);
MF3DErr MF3D_Uns16Write (MF3D_FilePtr metafile, MF3DUns16 data);
MF3DErr MF3D_Uns32Read (MF3D_FilePtr metafile, MF3DUns32 * data);
MF3DErr MF3D_Uns32Write (MF3D_FilePtr metafile, MF3DUns32 data);
MF3DErr MF3D_Uns64Read (MF3D_FilePtr metafile, MF3DUns64 * data);
MF3DErr MF3D_Uns64Write (MF3D_FilePtr metafile, MF3DUns64 data);
MF3DErr MF3D_Int8Read (MF3D_FilePtr metafile, MF3DInt8 * data);
MF3DErr MF3D_Int8Write (MF3D_FilePtr metafile, MF3DInt8 data);
MF3DErr MF3D_Int16Read (MF3D_FilePtr metafile, MF3DInt16 * data);
MF3DErr MF3D_Int16Write (MF3D_FilePtr metafile, MF3DInt16 data);
MF3DErr MF3D_Int32Read (MF3D_FilePtr metafile, MF3DInt32 * data);
MF3DErr MF3D_Int32Write (MF3D_FilePtr metafile, MF3DInt32 data);
MF3DErr MF3D_Int64Read (MF3D_FilePtr metafile, MFINT64 * data);
MF3DErr MF3D_Int64Write (MF3D_FilePtr metafile, MFINT64 data);
/* floats */
MF3DErr MF3D_Float32Read (MF3D_FilePtr metafile, MF3DFloat32 * data);
MF3DErr MF3D_Float32Write (MF3D_FilePtr metafile, MF3DFloat32 data);
MF3DErr MF3D_Float64Read (MF3D_FilePtr metafile, MF3DFloat64 * data);
MF3DErr MF3D_Float64Write (MF3D_FilePtr metafile, MF3DFloat64 data);
/* strings */
MF3DErr MF3D_CStringRead (MF3D_FilePtr metafile, MF3DCStringPtr * data);
MF3DErr MF3D_CStringWrite (MF3D_FilePtr metafile, MF3DCStringPtr data);
/* raw data */
MF3DErr MF3D_RawDataRead (MF3D_FilePtr metafile, MF3DSize size,
			  MF3DRawDataPtr data);
MF3DErr MF3D_RawDataWrite (MF3D_FilePtr metafile, MF3DSize size,
			   MF3DRawDataPtr data);
/* enums */
MF3DErr MF3D_EnumRead (MF3D_FilePtr metafile, MF3D_EnumTablePtr labelID,
		       MF3DEnumData * data);
MF3DErr MF3D_EnumWrite (MF3D_FilePtr metafile, MF3D_EnumTablePtr labelID,
			MF3DEnumData data);
/* flags */
MF3DErr MF3D_FlagsRead (MF3D_FilePtr metafile, MF3D_FlagTablePtr labelID,
			MF3DFlagData * data);
MF3DErr MF3D_FlagsWrite (MF3D_FilePtr metafile, MF3D_FlagTablePtr labelID,
			 MF3DFlagData data);
/* object ids */
MF3DErr MF3D_ObjIDRead (MF3D_FilePtr metafile, MF3DObjType * data);
MF3DErr MF3D_ObjWrite (MF3D_FilePtr metafile, MF3DObjType data);
/* begin/end (size) */
MF3DErr MF3D_BeginRead (MF3D_FilePtr metafile, MF3D_ObjStuffPtr * objStuff,
			MF3DReferenceID * refID, MF3DObjType * objType);
MF3DErr MF3D_EndRead (MF3D_FilePtr metafile);
MF3DErr MF3D_BeginWrite (MF3D_FilePtr metafile, MF3DVoidObjPtr object,
			 MF3D_ObjStuffPtr * objStuff);
MF3DErr MF3D_EndWrite (MF3D_FilePtr metafile, MF3DVoidObjPtr object);
/* object types */
MF3DErr MF3D_ObjTypeRead (MF3D_FilePtr metafile, MF3D_ObjStuffPtr * objStuff,
			  MF3DObjType * objType);
MF3DErr MF3D_ObjTypeWrite (MF3D_FilePtr metafile, MF3DVoidObjPtr object,
			   MF3D_ObjStuffPtr * objStuff);
/* references */
MF3DErr MF3D_RefRead (MF3D_FilePtr metafile, MF3DPositionReference * data);
MF3DErr MF3D_RefWrite (MF3D_FilePtr metafile, MF3D_TOCPosition data);
MF3DErr MF3D_TOCRefWrite (MF3D_FilePtr metafile, MF3DPositionReferencePtr data);
MF3DErr MF3D_LabelWrite (MF3D_FilePtr metafile, MF3DReferenceInfoPtr data,
			 MF3DBinaryFilePosition location, MF3DObjType type);

/* booleans */
MF3DErr MF3D_BooleanRead (MF3D_FilePtr metafile, MF3DBoolean * data);
MF3DErr MF3D_BooleanWrite (MF3D_FilePtr metafile, MF3DBoolean data);

/* preprocess */
MF3DErr MF3D_PreprocessFile (MF3D_FilePtr metafile);

/* get reference name */
MF3DCStringPtr MF3D_GetRefName (MF3D_FilePtr metafile, MF3DReferenceID refID);

/* postprocess */
MF3DErr MF3D_PostprocessFile (MF3D_FilePtr metafile);

/*==============================================================================
 *	Basic 3-D Types
 *==============================================================================
 */
MF3DErr MF3D_Point2DRead (MF3D_FilePtr metafile, MF3DPoint2D * point);
MF3DErr MF3D_Point2DWrite (MF3D_FilePtr metafile, MF3DPoint2D point);
MF3DErr MF3D_Point3DRead (MF3D_FilePtr metafile, MF3DPoint3D * point);
MF3DErr MF3D_Point3DWrite (MF3D_FilePtr metafile, MF3DPoint3D point);
MF3DErr MF3D_RationalPoint3DRead (MF3D_FilePtr metafile,
				  MF3DRationalPoint3D * point);
MF3DErr MF3D_RationalPoint3DWrite (MF3D_FilePtr metafile,
				   MF3DRationalPoint3D point);
MF3DErr MF3D_RationalPoint4DRead (MF3D_FilePtr metafile,
				  MF3DRationalPoint4D * point);
MF3DErr MF3D_RationalPoint4DWrite (MF3D_FilePtr metafile,
				   MF3DRationalPoint4D point);
MF3DErr MF3D_Vector3DRead (MF3D_FilePtr metafile, MF3DVector3D * vector);
MF3DErr MF3D_Vector3DWrite (MF3D_FilePtr metafile, MF3DVector3D vector);
MF3DErr MF3D_QuaternionRead (MF3D_FilePtr metafile, MF3DQuaternion * quaternion);
MF3DErr MF3D_QuaternionWrite (MF3D_FilePtr metafile,
			      MF3DQuaternion quaternion);
MF3DErr MF3D_Matrix3x3Read (MF3D_FilePtr metafile, MF3DMatrix3x3 matrix);
MF3DErr MF3D_Matrix3x3Write (MF3D_FilePtr metafile, MF3DConstMatrix3x3 matrix);
MF3DErr MF3D_Matrix4x4Read (MF3D_FilePtr metafile, MF3DMatrix4x4 matrix);
MF3DErr MF3D_Matrix4x4Write (MF3D_FilePtr metafile, MF3DConstMatrix4x4 matrix);
MF3DErr MF3D_RGBColorRead (MF3D_FilePtr metafile, MF3DRGBColor * color);
MF3DErr MF3D_RGBColorWrite (MF3D_FilePtr metafile, MF3DRGBColor color);
MF3DErr MF3D_Param2DRead (MF3D_FilePtr metafile, MF3DParam2D * param);
MF3DErr MF3D_Param2DWrite (MF3D_FilePtr metafile, MF3DParam2D param);

/*==============================================================================
 *	Comments
 *==============================================================================
 */
MF3DErr MF3D_CommentStrWrite (MF3D_FilePtr metafile, const char *data);
MF3DErr MF3D_CommentNumWrite (MF3D_FilePtr metafile, MF3DInt32 data);

/*==============================================================================
 *	Other Routines
 *==============================================================================
 */
MF3DErr MF3D_PositionReferenceDisposer (MF3DConstPositionReferencePtr inPosRef);
MF3DErr MF3D_VariableUnsRead (MF3D_FilePtr inMetafilePtr, MF3DUns32 inMaxValue,
			      MF3DUns32Ptr outValue);
MF3DErr MF3D_VariableUnsWrite (MF3D_FilePtr inMetafilePtr, MF3DUns32 inMaxValue,
			       MF3DUns32 inValue);

#endif
