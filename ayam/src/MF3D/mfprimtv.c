/*==============================================================================
 *
 *	File:		MFPRIMTV.C
 *
 *	Function:	Handle read/write of basic types.
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
 *		FB4_JRK	Parse object types we don't understand
 *		Fabio	Changed file name to 8 characters
 *		F3F_RWW Disable variable unsigned read/write.
 *		F3A_RWW	TOC stuff works.
 *		F2S_RWW	BeginGroup changes.
 *		F2R_RWW	Get rid of dumb curPos field in MF3D_FileRec
 *		F2E_RWW	File created.
 *==============================================================================
 */

#include "mfprimtv.h"

#include <stdio.h>		/* sprintf */

#include "mf3d.h"
#include "mferrors.h"
#include "mfint64.h"
#include "mfsystyp.h"
#include "mftypes.h"
#include "mfassert.h"
#include "mfbinrd.h"
#include "mfbinutl.h"
#include "mfintobj.h"
#include "mflabels.h"
#include "mfmacros.h"
#include "mfmemory.h"
#include "mfobjtyp.h"
#include "mftextrd.h"
#include "mftextst.h"
#include "mftextut.h"
#include "mftextwr.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	Methods structure
 *==============================================================================
 */
struct MF3D_IOPrimitiveMethods
  {				/* unsigned integers */
    MF3DErr (*readUns8) (MF3D_FilePtr metafile, MF3DUns8 * data);
    MF3DErr (*writeUns8) (MF3D_FilePtr metafile, MF3DUns8 data);
    MF3DErr (*readUns16) (MF3D_FilePtr metafile, MF3DUns16 * data);
    MF3DErr (*writeUns16) (MF3D_FilePtr metafile, MF3DUns16 data);
    MF3DErr (*readUns32) (MF3D_FilePtr metafile, MF3DUns32 * data);
    MF3DErr (*writeUns32) (MF3D_FilePtr metafile, MF3DUns32 data);
    MF3DErr (*readUns64) (MF3D_FilePtr metafile, MF3DUns64 * data);
    MF3DErr (*writeUns64) (MF3D_FilePtr metafile, MF3DUns64 data);

    /* signed integers */
    MF3DErr (*readInt8) (MF3D_FilePtr metafile, MF3DInt8 * data);
    MF3DErr (*writeInt8) (MF3D_FilePtr metafile, MF3DInt8 data);
    MF3DErr (*readInt16) (MF3D_FilePtr metafile, MF3DInt16 * data);
    MF3DErr (*writeInt16) (MF3D_FilePtr metafile, MF3DInt16 data);
    MF3DErr (*readInt32) (MF3D_FilePtr metafile, MF3DInt32 * data);
    MF3DErr (*writeInt32) (MF3D_FilePtr metafile, MF3DInt32 data);
    MF3DErr (*readInt64) (MF3D_FilePtr metafile, MFINT64 * data);
    MF3DErr (*writeInt64) (MF3D_FilePtr metafile, MFINT64 data);

    /* floats */
    MF3DErr (*readFloat32) (MF3D_FilePtr metafile, MF3DFloat32 * data);
    MF3DErr (*writeFloat32) (MF3D_FilePtr metafile, MF3DFloat32 data);
    MF3DErr (*readFloat64) (MF3D_FilePtr metafile, MF3DFloat64 * data);
    MF3DErr (*writeFloat64) (MF3D_FilePtr metafile, MF3DFloat64 data);

    /* strings */
    MF3DErr (*readCString) (MF3D_FilePtr metafile, MF3DCStringPtr * data);
    MF3DErr (*writeCString) (MF3D_FilePtr metafile, MF3DCStringPtr data);

    /* raw data */
    MF3DErr (*readRawData) (MF3D_FilePtr metafile, MF3DSize size,
			    MF3DRawDataPtr data);
    MF3DErr (*writeRawData) (MF3D_FilePtr metafile, MF3DSize size,
			     MF3DRawDataPtr data);

    /* enums */
    MF3DErr (*readEnum) (MF3D_FilePtr metafile,
			 MF3D_EnumTablePtr labelID,
			 MF3DEnumData * data);
    MF3DErr (*writeEnum) (MF3D_FilePtr metafile,
			  MF3D_EnumTablePtr labelID,
			  MF3DEnumData data);

    /* flags */
    MF3DErr (*readFlags) (MF3D_FilePtr metafile,
			  MF3D_FlagTablePtr labelID,
			  MF3DFlagData * data);
    MF3DErr (*writeFlags) (MF3D_FilePtr metafile,
			   MF3D_FlagTablePtr labelID,
			   MF3DFlagData data);

    /* begin/end object */
    MF3DErr (*readBegin) (MF3D_FilePtr metafile,
			  MF3D_ObjStuffPtr * objStuff,
			  MF3DReferenceID * refID,
			  MF3DObjType * objType);
    MF3DErr (*readEnd) (MF3D_FilePtr metafile);
    MF3DErr (*writeBegin) (MF3D_FilePtr metafile,
			   MF3DVoidObjPtr object,
			   MF3D_ObjStuffPtr * objStuff);
    MF3DErr (*writeEnd) (MF3D_FilePtr metafile);

    /* object ID */
    MF3DErr (*readObjType) (MF3D_FilePtr metafile,
			    MF3D_ObjStuffPtr * objStuff,
			    MF3DObjType * objType);
    MF3DErr (*writeObjType) (MF3D_FilePtr metafile, MF3DVoidObjPtr object,
			     MF3D_ObjStuffPtr * objStuff);

    /* Reference label */
    MF3DErr (*readRef) (MF3D_FilePtr metafile,
			MF3DPositionReference * data);
    MF3DErr (*writeRef) (MF3D_FilePtr metafile,
			 MF3D_TOCPosition data);
    MF3DErr (*writeTOCRef) (MF3D_FilePtr metafile,
			    MF3DPositionReferencePtr data);
    MF3DErr (*writeLabel) (MF3D_FilePtr metafile,
			   MF3DReferenceInfoPtr data,
			   MF3DBinaryFilePosition location,
			   MF3DObjType type);

    /* Preprocess */
    MF3DErr (*preprocess) (MF3D_FilePtr metafile);

    /* Get Reference Name */
    MF3DCStringPtr (*getRefName) (MF3D_FilePtr metafile, MF3DReferenceID refID);

    /* Postprocess */
    MF3DErr (*postprocess) (MF3D_FilePtr metafile);
  };

typedef struct MF3D_IOPrimitiveMethods MF3D_IOPrimitiveMethods;

/*==============================================================================
 *	Static prototypes
 *==============================================================================
 */
/* unsigned integers */
static MF3DErr MF3D_Uns8ReadB (MF3D_FilePtr metafile, MF3DUns8 * data);
static MF3DErr MF3D_Uns8ReadT (MF3D_FilePtr metafile, MF3DUns8 * data);

static MF3DErr MF3D_Uns8WriteB (MF3D_FilePtr metafile, MF3DUns8 data);
static MF3DErr MF3D_Uns8WriteT (MF3D_FilePtr metafile, MF3DUns8 data);

static MF3DErr MF3D_Uns16ReadB (MF3D_FilePtr metafile, MF3DUns16 * data);
static MF3DErr MF3D_Uns16ReadBS (MF3D_FilePtr metafile, MF3DUns16 * data);
static MF3DErr MF3D_Uns16ReadT (MF3D_FilePtr metafile, MF3DUns16 * data);

static MF3DErr MF3D_Uns16WriteB (MF3D_FilePtr metafile, MF3DUns16 data);
static MF3DErr MF3D_Uns16WriteBS (MF3D_FilePtr metafile, MF3DUns16 data);
static MF3DErr MF3D_Uns16WriteT (MF3D_FilePtr metafile, MF3DUns16 data);

static MF3DErr MF3D_Uns32ReadB (MF3D_FilePtr metafile, MF3DUns32 * data);
static MF3DErr MF3D_Uns32ReadBS (MF3D_FilePtr metafile, MF3DUns32 * data);
static MF3DErr MF3D_Uns32ReadT (MF3D_FilePtr metafile, MF3DUns32 * data);

static MF3DErr MF3D_Uns32WriteB (MF3D_FilePtr metafile, MF3DUns32 data);
static MF3DErr MF3D_Uns32WriteBS (MF3D_FilePtr metafile, MF3DUns32 data);
static MF3DErr MF3D_Uns32WriteT (MF3D_FilePtr metafile, MF3DUns32 data);

static MF3DErr MF3D_Uns64ReadB (MF3D_FilePtr metafile, MF3DUns64 * data);
static MF3DErr MF3D_Uns64ReadBS (MF3D_FilePtr metafile, MF3DUns64 * data);
static MF3DErr MF3D_Uns64ReadT (MF3D_FilePtr metafile, MF3DUns64 * data);

static MF3DErr MF3D_Uns64WriteB (MF3D_FilePtr metafile, MF3DUns64 data);
static MF3DErr MF3D_Uns64WriteBS (MF3D_FilePtr metafile, MF3DUns64 data);
static MF3DErr MF3D_Uns64WriteT (MF3D_FilePtr metafile, MF3DUns64 data);

static MF3DErr MF3D_Int8ReadB (MF3D_FilePtr metafile, MF3DInt8 * data);
static MF3DErr MF3D_Int8ReadT (MF3D_FilePtr metafile, MF3DInt8 * data);

static MF3DErr MF3D_Int8WriteB (MF3D_FilePtr metafile, MF3DInt8 data);
static MF3DErr MF3D_Int8WriteT (MF3D_FilePtr metafile, MF3DInt8 data);

static MF3DErr MF3D_Int16ReadB (MF3D_FilePtr metafile, MF3DInt16 * data);
static MF3DErr MF3D_Int16ReadBS (MF3D_FilePtr metafile, MF3DInt16 * data);
static MF3DErr MF3D_Int16ReadT (MF3D_FilePtr metafile, MF3DInt16 * data);

static MF3DErr MF3D_Int16WriteB (MF3D_FilePtr metafile, MF3DInt16 data);
static MF3DErr MF3D_Int16WriteBS (MF3D_FilePtr metafile, MF3DInt16 data);
static MF3DErr MF3D_Int16WriteT (MF3D_FilePtr metafile, MF3DInt16 data);

static MF3DErr MF3D_Int32ReadB (MF3D_FilePtr metafile, MF3DInt32 * data);
static MF3DErr MF3D_Int32ReadBS (MF3D_FilePtr metafile, MF3DInt32 * data);
static MF3DErr MF3D_Int32ReadT (MF3D_FilePtr metafile, MF3DInt32 * data);

static MF3DErr MF3D_Int32WriteB (MF3D_FilePtr metafile, MF3DInt32 data);
static MF3DErr MF3D_Int32WriteBS (MF3D_FilePtr metafile, MF3DInt32 data);
static MF3DErr MF3D_Int32WriteT (MF3D_FilePtr metafile, MF3DInt32 data);

static MF3DErr MF3D_Int64ReadB (MF3D_FilePtr metafile, MFINT64 * data);
static MF3DErr MF3D_Int64ReadBS (MF3D_FilePtr metafile, MFINT64 * data);
static MF3DErr MF3D_Int64ReadT (MF3D_FilePtr metafile, MFINT64 * data);

static MF3DErr MF3D_Int64WriteB (MF3D_FilePtr metafile, MFINT64 data);
static MF3DErr MF3D_Int64WriteBS (MF3D_FilePtr metafile, MFINT64 data);
static MF3DErr MF3D_Int64WriteT (MF3D_FilePtr metafile, MFINT64 data);

/* floats */
static MF3DErr MF3D_Float32ReadB (MF3D_FilePtr metafile, MF3DFloat32 * data);
static MF3DErr MF3D_Float32ReadBS (MF3D_FilePtr metafile, MF3DFloat32 * data);
static MF3DErr MF3D_Float32ReadT (MF3D_FilePtr metafile, MF3DFloat32 * data);

static MF3DErr MF3D_Float32WriteB (MF3D_FilePtr metafile, MF3DFloat32 data);
static MF3DErr MF3D_Float32WriteBS (MF3D_FilePtr metafile, MF3DFloat32 data);
static MF3DErr MF3D_Float32WriteT (MF3D_FilePtr metafile, MF3DFloat32 data);

static MF3DErr MF3D_Float64ReadB (MF3D_FilePtr metafile, MF3DFloat64 * data);
static MF3DErr MF3D_Float64ReadBS (MF3D_FilePtr metafile, MF3DFloat64 * data);
static MF3DErr MF3D_Float64ReadT (MF3D_FilePtr metafile, MF3DFloat64 * data);

static MF3DErr MF3D_Float64WriteB (MF3D_FilePtr metafile, MF3DFloat64 data);
static MF3DErr MF3D_Float64WriteBS (MF3D_FilePtr metafile, MF3DFloat64 data);
static MF3DErr MF3D_Float64WriteT (MF3D_FilePtr metafile, MF3DFloat64 data);

/* strings */
static MF3DErr MF3D_CStringReadB (MF3D_FilePtr metafile, MF3DCStringPtr * data);
static MF3DErr MF3D_CStringReadT (MF3D_FilePtr metafile, MF3DCStringPtr * data);

static MF3DErr MF3D_CStringWriteB (MF3D_FilePtr metafile, MF3DCStringPtr data);
static MF3DErr MF3D_CStringWriteT (MF3D_FilePtr metafile, MF3DCStringPtr data);

/* raw data */
static MF3DErr MF3D_RawDataReadB (MF3D_FilePtr metafile, MF3DSize size,
				  MF3DRawDataPtr data);
static MF3DErr MF3D_RawDataReadT (MF3D_FilePtr metafile, MF3DSize size,
				  MF3DRawDataPtr data);

static MF3DErr MF3D_RawDataWriteB (MF3D_FilePtr metafile, MF3DSize size,
				   MF3DRawDataPtr data);
static MF3DErr MF3D_RawDataWriteT (MF3D_FilePtr metafile, MF3DSize size,
				   MF3DRawDataPtr data);

/* enums */
static MF3DErr MF3D_EnumReadB (MF3D_FilePtr metafile,
			       MF3D_EnumTablePtr labelID,
			       MF3DEnumData * data);
static MF3DErr MF3D_EnumReadBS (MF3D_FilePtr metafile,
				MF3D_EnumTablePtr labelID,
				MF3DEnumData * data);
static MF3DErr MF3D_EnumReadT (MF3D_FilePtr metafile,
			       MF3D_EnumTablePtr labelID,
			       MF3DEnumData * data);

static MF3DErr MF3D_EnumWriteB (MF3D_FilePtr metafile,
				MF3D_EnumTablePtr labelID,
				MF3DEnumData data);
static MF3DErr MF3D_EnumWriteBS (MF3D_FilePtr metafile,
				 MF3D_EnumTablePtr labelID,
				 MF3DEnumData data);
static MF3DErr MF3D_EnumWriteT (MF3D_FilePtr metafile,
				MF3D_EnumTablePtr labelID,
				MF3DEnumData data);

/* flags */
static MF3DErr MF3D_FlagsReadB (MF3D_FilePtr metafile,
				MF3D_FlagTablePtr labelID,
				MF3DFlagData * data);
static MF3DErr MF3D_FlagsReadBS (MF3D_FilePtr metafile,
				 MF3D_FlagTablePtr labelID,
				 MF3DFlagData * data);
static MF3DErr MF3D_FlagsReadT (MF3D_FilePtr metafile,
				MF3D_FlagTablePtr labelID,
				MF3DFlagData * data);

static MF3DErr MF3D_FlagsWriteB (MF3D_FilePtr metafile,
				 MF3D_FlagTablePtr labelID,
				 MF3DFlagData data);
static MF3DErr MF3D_FlagsWriteBS (MF3D_FilePtr metafile,
				  MF3D_FlagTablePtr labelID,
				  MF3DFlagData data);
static MF3DErr MF3D_FlagsWriteT (MF3D_FilePtr metafile,
				 MF3D_FlagTablePtr labelID,
				 MF3DFlagData data);

/* begin/end object */
static MF3DErr MF3D_BeginReadB (MF3D_FilePtr metafile,
				MF3D_ObjStuffPtr * objStuff,
				MF3DReferenceID * refID,
				MF3DObjType * objType);
static MF3DErr MF3D_BeginReadT (MF3D_FilePtr metafile,
				MF3D_ObjStuffPtr * objStuff,
				MF3DReferenceID * refID,
				MF3DObjType * objType);

static MF3DErr MF3D_EndReadB (MF3D_FilePtr metafile);
static MF3DErr MF3D_EndReadBS (MF3D_FilePtr metafile);
static MF3DErr MF3D_EndReadT (MF3D_FilePtr metafile);

static MF3DErr MF3D_BeginWriteB (MF3D_FilePtr metafile,
				 MF3DVoidObjPtr object,
				 MF3D_ObjStuffPtr * objStuff);
static MF3DErr MF3D_BeginWriteT (MF3D_FilePtr metafile,
				 MF3DVoidObjPtr object,
				 MF3D_ObjStuffPtr * objStuff);

static MF3DErr MF3D_EndWriteB (MF3D_FilePtr metafile);
static MF3DErr MF3D_EndWriteT (MF3D_FilePtr metafile);

/* object ID */
static MF3DErr MF3D_ObjTypeReadB (MF3D_FilePtr metafile,
				  MF3D_ObjStuffPtr * objStuff,
				  MF3DObjType * objType);
static MF3DErr MF3D_ObjTypeReadBS (MF3D_FilePtr metafile,
				   MF3D_ObjStuffPtr * objStuff,
				   MF3DObjType * objType);
static MF3DErr MF3D_ObjTypeReadT (MF3D_FilePtr metafile,
				  MF3D_ObjStuffPtr * objStuff,
				  MF3DObjType * objType);

static MF3DErr MF3D_ObjTypeWriteB (MF3D_FilePtr metafile,
				   MF3DVoidObjPtr object,
				   MF3D_ObjStuffPtr * objStuff);
static MF3DErr MF3D_ObjTypeWriteT (MF3D_FilePtr metafile,
				   MF3DVoidObjPtr object,
				   MF3D_ObjStuffPtr * objStuff);

/* reference label */
static MF3DErr MF3D_RefReadB (MF3D_FilePtr metafile,
			      MF3DPositionReference * data);
static MF3DErr MF3D_RefReadT (MF3D_FilePtr metafile,
			      MF3DPositionReference * data);

static MF3DErr MF3D_RefWriteB (MF3D_FilePtr metafile,
			       MF3D_TOCPosition data);
static MF3DErr MF3D_RefWriteT (MF3D_FilePtr metafile,
			       MF3D_TOCPosition data);

static MF3DErr MF3D_TOCRefWriteB (MF3D_FilePtr metafile,
				  MF3DPositionReferencePtr data);
static MF3DErr MF3D_TOCRefWriteT (MF3D_FilePtr metafile,
				  MF3DPositionReferencePtr data);

static MF3DErr MF3D_LabelWriteB (MF3D_FilePtr metafile,
				 MF3DReferenceInfoPtr data,
				 MF3DBinaryFilePosition location,
				 MF3DObjType type);
static MF3DErr MF3D_LabelWriteT (MF3D_FilePtr metafile,
				 MF3DReferenceInfoPtr data,
				 MF3DBinaryFilePosition location,
				 MF3DObjType type);

/*==============================================================================
 *	Static Globals
 *==============================================================================
 */
static MF3D_IOPrimitiveMethods gMF3D_BinaryPrimitiveMethods =
{				/* integers */
  MF3D_Uns8ReadB,
  MF3D_Uns8WriteB,
  MF3D_Uns16ReadB,
  MF3D_Uns16WriteB,
  MF3D_Uns32ReadB,
  MF3D_Uns32WriteB,
  MF3D_Uns64ReadB,
  MF3D_Uns64WriteB,
  MF3D_Int8ReadB,
  MF3D_Int8WriteB,
  MF3D_Int16ReadB,
  MF3D_Int16WriteB,
  MF3D_Int32ReadB,
  MF3D_Int32WriteB,
  MF3D_Int64ReadB,
  MF3D_Int64WriteB,
	/* floats */
  MF3D_Float32ReadB,
  MF3D_Float32WriteB,
  MF3D_Float64ReadB,
  MF3D_Float64WriteB,
	/* strings */
  MF3D_CStringReadB,
  MF3D_CStringWriteB,
	/* raw data */
  MF3D_RawDataReadB,
  MF3D_RawDataWriteB,
	/* enums */
  MF3D_EnumReadB,
  MF3D_EnumWriteB,
	/* flags */
  MF3D_FlagsReadB,
  MF3D_FlagsWriteB,
	/* begin/end (size) */
  MF3D_BeginReadB,
  MF3D_EndReadB,
  MF3D_BeginWriteB,
  MF3D_EndWriteB,
	/* object id */
  MF3D_ObjTypeReadB,
  MF3D_ObjTypeWriteB,
	/* reference */
  MF3D_RefReadB,
  MF3D_RefWriteB,
  MF3D_TOCRefWriteB,
  MF3D_LabelWriteB,
	/* preprocess */
  MF3D_PreprocessBinaryFile,
	/* get reference name */
  MF3D_GetRefNameB,
	/* postprocess */
  MF3D_PostprocessBinaryFile
};

static MF3D_IOPrimitiveMethods gMF3D_BinarySwappedPrimitiveMethods =
{				/* integers */
  MF3D_Uns8ReadB,
  MF3D_Uns8WriteB,
  MF3D_Uns16ReadBS,
  MF3D_Uns16WriteBS,
  MF3D_Uns32ReadBS,
  MF3D_Uns32WriteBS,
  MF3D_Uns64ReadBS,
  MF3D_Uns64WriteBS,
  MF3D_Int8ReadB,
  MF3D_Int8WriteB,
  MF3D_Int16ReadBS,
  MF3D_Int16WriteBS,
  MF3D_Int32ReadBS,
  MF3D_Int32WriteBS,
  MF3D_Int64ReadBS,
  MF3D_Int64WriteBS,
	/* floats */
  MF3D_Float32ReadBS,
  MF3D_Float32WriteBS,
  MF3D_Float64ReadBS,
  MF3D_Float64WriteBS,
	/* strings */
  MF3D_CStringReadB,
  MF3D_CStringWriteB,
	/* raw data */
  MF3D_RawDataReadB,
  MF3D_RawDataWriteB,
	/* enums */
  MF3D_EnumReadBS,
  MF3D_EnumWriteBS,
	/* flags */
  MF3D_FlagsReadBS,
  MF3D_FlagsWriteBS,
	/* begin/end (size) */
  MF3D_BeginReadB,
  MF3D_EndReadB,
  MF3D_BeginWriteB,
  MF3D_EndWriteB,
	/* object id */
  MF3D_ObjTypeReadBS,
  MF3D_ObjTypeWriteB,
	/* reference */
  MF3D_RefReadB,
  MF3D_RefWriteB,
  MF3D_TOCRefWriteB,
  MF3D_LabelWriteB,
	/* preprocess */
  MF3D_PreprocessBinaryFile,
	/* get reference name */
  MF3D_GetRefNameB,
	/* postprocess */
  MF3D_PostprocessBinaryFile
};

static MF3D_IOPrimitiveMethods gMF3D_TextPrimitiveMethods =
{				/* integers */
  MF3D_Uns8ReadT,
  MF3D_Uns8WriteT,
  MF3D_Uns16ReadT,
  MF3D_Uns16WriteT,
  MF3D_Uns32ReadT,
  MF3D_Uns32WriteT,
  MF3D_Uns64ReadT,
  MF3D_Uns64WriteT,
  MF3D_Int8ReadT,
  MF3D_Int8WriteT,
  MF3D_Int16ReadT,
  MF3D_Int16WriteT,
  MF3D_Int32ReadT,
  MF3D_Int32WriteT,
  MF3D_Int64ReadT,
  MF3D_Int64WriteT,
	/* floats */
  MF3D_Float32ReadT,
  MF3D_Float32WriteT,
  MF3D_Float64ReadT,
  MF3D_Float64WriteT,
	/* strings */
  MF3D_CStringReadT,
  MF3D_CStringWriteT,
	/* raw data */
  MF3D_RawDataReadT,
  MF3D_RawDataWriteT,
	/* enums */
  MF3D_EnumReadT,
  MF3D_EnumWriteT,
	/* flags */
  MF3D_FlagsReadT,
  MF3D_FlagsWriteT,
	/* begin/end object */
  MF3D_BeginReadT,
  MF3D_EndReadT,
  MF3D_BeginWriteT,
  MF3D_EndWriteT,
	/* object id */
  MF3D_ObjTypeReadT,
  MF3D_ObjTypeWriteT,
	/* reference */
  MF3D_RefReadT,
  MF3D_RefWriteT,
  MF3D_TOCRefWriteT,
  MF3D_LabelWriteT,
	/* preprocess */
  MF3D_PreprocessTextFile,
	/* get reference name */
  MF3D_GetRefNameT,
	/* postprocess */
  MF3D_PostprocessTextFile
};

static MF3DUns32 zero = 0;

/*==============================================================================
 *	MF3D_GetPrimitivesAccessor
 *==============================================================================
 */
MF3DErr
MF3D_GetPrimitivesAccessor (
			     MF3DDataFormatEnum inDataFormat,
			     MF3D_IOPrimitivesTable * outPrimitives)
{
  switch (inDataFormat)
    {
    case kMF3DFormatBinary:
      *outPrimitives = &gMF3D_BinaryPrimitiveMethods;
      break;

    case kMF3DFormatSwappedBinary:
      *outPrimitives = &gMF3D_BinarySwappedPrimitiveMethods;
      break;

    case kMF3DFormatText:
      *outPrimitives = &gMF3D_TextPrimitiveMethods;
      break;

    default:
      return kMF3DErrIllegalDataType;
    }
  return kMF3DNoErr;
}

/*==============================================================================
 *	Dispatch Routines
 *	Integers
 *==============================================================================
 */
MF3DErr
MF3D_Uns8Read (
		MF3D_FilePtr metafile,
		MF3DUns8 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DUns8));
  return (*metafile->primitives->readUns8) (metafile, data);
}

MF3DErr
MF3D_Uns8Write (
		 MF3D_FilePtr metafile,
		 MF3DUns8 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DUns8));
  return (*metafile->primitives->writeUns8) (metafile, data);
}

MF3DErr
MF3D_Uns16Read (
		 MF3D_FilePtr metafile,
		 MF3DUns16 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DUns16));
  return (*metafile->primitives->readUns16) (metafile, data);
}

MF3DErr
MF3D_Uns16Write (
		  MF3D_FilePtr metafile,
		  MF3DUns16 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DUns16));
  return (*metafile->primitives->writeUns16) (metafile, data);
}

MF3DErr
MF3D_Uns32Read (
		 MF3D_FilePtr metafile,
		 MF3DUns32 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DUns32));
  return (*metafile->primitives->readUns32) (metafile, data);
}

MF3DErr
MF3D_Uns32Write (
		  MF3D_FilePtr metafile,
		  MF3DUns32 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DUns32));
  return (*metafile->primitives->writeUns32) (metafile, data);
}

MF3DErr
MF3D_Uns64Read (
		 MF3D_FilePtr metafile,
		 MF3DUns64 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DUns64));
  return (*metafile->primitives->readUns64) (metafile, data);
}

MF3DErr
MF3D_Uns64Write (
		  MF3D_FilePtr metafile,
		  MF3DUns64 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DUns64));
  return (*metafile->primitives->writeUns64) (metafile, data);
}

MF3DErr
MF3D_Int8Read (
		MF3D_FilePtr metafile,
		MF3DInt8 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DInt8));
  return (*metafile->primitives->readInt8) (metafile, data);
}

MF3DErr
MF3D_Int8Write (
		 MF3D_FilePtr metafile,
		 MF3DInt8 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DInt8));
  return (*metafile->primitives->writeInt8) (metafile, data);
}

MF3DErr
MF3D_Int16Read (
		 MF3D_FilePtr metafile,
		 MF3DInt16 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DInt16));
  return (*metafile->primitives->readInt16) (metafile, data);
}

MF3DErr
MF3D_Int16Write (
		  MF3D_FilePtr metafile,
		  MF3DInt16 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DInt16));
  return (*metafile->primitives->writeInt16) (metafile, data);
}

MF3DErr
MF3D_Int32Read (
		 MF3D_FilePtr metafile,
		 MF3DInt32 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DInt32));
  return (*metafile->primitives->readInt32) (metafile, data);
}

MF3DErr
MF3D_Int32Write (
		  MF3D_FilePtr metafile,
		  MF3DInt32 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DInt32));
  return (*metafile->primitives->writeInt32) (metafile, data);
}

MF3DErr
MF3D_Int64Read (
		 MF3D_FilePtr metafile,
		 MFINT64 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MFINT64));
  return (*metafile->primitives->readInt64) (metafile, data);
}

MF3DErr
MF3D_Int64Write (
		  MF3D_FilePtr metafile,
		  MFINT64 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MFINT64));
  return (*metafile->primitives->writeInt64) (metafile, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Floats
 *==============================================================================
 */
MF3DErr
MF3D_Float32Read (
		   MF3D_FilePtr metafile,
		   MF3DFloat32 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DFloat32));
  return (*metafile->primitives->readFloat32) (metafile, data);
}

MF3DErr
MF3D_Float32Write (
		    MF3D_FilePtr metafile,
		    MF3DFloat32 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DFloat32));
  return (*metafile->primitives->writeFloat32) (metafile, data);
}

MF3DErr
MF3D_Float64Read (
		   MF3D_FilePtr metafile,
		   MF3DFloat64 * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DFloat64));
  return (*metafile->primitives->readFloat64) (metafile, data);
}

MF3DErr
MF3D_Float64Write (
		    MF3D_FilePtr metafile,
		    MF3DFloat64 data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DFloat64));
  return (*metafile->primitives->writeFloat64) (metafile, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Strings
 *==============================================================================
 */
MF3DErr
MF3D_CStringRead (
		   MF3D_FilePtr metafile,
		   MF3DCStringPtr * data)
{
  /* Must do size validation later */
  return (*metafile->primitives->readCString) (metafile, data);
}

MF3DErr
MF3D_CStringWrite (
		    MF3D_FilePtr metafile,
		    MF3DCStringPtr data)
{
  /* Do size validation later */
  return (*metafile->primitives->writeCString) (metafile, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Booleans
 *==============================================================================
 */
MF3DErr
MF3D_BooleanRead (
		   MF3D_FilePtr metafile,
		   MF3DBoolean * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DBoolean));
  return (*metafile->primitives->readEnum) (metafile,
					    gMF3D_BooleanEnum, data);
}

MF3DErr
MF3D_BooleanWrite (
		    MF3D_FilePtr metafile,
		    MF3DBoolean data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DBoolean));
  return (*metafile->primitives->writeEnum) (metafile,
					     gMF3D_BooleanEnum, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Raw Data
 *==============================================================================
 */
MF3DErr
MF3D_RawDataRead (
		   MF3D_FilePtr metafile,
		   MF3DSize size,
		   MF3DRawDataPtr data)
{
  MF3D_ValidateReadSize (metafile, size);
  return (*metafile->primitives->readRawData) (metafile, size, data);
}

MF3DErr
MF3D_RawDataWrite (
		    MF3D_FilePtr metafile,
		    MF3DSize size,
		    MF3DRawDataPtr data)
{
  MF3D_ValidateWriteSize (metafile, size);
  return (*metafile->primitives->writeRawData) (metafile, size, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Enums
 *==============================================================================
 */
MF3DErr 
MF3D_EnumRead (
		MF3D_FilePtr metafile,
		MF3D_EnumTablePtr labelID,
		MF3DEnumData * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DEnumData));
  return (*metafile->primitives->readEnum) (metafile, labelID, data);
}

MF3DErr 
MF3D_EnumWrite (
		 MF3D_FilePtr metafile,
		 MF3D_EnumTablePtr labelID,
		 MF3DEnumData data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DEnumData));
  return (*metafile->primitives->writeEnum) (metafile, labelID, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Flags
 *==============================================================================
 */
MF3DErr 
MF3D_FlagsRead (
		 MF3D_FilePtr metafile,
		 MF3D_FlagTablePtr labelID,
		 MF3DFlagData * data)
{
  MF3D_ValidateReadSize (metafile, sizeof (MF3DFlagData));
  return (*metafile->primitives->readFlags) (metafile, labelID, data);
}

MF3DErr 
MF3D_FlagsWrite (
		  MF3D_FilePtr metafile,
		  MF3D_FlagTablePtr labelID,
		  MF3DFlagData data)
{
  MF3D_ValidateWriteSize (metafile, sizeof (MF3DFlagData));
  return (*metafile->primitives->writeFlags) (metafile, labelID, data);
}

/*==============================================================================
 *	Dispatch Routines
 *	Begin/End (Size)
 *==============================================================================
 */
MF3DErr
MF3D_BeginRead (
		 MF3D_FilePtr metafile,
		 MF3D_ObjStuffPtr * objStuff,
		 MF3DReferenceID * refID,
		 MF3DObjType * objType)
{
  /* Validate size later */
  return (*metafile->primitives->readBegin)
    (metafile, objStuff, refID, objType);
}

MF3DErr
MF3D_EndRead (
	       MF3D_FilePtr metafile)
{
  /* Validate size later */
  return (*metafile->primitives->readEnd) (metafile);
}

MF3DErr
MF3D_BeginWrite (
		  MF3D_FilePtr metafile,
		  MF3DVoidObjPtr object,
		  MF3D_ObjStuffPtr * objStuff)
{
  return (*metafile->primitives->writeBegin) (metafile, object, objStuff);
}

MF3DErr
MF3D_EndWrite (
		MF3D_FilePtr metafile,
		MF3DVoidObjPtr object)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (object->objectType != kMF3DObjContainer)
    result = (*metafile->primitives->writeEnd) (metafile);

  return result;
}

/*==============================================================================
 *	Dispatch Routines
 *	Object Type
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeRead (
		   MF3D_FilePtr metafile,
		   MF3D_ObjStuffPtr * objStuff,
		   MF3DObjType * objType)
{
  return (*metafile->primitives->readObjType) (metafile, objStuff, objType);
}

MF3DErr
MF3D_ObjTypeWrite (
		    MF3D_FilePtr metafile,
		    MF3DVoidObjPtr object,
		    MF3D_ObjStuffPtr * objStuff)
{
  return (*metafile->primitives->writeObjType) (metafile, object, objStuff);
}

/*==============================================================================
 *	Dispatch Routines
 *	Reference
 *==============================================================================
 */
MF3DErr
MF3D_RefRead (
	       MF3D_FilePtr metafile,
	       MF3DPositionReference * data)
{
  data->format = metafile->dataFormat;
  return (*metafile->primitives->readRef) (metafile, data);
}

MF3DErr
MF3D_RefWrite (
		MF3D_FilePtr metafile,
		MF3D_TOCPosition data)
{
  return (*metafile->primitives->writeRef) (metafile, data);
}

MF3DErr
MF3D_TOCRefWrite (
		   MF3D_FilePtr metafile,
		   MF3DPositionReferencePtr data)
{
  return (*metafile->primitives->writeTOCRef) (metafile, data);
}

MF3DErr
MF3D_LabelWrite (
		  MF3D_FilePtr metafile,
		  MF3DReferenceInfoPtr data,
		  MF3DBinaryFilePosition location,
		  MF3DObjType type)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (data->refID >= metafile->tocStuff.refSeed)
    {
      if (data->refID == kMF3DUnreferencedLabel)
	result = kMF3DErrIllegalRefID;
      else
	metafile->tocStuff.refSeed = data->refID + 1;
    }

  if (result == kMF3DNoErr)
    {
      ++metafile->tocStuff.numReferences;
      result = MF3D_Reallocate (&metafile->tocStuff.references,
				(metafile->tocStuff.numReferences *
				 sizeof (*metafile->tocStuff.references)));
    }

  if (result == kMF3DNoErr)
    {
      result = (*metafile->primitives->writeLabel)
	(metafile, data, location, type);
    }

  if (result == kMF3DNoErr && type == kMF3DObjContainer)
    metafile->tocStuff.needToUpdate = kMF3DBooleanTrue;

  return result;
}

/*==============================================================================
 *	Dispatch Routines
 *	Preprocess
 *==============================================================================
 */
MF3DErr
MF3D_PreprocessFile (
		      MF3D_FilePtr metafile)
{
  return (*metafile->primitives->preprocess) (metafile);
}

/*==============================================================================
 *	Dispatch Routines
 *	Get reference name
 *==============================================================================
 */
MF3DCStringPtr
MF3D_GetRefName (
		  MF3D_FilePtr metafile,
		  MF3DReferenceID refID)
{
  return (*metafile->primitives->getRefName) (metafile, refID);
}

/*==============================================================================
 *	Dispatch Routines
 *	Postprocess
 *==============================================================================
 */
MF3DErr
MF3D_PostprocessFile (
		       MF3D_FilePtr metafile)
{
  return (*metafile->primitives->postprocess) (metafile);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns8 Read
 *==============================================================================
 */
MF3DErr
MF3D_Uns8ReadB (
		 MF3D_FilePtr metafile,
		 MF3DUns8 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Uns8ReadT (
		 MF3D_FilePtr metafile,
		 MF3DUns8 * data)
{
  unsigned short foo;
  MF3DErr result;

  result = MF3D_ScanTextBuffer (metafile, "%hu", &foo);

  if (result == kMF3DNoErr)
    *data = (MF3DUns8) foo;

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns8 Write
 *==============================================================================
 */
MF3DErr
MF3D_Uns8WriteB (
		  MF3D_FilePtr metafile,
		  MF3DUns8 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns8WriteT (
		  MF3D_FilePtr metafile,
		  MF3DUns8 data)
{
  return MF3D_OutputText (metafile, "%hu ", (unsigned short) data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns16 Read
 *==============================================================================
 */
MF3DErr
MF3D_Uns16ReadB (
		  MF3D_FilePtr metafile,
		  MF3DUns16 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Uns16ReadBS (
		   MF3D_FilePtr metafile,
		   MF3DUns16 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Uns16ReadT (
		  MF3D_FilePtr metafile,
		  MF3DUns16 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%hu", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns16 Write
 *==============================================================================
 */
MF3DErr
MF3D_Uns16WriteB (
		   MF3D_FilePtr metafile,
		   MF3DUns16 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns16WriteBS (
		    MF3D_FilePtr metafile,
		    MF3DUns16 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns16WriteT (
		   MF3D_FilePtr metafile,
		   MF3DUns16 data)
{
  return MF3D_OutputText (metafile, "%hu ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns32 Read
 *==============================================================================
 */
MF3DErr
MF3D_Uns32ReadB (
		  MF3D_FilePtr metafile,
		  MF3DUns32 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Uns32ReadBS (
		   MF3D_FilePtr metafile,
		   MF3DUns32 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Uns32ReadT (
		  MF3D_FilePtr metafile,
		  MF3DUns32 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%lu", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns32 Write
 *==============================================================================
 */
MF3DErr
MF3D_Uns32WriteB (
		   MF3D_FilePtr metafile,
		   MF3DUns32 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns32WriteBS (
		    MF3D_FilePtr metafile,
		    MF3DUns32 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns32WriteT (
		   MF3D_FilePtr metafile,
		   MF3DUns32 data)
{
  return MF3D_OutputText (metafile, "%lu ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns64 Read
 *==============================================================================
 */
MF3DErr
MF3D_Uns64ReadB (
		  MF3D_FilePtr metafile,
		  MF3DUns64 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Uns64ReadBS (
		   MF3D_FilePtr metafile,
		   MF3DUns64 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Uns64ReadT (
		  MF3D_FilePtr metafile,
		  MF3DUns64 * data)
{
  /* ### NOT FULLY SUPPORTED IN 1.0 ### */
  data->hi = 0L;
  return MF3D_ScanTextBuffer (metafile, "%lu", &(data->lo));
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Uns64 Write
 *==============================================================================
 */
MF3DErr
MF3D_Uns64WriteB (
		   MF3D_FilePtr metafile,
		   MF3DUns64 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns64WriteBS (
		    MF3D_FilePtr metafile,
		    MF3DUns64 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Uns64WriteT (
		   MF3D_FilePtr metafile,
		   MF3DUns64 data)
{
  MFASSERT (data.hi == 0);

  /* ### NOT FULLY SUPPORTED IN 1.0 ### */
  if (data.hi != 0)
    {				/* ### Values > 2^32-1 cannot be written to text files. */
      return kMF3DErrUnsupportedUns64;
    }

  return MF3D_OutputText (metafile, "%lu ", data.lo);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int8 Read
 *==============================================================================
 */
MF3DErr
MF3D_Int8ReadB (
		 MF3D_FilePtr metafile,
		 MF3DInt8 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Int8ReadT (
		 MF3D_FilePtr metafile,
		 MF3DInt8 * data)
{
  short foo;
  MF3DErr result;

  result = MF3D_ScanTextBuffer (metafile, "%hd", &foo);

  if (result == kMF3DNoErr)
    *data = (MF3DInt8) foo;

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int8 Write
 *==============================================================================
 */
MF3DErr
MF3D_Int8WriteB (
		  MF3D_FilePtr metafile,
		  MF3DInt8 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int8WriteT (
		  MF3D_FilePtr metafile,
		  MF3DInt8 data)
{
  return MF3D_OutputText (metafile, "%hd ", (short) data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int16 Read
 *==============================================================================
 */
MF3DErr
MF3D_Int16ReadB (
		  MF3D_FilePtr metafile,
		  MF3DInt16 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Int16ReadBS (
		   MF3D_FilePtr metafile,
		   MF3DInt16 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Int16ReadT (
		  MF3D_FilePtr metafile,
		  MF3DInt16 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%hd", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int16 Write
 *==============================================================================
 */
MF3DErr
MF3D_Int16WriteB (
		   MF3D_FilePtr metafile,
		   MF3DInt16 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int16WriteBS (
		    MF3D_FilePtr metafile,
		    MF3DInt16 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int16WriteT (
		   MF3D_FilePtr metafile,
		   MF3DInt16 data)
{
  return MF3D_OutputText (metafile, "%hd ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int32 Read
 *==============================================================================
 */
MF3DErr
MF3D_Int32ReadB (
		  MF3D_FilePtr metafile,
		  MF3DInt32 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Int32ReadBS (
		   MF3D_FilePtr metafile,
		   MF3DInt32 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Int32ReadT (
		  MF3D_FilePtr metafile,
		  MF3DInt32 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%ld", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int32 Write
 *==============================================================================
 */
MF3DErr
MF3D_Int32WriteB (
		   MF3D_FilePtr metafile,
		   MF3DInt32 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int32WriteBS (
		    MF3D_FilePtr metafile,
		    MF3DInt32 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int32WriteT (
		   MF3D_FilePtr metafile,
		   MF3DInt32 data)
{
  return MF3D_OutputText (metafile, "%ld ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int64 Read
 *==============================================================================
 */
MF3DErr
MF3D_Int64ReadB (
		  MF3D_FilePtr metafile,
		  MFINT64 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Int64ReadBS (
		   MF3D_FilePtr metafile,
		   MFINT64 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Int64ReadT (
		  MF3D_FilePtr metafile,
		  MFINT64 * data)
{
  /* ### NOT FULLY SUPPORTED IN 1.0 ### */
  data->hi = 0L;
  return MF3D_ScanTextBuffer (metafile, "%ld", &(data->lo));
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Int64 Write
 *==============================================================================
 */
MF3DErr
MF3D_Int64WriteB (
		   MF3D_FilePtr metafile,
		   MFINT64 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int64WriteBS (
		    MF3D_FilePtr metafile,
		    MFINT64 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Int64WriteT (
		   MF3D_FilePtr metafile,
		   MFINT64 data)
{
  /* ### NOT FULLY SUPPORTED IN 1.0 ### */

  if ((data.lo < 0 && data.hi != -1) ||
      (data.lo >= 0 && data.hi != 0))
    {				/* ### Full 64-bit numbers cannot be written to text files. ### */
      return kMF3DErrUnsupportedInt64;
    }

  return MF3D_OutputText (metafile, "%ld ", data.lo);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Float32 Read
 *==============================================================================
 */
MF3DErr
MF3D_Float32ReadB (
		    MF3D_FilePtr metafile,
		    MF3DFloat32 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Float32ReadBS (
		     MF3D_FilePtr metafile,
		     MF3DFloat32 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Float32ReadT (
		    MF3D_FilePtr metafile,
		    MF3DFloat32 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%f", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Float32 Write
 *==============================================================================
 */
MF3DErr
MF3D_Float32WriteB (
		     MF3D_FilePtr metafile,
		     MF3DFloat32 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Float32WriteBS (
		      MF3D_FilePtr metafile,
		      MF3DFloat32 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Float32WriteT (
		     MF3D_FilePtr metafile,
		     MF3DFloat32 data)
{
  return MF3D_OutputText (metafile, "%.7hg ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Float64 Read
 *==============================================================================
 */
MF3DErr
MF3D_Float64ReadB (
		    MF3D_FilePtr metafile,
		    MF3DFloat64 * data)
{
  return MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);
}

MF3DErr
MF3D_Float64ReadBS (
		     MF3D_FilePtr metafile,
		     MF3DFloat64 * data)
{
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, sizeof (*data), (char *) data);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (*data), *data);

  return result;
}

MF3DErr
MF3D_Float64ReadT (
		    MF3D_FilePtr metafile,
		    MF3DFloat64 * data)
{
  return MF3D_ScanTextBuffer (metafile, "%lf", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Float64 Write
 *==============================================================================
 */
MF3DErr
MF3D_Float64WriteB (
		     MF3D_FilePtr metafile,
		     MF3DFloat64 data)
{
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Float64WriteBS (
		      MF3D_FilePtr metafile,
		      MF3DFloat64 data)
{
  MF3D_ByteSwap (sizeof (data), data);
  return MF3D_WriteProc (metafile, sizeof (data), &data);
}

MF3DErr
MF3D_Float64WriteT (
		     MF3D_FilePtr metafile,
		     MF3DFloat64 data)
{
  return MF3D_OutputText (metafile, "%.17lg ", data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	String Read
 *==============================================================================
 */
MF3DErr
MF3D_CStringReadB (
		    MF3D_FilePtr metafile,
		    MF3DCStringPtr * data)
{
  MF3D_BuildString tempStr;
  MF3DErr result;
  char c;

  MFASSERT ((tempStr.str = NULL) == NULL);

  result = MF3D_BuildString_New (&tempStr);

  while (result == kMF3DNoErr)
    {
      MF3D_ValidateReadSize (metafile, sizeof (c));
      result = MF3D_BinaryReadProc (metafile, sizeof (c), &c);
      if (result == kMF3DNoErr && c == '\0')
	{
	  result = MF3D_BuildString_EndString (&tempStr);
	  *data = tempStr.str;
	  break;		/* ### NORMAL EXIT IS HERE ### */
	}
      if (result == kMF3DNoErr)
	result = MF3D_BuildString_AddChar (&tempStr, c);
    }

  if (result != kMF3DNoErr)
    MFASSERT (tempStr.str == NULL);

  return result;
}

MF3DErr
MF3D_CStringReadT (
		    MF3D_FilePtr metafile,
		    MF3DCStringPtr * data)
{
  /* NOTE! We are not currently calling MF3D_ValidateReadSize */
  return MF3D_ReadTextString (metafile, data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	String Write
 *==============================================================================
 */
MF3DErr
MF3D_CStringWriteB (
		     MF3D_FilePtr metafile,
		     MF3DCStringPtr data)
{
  MF3DUns32 len;
  MF3DErr result;

  len = CStringLen (data);

  /* Write the string */
  MF3D_ValidateWriteSize (metafile, len);
  result = MF3D_WriteProc (metafile, len, data);

  /* Add end-of-string character and pad to 4-byte boundary */
  if (result == kMF3DNoErr)
    {
      MF3DUns32 padding;

/* F7K_PAM Nyet!                padding = 5 - ((len + 1) % 4); */
      padding = 4 - (len % 4);	/* F7K_PAM includes string terminator */
      MFASSERT (padding >= 1 && padding <= 4);

      MF3D_ValidateWriteSize (metafile, padding);
      result = MF3D_WriteProc (metafile, padding, &zero);
    }

  return result;
}

MF3DErr
MF3D_CStringWriteT (
		     MF3D_FilePtr metafile,
		     MF3DCStringPtr data)
{
  return MF3D_WriteTextString (metafile, data);
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Raw Data Read
 *==============================================================================
 */
MF3DErr
MF3D_RawDataReadB (
		    MF3D_FilePtr metafile,
		    MF3DSize size,
		    MF3DRawDataPtr data)
{
  char ignoredData[3];
  MF3DErr result;

  result = MF3D_BinaryReadProc (metafile, size, (char *) data);

  if (result == kMF3DNoErr)
    {
      if ((size % 4) != 0)
	result = MF3D_BinaryReadProc (metafile, 4 - (size % 4), ignoredData);
    }

  return result;
}

MF3DErr
MF3D_RawDataReadT (
		    MF3D_FilePtr metafile,
		    MF3DSize size,
		    MF3DRawDataPtr data)
{
  MF3DSize leftToRead;
  MF3DErr result;
  char *rawDataPtr;
  char *inputStr;
  char formatString[kMF3D_MaxDigitsInUns32 + 5 +
		    kMF3D_HexCharListLen + 1];

  leftToRead = size;
  result = kMF3DNoErr;
  rawDataPtr = (char *) data;

  /* two text chars represent one hex byte */
  inputStr = (char *) MF3D_Malloc (leftToRead * 2 +
				   kMF3D_RawDataHexPrefixLen + 1);
  if (inputStr == NULL)
    result = kMF3DErrOutOfMemory;

  while (leftToRead > 0 && result == kMF3DNoErr)
    {
      char numString[kMF3D_MaxDigitsInUns32 + 1];

      inputStr = (char *) MF3D_Realloc (inputStr, leftToRead * 2 +
					kMF3D_RawDataHexPrefixLen + 1);
      MFASSERT (inputStr != NULL);	/* should not happen when downsizing */

      /* Build a format string such that data will not be overflowed */
      strcpy (formatString, kMF3D_RawDataHexPrefix "%");
      MF3D_NumToString (leftToRead * 2, numString);
      strcat (formatString, numString);
      strcat (formatString, kMF3D_HexCharPattern);

      /* Copy into where we last left off */
      result = MF3D_ScanTextBuffer (metafile, formatString, inputStr);

      if (result == kMF3DNoErr)
	{
	  char c1, c2, *indexStr;

	  indexStr = inputStr;
	  while ((c1 = *indexStr++) != '\0' && (c2 = *indexStr++) != '\0')
	    {
	      *rawDataPtr++ = (MF3D_TextToHex (c1) << 4) | MF3D_TextToHex (c2);
	      --leftToRead;
	    }
	  if (c1 != '\0')
	    result = kMF3DErrRawDataOddNumberOfHexChars;
	}
    }

  MF3D_Free (inputStr);

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Raw Data Write
 *==============================================================================
 */
MF3DErr
MF3D_RawDataWriteB (
		     MF3D_FilePtr metafile,
		     MF3DSize size,
		     MF3DRawDataPtr data)
{
  MF3DErr result;

  result = MF3D_WriteProc (metafile, size, data);

  if (result == kMF3DNoErr && (size % 4) > 0)
    {
      result = MF3D_WriteProc (metafile, 4 - (size % 4), &zero);
    }

  return result;
}

MF3DErr
MF3D_RawDataWriteT (
		     MF3D_FilePtr metafile,
		     MF3DSize size,
		     MF3DRawDataPtr data)
{
  unsigned char *dataPtr;
  MF3DSize leftToDo;
  unsigned int bytesToDoOnThisLine;
  MF3DErr result = kMF3DNoErr;

  leftToDo = size;
  dataPtr = (unsigned char *) data;

  MF3D_WriteNewLine (metafile);

  while (result == kMF3DNoErr && leftToDo > 0)
    {
      bytesToDoOnThisLine = kMF3D_RawDataBytesPerLine;

      result = MF3D_OutputText (metafile, "%s", kMF3D_RawDataHexPrefix);

      while (result == kMF3DNoErr &&
	     leftToDo > 0 && bytesToDoOnThisLine-- > 0)
	{
	  result = MF3D_OutputText (metafile, "%.2X", *dataPtr++);
	  --leftToDo;
	}

      if (result == kMF3DNoErr)
	MF3D_WriteNewLine (metafile);
    }

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Enum Read
 *==============================================================================
 */
MF3DErr 
MF3D_EnumReadB (
		 MF3D_FilePtr metafile,
		 MF3D_EnumTablePtr labelID,
		 MF3DEnumData * data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32ReadB (metafile, data);
}

MF3DErr 
MF3D_EnumReadBS (
		  MF3D_FilePtr metafile,
		  MF3D_EnumTablePtr labelID,
		  MF3DEnumData * data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32ReadBS (metafile, data);
}

MF3DErr 
MF3D_EnumReadT (
		 MF3D_FilePtr metafile,
		 MF3D_EnumTablePtr labelID,
		 MF3DEnumData * data)
{
  MF3DUns32 num;
  MF3DErr result;
  char str[kMF3D_MaxLabelLength + 1];

  /* Check for number or text */
  result = MF3D_ScanTextBuffer (metafile, "%li", &num);

  if (result == kMF3DNoErr)
    {				/* got a number for some reason */
      result = MF3D_MatchLabelByValue (labelID, num, NULL);
    }
  else
    {
      result = MF3D_ScanTextBuffer (metafile, kMF3D_TextCharFormat, str);
      if (result == kMF3DNoErr)
	{
	  result = MF3D_MatchLabelByName (labelID, str, &num);
	}
    }

  if (result == kMF3DNoErr)
    *data = num;

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Enum Write
 *==============================================================================
 */
MF3DErr 
MF3D_EnumWriteB (
		  MF3D_FilePtr metafile,
		  MF3D_EnumTablePtr labelID,
		  MF3DEnumData data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32WriteB (metafile, data);
}

MF3DErr 
MF3D_EnumWriteBS (
		   MF3D_FilePtr metafile,
		   MF3D_EnumTablePtr labelID,
		   MF3DEnumData data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32WriteBS (metafile, data);
}

MF3DErr 
MF3D_EnumWriteT (
		  MF3D_FilePtr metafile,
		  MF3D_EnumTablePtr labelID,
		  MF3DEnumData data)
{
  MF3DErr result;
  MF3DCStringPtr labelStr;

  result = MF3D_MatchLabelByValue (labelID, data, &labelStr);

  if (result == kMF3DNoErr)
    {
      result = MF3D_OutputText (metafile, "%s ", labelStr);
      MF3D_Free (labelStr);
    }

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Flags Read
 *==============================================================================
 */
MF3DErr 
MF3D_FlagsReadB (
		  MF3D_FilePtr metafile,
		  MF3D_FlagTablePtr labelID,
		  MF3DFlagData * data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32ReadB (metafile, data);
}

MF3DErr 
MF3D_FlagsReadBS (
		   MF3D_FilePtr metafile,
		   MF3D_FlagTablePtr labelID,
		   MF3DFlagData * data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32ReadBS (metafile, data);
}

MF3DErr 
MF3D_FlagsReadT (
		  MF3D_FilePtr metafile,
		  MF3D_FlagTablePtr labelID,
		  MF3DFlagData * data)
{
  MF3DUns32 num;
  MF3DUns32 flagValue;
  MF3DErr result;
  char str[kMF3D_MaxLabelLength + 1];

  /* Check for number or text */
  result = MF3D_ScanTextBuffer (metafile, "%li", &num);

  if (result == kMF3DNoErr)
    {				/* got a number for some reason */
      MF3DUns32 flagShifter;

      flagShifter = num;
      flagValue = 1;
      while (result == kMF3DNoErr && flagShifter > 0)
	{			/* Check each set flag to see whether it exists */
	  if (flagShifter & 0x00000001)
	    {
	      result = MF3D_MatchLabelByValue (labelID, flagValue, NULL);
	    }
	  flagValue <<= 1;
	  flagShifter >>= 1;
	}
    }
  else
    {
      num = 0;
      do
	{
	  result = MF3D_ScanTextBuffer (metafile, kMF3D_TextCharFormat, str);
	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_MatchLabelByName (labelID, str, &flagValue);
	      num |= flagValue;
	    }
	}
      while (result == kMF3DNoErr && MF3D_ScanTextBuffer (metafile,
				    kMF3D_OrCharFormat, str) == kMF3DNoErr);
    }

  if (result == kMF3DNoErr)
    *data = num;

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Flags Write
 *==============================================================================
 */
MF3DErr 
MF3D_FlagsWriteB (
		   MF3D_FilePtr metafile,
		   MF3D_FlagTablePtr labelID,
		   MF3DFlagData data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32WriteB (metafile, data);
}

MF3DErr 
MF3D_FlagsWriteBS (
		    MF3D_FilePtr metafile,
		    MF3D_FlagTablePtr labelID,
		    MF3DFlagData data)
{
  MF3D_Unused (labelID);

  return MF3D_Uns32WriteBS (metafile, data);
}

MF3DErr 
MF3D_FlagsWriteT (
		   MF3D_FilePtr metafile,
		   MF3D_FlagTablePtr labelID,
		   MF3DFlagData data)
{
  MF3DErr result;
  MF3DCStringPtr labelStr;

  result = kMF3DNoErr;

  if (data == 0)
    {				/* All flags off */
      result = MF3D_MatchLabelByValue (labelID, data, &labelStr);

      if (result == kMF3DNoErr)
	{
	  result = MF3D_OutputText (metafile, "%s ", labelStr);
	  MF3D_Free (labelStr);
	}
    }
  else
    {				/* At least one flag on */
      MF3DUns32 flagShifter;
      MF3DUns32 flagValue;

      flagShifter = data;
      flagValue = 0x00000001;
      do
	{
	  if (flagShifter & 0x00000001)
	    {
	      char formatStr[5];

	      result = MF3D_MatchLabelByValue (labelID, flagValue, &labelStr);
	      if (result == kMF3DNoErr)
		{
		  strcpy (formatStr, "%s ");
		  /* If there is more to come, add the Or character */
		  if (flagShifter > 0x00000001)
		    {
		      strcat (formatStr, kMF3D_OrCharStr);
		      strcat (formatStr, " ");
		    }

		  result = MF3D_OutputText (metafile, formatStr, labelStr);
		  MF3D_Free (labelStr);
		}
	    }
	  flagShifter >>= 1;
	  flagValue <<= 1;
	}
      while (result == kMF3DNoErr && flagShifter > 0);
    }

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Begin Object
 *==============================================================================
 */
MF3DErr
MF3D_BeginReadB (
		  MF3D_FilePtr metafile,
		  MF3D_ObjStuffPtr * objStuff,
		  MF3DReferenceID * refID,
		  MF3DObjType * objType)
{
  MF3DBinaryFilePosition location;
  MF3DUns32 objectSize;
  char *readBuffer;
  MF3DErr result;

  /* Need to know where we are to see whether there is a label here. */
  result = MF3DTellPosition (metafile, &location);

  /* Read signature and size */
  if (result == kMF3DNoErr)
    {
      if (metafile->readBuffer.buf == NULL)
	{			/* Call readProc directly because buffer is not set up */
	  result = (*metafile->procsRec.readProc) (metafile->userFilePtr,
				    sizeof (MF3DObjType), (char *) objType);

	  if (result == kMF3DErrReadFailedEOF)
	    result = kMF3DNoMoreObjects;

	  if (result == kMF3DNoErr)
	    {
	      result = (*metafile->procsRec.readProc) (metafile->userFilePtr,
				  sizeof (MF3DUns32), (char *) &objectSize);
	    }
	}
      else
	{			/* Buffer already exists because we are in a container or group.
				 * Use normal read routines.
				 */
	  result = MF3D_BinaryReadProc (metafile, sizeof (MF3DObjType),
					(char *) objType);

	  if (result == kMF3DNoErr)
	    {
	      result = MF3D_BinaryReadProc (metafile, sizeof (MF3DUns32),
					    (char *) &objectSize);
	    }
	}
    }

  /* F7K_PAM swap object type and size that were just read in from file */
  if (result == kMF3DNoErr && metafile->dataFormat == kMF3DFormatSwappedBinary)
    {
      MF3D_ByteSwap4 (*objType);
      MF3D_ByteSwap4 (objectSize);
    }

  /* FB4_JRK */
  /* Allow unknown objects to be parsed by fixing error */
  if (result == kMF3DNoErr)
    {
      result = MF3D_FindObjectFromType (*objType, objStuff);
      if (result == kMF3DErrIllegalObjName)
	result = kMF3DNoErr;
    }

  /* Get refID if in TOC */
  if (result == kMF3DNoErr)
    result = MF3D_GetBinaryRefID (metafile, location, refID);

  if (result == kMF3DNoErr)
    {
      if (metafile->readBuffer.buf == NULL)
	{			/* Allocate the readBuffer */
	  readBuffer = (char *) MF3D_Malloc (objectSize);
	  if (readBuffer == NULL)
	    result = kMF3DErrOutOfMemory;

	  if (result == kMF3DNoErr)
	    {
	      result = (*metafile->procsRec.readProc)
		(metafile->userFilePtr, objectSize, readBuffer);
	    }

	  if (result == kMF3DNoErr)
	    {
	      MFASSERT (metafile->readBuffer.buf == NULL);
	      metafile->readBuffer.buf = readBuffer;
	      metafile->readBuffer.bufPos = 0;
	      metafile->readBuffer.bufSize = objectSize;
	      metafile->readBuffer.saveSize = NULL;
	    }
	}
      else
	{			/* Inside a container, readBuffer already allocated.
				 * So, we need to save the current buffer size and
				 * shrink apparent size of the buffer to size the new object.
				 */
	  MF3D_SaveBufferPtr sizeStack;

	  MF3D_Allocate (sizeStack);

	  if (result == kMF3DNoErr)
	    {			/* Save size of parent */
	      sizeStack->bufSize = metafile->readBuffer.bufSize;
	      sizeStack->next = metafile->readBuffer.saveSize;

	      /* Validate the new buffer size */
	      if ((metafile->readBuffer.bufPos + objectSize) >
		  sizeStack->bufSize)
		{
		  result = kMF3DErrContainedObjTooBig;
		}

	      /* Set new (temporary) buffer size */
	      if (result == kMF3DNoErr)
		{
		  metafile->readBuffer.bufSize = metafile->readBuffer.bufPos +
		    objectSize;
		  metafile->readBuffer.saveSize = sizeStack;
		}
	    }
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_BeginReadT
 *
 *	Begin reading a text object: read name and open paren
 *
 *	Return
 *		kMF3DNoErr			if an object is successfully read
 *		kMF3DNoMoreObjects	if a close parenthesis was read
 *		any other error		if an error occurs during parsing
 *==============================================================================
 */
MF3DErr
MF3D_BeginReadT (
		  MF3D_FilePtr metafile,
		  MF3D_ObjStuffPtr * objStuff,
		  MF3DReferenceID * refID,
		  MF3DObjType * objType)
{
  MF3DUns32 objIndex, numObjTableEntries;
  MF3D_ObjectTableEntryPtr objTableEntryPtr;
  MF3DBinaryFilePosition curPos;
  char *readBuffer;
  MF3DUns32 objectSize;
  MF3DErr result;

  /* Find out where we are */
  result = MF3DTellPosition (metafile, &curPos);

  /* Find curPos in the objTable */
  numObjTableEntries = metafile->objTable.numObjects;
  objTableEntryPtr = metafile->objTable.objects;
  for (objIndex = 0; objIndex < numObjTableEntries;
       ++objIndex, ++objTableEntryPtr)
    {				/* Find the first position >= curPos.
				 * That will be the beginning of the object.
				 */
      if (CompareInt64 (objTableEntryPtr->objLocation, curPos) >= 0)
	break;
    }
  if (objIndex >= numObjTableEntries)
    result = kMF3DNoMoreObjects;

  MFASSERT (result == kMF3DNoMoreObjects ||
	    CompareInt64 (objTableEntryPtr->objLocation, curPos) == 0);

  /* Create read buffer */
  if (result == kMF3DNoErr && metafile->readBuffer.buf == NULL)
    {				/* ###NOTE###
				 * Objects must be no bigger than an Uns32 for this subtract
				 * to work. (If we have that much data, our malloc is probably
				 * going to fail anyway.)
				 */
      objectSize = SubtractUns64 (objTableEntryPtr[1].objLocation, curPos);
      readBuffer = (char *) MF3D_Malloc (objectSize + 1);
      if (readBuffer == NULL)
	result = kMF3DErrOutOfMemory;

      if (result == kMF3DNoErr)
	{
	  result = (*metafile->procsRec.readProc)
	    (metafile->userFilePtr, objectSize, readBuffer);
	}

      /* Setup the read buffer (if we were not called from IntObjectRead) */
      if (result == kMF3DNoErr)
	{
	  metafile->readBuffer.buf = readBuffer;
	  metafile->readBuffer.buf[objectSize] = '\0';
	  metafile->readBuffer.bufPos = 0;
	  metafile->readBuffer.saveSize = NULL;
	  metafile->readBuffer.bufSize = objectSize;
	}

      /* If this object had a label, we need to skip past it */
      /* JRK */
      /* This used to be outside the check for buf == NULL, but we
       * had a problem with BeginGroup, because the object inside
       * looked as thought it had a label when it didn't. Now we
       * only look for labels in outer-most object.
       */
      if (result == kMF3DNoErr)
	{
	  *refID = objTableEntryPtr->objRefID;
	  if (*refID > 0)
	    {
	      result = MF3D_SkipText (metafile);
	    }
	}
    }
  else
    {
      /* If we're not the outside object, we don't have a ref */
      *refID = 0;
    }

  /* FB5_JRK
   * Read object name and translate to signature.
   * Allow unknown objects to be parsed by fixing error.
   */
  if (result == kMF3DNoErr)
    {
      result = MF3D_ReadObjectStuff (metafile, objStuff, objType);
      if (result == kMF3DErrIllegalObjName)
	result = kMF3DNoErr;
    }

  /* Read open paren */
  if (result == kMF3DNoErr)
    result = MF3D_ReadOpenParen (metafile);

  return result;
}

/*==============================================================================
 *	MF3D_BeginWriteB
 *
 *	Write object name and allot room for object size
 *==============================================================================
 */
MF3DErr
MF3D_BeginWriteB (
		   MF3D_FilePtr metafile,
		   MF3DVoidObjPtr object,
		   MF3D_ObjStuffPtr * objStuff)
{
  MF3D_WriteStackPtr wsPtr;
  MF3DErr result;

  /* Get objStuff and write the object type */
  result = MF3D_ObjTypeWrite (metafile, object, objStuff);

  if (result == kMF3DNoErr && object->objectType != kMF3DObjEndContainer)
    {				/* Allocate write stack link */
      MF3D_Allocate (wsPtr);

      /* Remember the location for the object size */
      if (result == kMF3DNoErr)
	result = MF3DTellPosition (metafile, &wsPtr->location);

      /* Write the phony object size */
      if (result == kMF3DNoErr)
	result = MF3D_Uns32Write (metafile, 0L);

      if (result == kMF3DNoErr)
	{
	  wsPtr->next = metafile->writeStack;
	  metafile->writeStack = wsPtr;
	}
      else
	MF3D_Free (wsPtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_BeginWriteT
 *
 *	Write the object name and opening parenthesis
 *==============================================================================
 */
MF3DErr
MF3D_BeginWriteT (
		   MF3D_FilePtr metafile,
		   MF3DVoidObjPtr object,
		   MF3D_ObjStuffPtr * objStuff)
{
  MF3DErr result;

  result = kMF3DNoErr;

  /* Write the object type */
  result = MF3D_ObjTypeWrite (metafile, object, objStuff);

  /* Write the opening paren */
  if (result == kMF3DNoErr && object->objectType != kMF3DObjEndContainer)
    {
      ++metafile->indent;
      result = MF3D_WriteProc (metafile, strlen (kMF3D_BeginCharStr " "),
			       kMF3D_BeginCharStr " ");
    }

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	End Object
 *==============================================================================
 */
MF3DErr
MF3D_EndReadB (
		MF3D_FilePtr metafile)
{
  MF3DUns32 bufPos;
  MF3DErr result;

  MFASSERT (metafile->readBuffer.buf != NULL);

  result = kMF3DNoErr;

  /* Binary file objects are supposed to be padded to four-byte boundaries */
  bufPos = metafile->readBuffer.bufPos;
  if ((metafile->readBuffer.bufSize & 0x00000003) == 0)
    bufPos = (bufPos + 3) & (~0x00000003);

  if (bufPos != metafile->readBuffer.bufSize)
    result = kMF3DErrDidntReadEntireObj;


  if (result == kMF3DNoErr)
    {
      if (metafile->readBuffer.saveSize != NULL)
	{			/* If saveSize exists, we are inside a container/group.
				 * We need to expand the buffer size to the parent object again.
				 */
	  MF3D_SaveBufferPtr sizeStack;

	  sizeStack = metafile->readBuffer.saveSize;
	  metafile->readBuffer.bufSize = sizeStack->bufSize;
	  metafile->readBuffer.saveSize = sizeStack->next;
	  MF3D_Free (sizeStack);
	}
      else
	{			/* If there was no saveSize, then we have reached the end
				 * of a top-level object, and we can free the readBuffer.
				 */
	  MF3D_Free (metafile->readBuffer.buf);
	  metafile->readBuffer.buf = NULL;
	}
    }

  return result;
}

MF3DErr
MF3D_EndReadT (
		MF3D_FilePtr metafile)
{
  char closeParen[2];
  MF3DErr result;

  result = MF3D_ScanTextBuffer (metafile, kMF3D_EndCharFormat, closeParen);

  if (result == kMF3DNoErr)
    {
      MF3D_SkipWhitespace (metafile);
      /* If we have reached the end of the object, free the buffer */
      if (metafile->readBuffer.buf[metafile->readBuffer.bufPos] == '\0')
	{
	  MFASSERT (metafile->readBuffer.bufPos ==
		    metafile->readBuffer.bufSize);
	  MF3D_Free (metafile->readBuffer.buf);
	  metafile->readBuffer.buf = NULL;
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_EndWriteB
 *
 *	Now that we know the object size, patch it back into the top of the
 *	object structure
 *==============================================================================
 */
MF3DErr
MF3D_EndWriteB (
		 MF3D_FilePtr metafile)
{
  MF3D_WriteStackPtr writeStackPtr;
  MF3DBinaryFilePosition curPos;
  MF3DSize size;
  MF3DSize padding;
  MF3DErr result;

  result = kMF3DNoErr;

  writeStackPtr = metafile->writeStack;

  if (writeStackPtr == NULL)
    result = kMF3DErrTooManyEndContainers;

  if (result == kMF3DNoErr)
    result = MF3DTellPosition (metafile, &curPos);

  if (result == kMF3DNoErr)
    result = MF3DSeekPosition (metafile, writeStackPtr->location);

  if (result == kMF3DNoErr)
    {
      size = SubtractUns64 (curPos, writeStackPtr->location);
      size -= sizeof (MF3DSize);

      /* Binary objects are padded to four-byte boundaries */
      padding = 3 - (size + 3) % 4;

      result = MF3D_Uns32Write (metafile, size + padding);
    }

  if (result == kMF3DNoErr)
    result = MF3DSeekPosition (metafile, curPos);

  if (result == kMF3DNoErr)
    {				/* Pad the object as required */
      if (padding > 0)
	{
	  MF3DUns32 zero;

	  zero = 0;
	  result = MF3D_WriteProc (metafile, padding, &zero);
	}
    }

  if (result == kMF3DNoErr)
    {
      metafile->writeStack = writeStackPtr->next;
      MF3D_Free (writeStackPtr);
    }

  return result;
}

/*==============================================================================
 *	MF3D_EndWriteT
 *
 *	Just write a closing parenthesis
 *==============================================================================
 */
MF3DErr
MF3D_EndWriteT (
		 MF3D_FilePtr metafile)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (metafile->indent > 0)
    --metafile->indent;
  else
    result = kMF3DErrTooManyEndContainers;

  if (result == kMF3DNoErr)
    {
      result = MF3D_WriteProc (metafile, strlen (kMF3D_EndCharStr " "),
			       kMF3D_EndCharStr " ");
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeReadB
 *
 *	Read the objectType
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeReadB (
		    MF3D_FilePtr metafile,
		    MF3D_ObjStuffPtr * objStuff,
		    MF3DObjType * objType)
{
  MF3DErr result;
  result = MF3D_BinaryReadProc (metafile, sizeof (MF3DObjType), (char *) objType);

  if (result == kMF3DNoErr && objStuff != NULL)
    result = MF3D_FindObjectFromType (*objType, objStuff);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeReadBS
 *
 *	Read the objectType and unswap
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeReadBS (
		     MF3D_FilePtr metafile,
		     MF3D_ObjStuffPtr * objStuff,
		     MF3DObjType * objType)
{
  MF3DErr result;
  result = MF3D_BinaryReadProc (metafile, sizeof (MF3DObjType), (char *) objType);

  if (result == kMF3DNoErr)
    MF3D_ByteSwap (sizeof (MF3DObjType), *objType);

  if (result == kMF3DNoErr)
    result = MF3D_FindObjectFromType (*objType, objStuff);

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeReadT
 *
 *	Read the object name and convert it to an objectType.
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeReadT (
		    MF3D_FilePtr metafile,
		    MF3D_ObjStuffPtr * objStuff,
		    MF3DObjType * objType)
{
  return MF3D_ReadObjectStuff (metafile, objStuff, objType);
}

/*==============================================================================
 *	MF3D_ObjTypeWriteB
 *
 *	Write the object type and return object stuff.
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeWriteB (
		     MF3D_FilePtr metafile,
		     MF3DVoidObjPtr object,
		     MF3D_ObjStuffPtr * objStuff)
{
  MF3DErr result;
  MF3DObjType objType;

  MFASSERT (sizeof (MF3DObjType) == sizeof (MF3DUns32));

  objType = object->objectType;
  result = MF3D_FindObjectFromType (objType, objStuff);

  if (result == kMF3DNoErr)
    {
      if (objType == kMF3DObjUnknownType)
	{
	  result = MF3D_Uns32Write (metafile,
			      ((MF3DUnknownObjPtr) object)->realObjectType);
	}
      else
	{			/* If we have a writeable type, spit it out */
	  if ((*objStuff)->name != NULL)
	    result = MF3D_Uns32Write (metafile, objType);
	}
    }

  return result;
}

/*==============================================================================
 *	MF3D_ObjTypeWriteT
 *
 *	Write the object name and return object stuff.
 *==============================================================================
 */
MF3DErr
MF3D_ObjTypeWriteT (
		     MF3D_FilePtr metafile,
		     MF3DVoidObjPtr object,
		     MF3D_ObjStuffPtr * objStuff)
{
  MF3DErr result;

  result = MF3D_FindObjectFromType (object->objectType, objStuff);

  if (result == kMF3DNoErr)
    {
      if (object->objectType == kMF3DObjUnknownType)
	{
	  result = MF3D_Int32Write (metafile,
			      ((MF3DUnknownObjPtr) object)->realObjectType);
	}
      else if ((*objStuff)->name != NULL)
	{
	  MF3D_ValidateWriteSize (metafile, strlen (data) + 1);
	  result = MF3D_OutputText (metafile, "%s ", (*objStuff)->name);
	}
    }

  return result;
}

/*==============================================================================
 *	I/O Primitive Routines
 *	Reference
 *==============================================================================
 */
MF3DErr
MF3D_RefReadB (
		MF3D_FilePtr metafile,
		MF3DPositionReference * data)
{
  MF3DErr result;

  result = MF3D_Uns64Read (metafile, &data->location.binary);

  return result;
}

MF3DErr
MF3D_RefReadT (
		MF3D_FilePtr metafile,
		MF3DPositionReference * data)
{
  return MF3D_ReadTextLabel (metafile, &data->location.text);
}

MF3DErr
MF3D_TOCRefWriteB (
		    MF3D_FilePtr metafile,
		    MF3DPositionReferencePtr data)
{
  MF3DUns64 zero;
  MF3DErr result;

  MF3D_Unused (data);

  /* Always write 0 and backpatch later */
  SetInt64ToZero (zero);

  /* If it is a binary file, we will want to update the location later */
  result = MF3DTellPosition (metafile, &metafile->tocLocation);

  if (result == kMF3DNoErr)
    result = MF3D_Uns64Write (metafile, zero);

  return result;
}

MF3DErr
MF3D_TOCRefWriteT (
		    MF3D_FilePtr metafile,
		    MF3DPositionReferencePtr data)
{
  MF3DErr result;

  /* Rare pessimism */
  result = kMF3DErrOutOfMemory;

  MFASSERT (metafile->tocStuff.tocLabelName == NULL);

  if (data == NULL)
    {				/* Write "nextTOC>" */
      MF3D_ValidateWriteSize (metafile, strlen (kMF3D_NoTOCPtrStr
					    kMF3D_ReferencePtrStr " ") + 2);
      metafile->tocStuff.tocLabelName =
	MF3D_DuplicateCString (kMF3D_NoTOCPtrStr);
    }
  else if (MF3DIsTextFormat (data->format))
    {
      MF3D_ValidateWriteSize (metafile, strlen (data) + 2);
      metafile->tocStuff.tocLabelName =
	MF3D_DuplicateCString (data->location.text);
    }
  else
    {				/* Write "toc>" */
      MF3D_ValidateWriteSize (metafile, strlen (kMF3D_DefaultTOCPtrStr
					    kMF3D_ReferencePtrStr " ") + 2);
      metafile->tocStuff.tocLabelName =
	MF3D_DuplicateCString (kMF3D_DefaultTOCPtrStr);
    }

  if (metafile->tocStuff.tocLabelName != NULL)
    {
      result = MF3D_OutputText (metafile, "%s%s ",
		    metafile->tocStuff.tocLabelName, kMF3D_ReferencePtrStr);
    }

  return result;
}

MF3DErr
MF3D_RefWriteB (
		 MF3D_FilePtr metafile,
		 MF3D_TOCPosition data)
{
  return MF3D_Uns64Write (metafile, data.location);
}

MF3DErr
MF3D_RefWriteT (
		 MF3D_FilePtr metafile,
		 MF3D_TOCPosition data)
{
  MFASSERT (data.name != NULL);

  MF3D_ValidateWriteSize (metafile, strlen (data) + 2);
  return MF3D_OutputText (metafile, "%s%s ", data.name, kMF3D_ReferencePtrStr);
}

MF3DErr
MF3D_LabelWriteB (
		   MF3D_FilePtr metafile,
		   MF3DReferenceInfoPtr data,
		   MF3DBinaryFilePosition location,
		   MF3DObjType type)
{
  MF3D_TOCReferencePtr refPtr;
  MF3DUns32 refCount;
  MF3DReferenceID refID;
  MF3DErr result;

  result = kMF3DNoErr;
  refPtr = metafile->tocStuff.references;
  /* Subtract 1 because numReferences was already incremented
   * by MF3D_LabelWrite
   */
  refCount = metafile->tocStuff.numReferences - 1;
  refID = data->refID;
  while (result == kMF3DNoErr && refCount > 0)
    {
      if (refPtr->refID == refID)
	result = kMF3DErrTwoObjectsWithSameRefID;
      ++refPtr;
      --refCount;
    }

  if (result == kMF3DNoErr)
    {
      refPtr->refID = refID;
      refPtr->ref.location = location;
      refPtr->type = type;
    }

  return result;
}

MF3DErr
MF3D_LabelWriteT (
		   MF3D_FilePtr metafile,
		   MF3DReferenceInfoPtr data,
		   MF3DBinaryFilePosition location,
		   MF3DObjType type)
{
  MF3D_TOCReferencePtr refPtr;
  MF3DUns32 refCount;
  MF3DReferenceID refID;
  MF3DCStringPtr refNameString;
  MF3DErr result;

  MF3D_Unused (location);
  MF3D_Unused (type);

  result = kMF3DNoErr;

  if (data->refName != NULL)
    {
      MF3D_ValidateWriteSize (metafile, strlen (kMF3D_LabelCharStr " ") +
			      strlen (data->refName));
      result = MF3D_OutputText (metafile, "%s%s ",
				data->refName, kMF3D_LabelCharStr);
      if (result == kMF3DNoErr)
	refNameString = MF3D_DuplicateCString (data->refName);
    }
  else
    {				/* If no label exists, make one up */
      MF3D_ValidateWriteSize (metafile, strlen (kMF3D_DefaultLabelStr "%ld"
						kMF3D_LabelCharStr " "));
      result = MF3D_OutputText (metafile, "%s%ld%s ", kMF3D_DefaultLabelStr,
				data->refID, kMF3D_LabelCharStr);
      if (result == kMF3DNoErr)
	{
	  refNameString = (MF3DCStringPtr) MF3D_Malloc (kMF3D_MaxDefaultLabelLen);
	  if (refNameString == NULL)
	    result = kMF3DErrOutOfMemory;
	}
      if (result == kMF3DNoErr)
	{
	  int numChars;

	  numChars = sprintf (refNameString, "%s%ld%s", kMF3D_DefaultLabelStr,
			      data->refID, kMF3D_LabelCharStr);
	  MFASSERT (numChars < kMF3D_MaxDefaultLabelLen);
	  refNameString = (MF3DCStringPtr) MF3D_Realloc (refNameString,
							 numChars + 1);
	}
    }

  if (result == kMF3DNoErr)
    {				/* Write a newline here */
      MF3D_WriteNewLine (metafile);

      refPtr = metafile->tocStuff.references;
      /* Subtract 1 because numReferences was already incremented
       * by MF3D_LabelWrite
       */
      refCount = metafile->tocStuff.numReferences - 1;
      refID = data->refID;
      while (result == kMF3DNoErr && refCount > 0)
	{
	  if (refPtr->refID == refID)
	    {
	      result = kMF3DErrTwoObjectsWithSameRefID;
	    }
	  else if (MF3D_CmpStrInsensitive (refPtr->ref.name, refNameString) == 0)
	    {
	      result = kMF3DErrTwoObjectsWithSameRefName;
	    }
	  ++refPtr;
	  --refCount;
	}

      if (result == kMF3DNoErr)
	{
	  refPtr->refID = refID;
	  refPtr->ref.name = refNameString;
	  refPtr->type = type;
	}
      else
	MF3D_Free (refNameString);
    }

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	2-D Point
 *==============================================================================
 */
MF3DErr
MF3D_Point2DRead (
		   MF3D_FilePtr metafile,
		   MF3DPoint2D * point)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &point->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->y);

  return result;
}

MF3DErr
MF3D_Point2DWrite (
		    MF3D_FilePtr metafile,
		    MF3DPoint2D point)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, point.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.y);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	3-D Point
 *==============================================================================
 */
MF3DErr
MF3D_Point3DRead (
		   MF3D_FilePtr metafile,
		   MF3DPoint3D * point)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &point->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->z);

  return result;
}

MF3DErr
MF3D_Point3DWrite (
		    MF3D_FilePtr metafile,
		    MF3DPoint3D point)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, point.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.z);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	Rational 3-D Point
 *==============================================================================
 */
MF3DErr
MF3D_RationalPoint3DRead (
			   MF3D_FilePtr metafile,
			   MF3DRationalPoint3D * point)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &point->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->w);

  return result;
}

MF3DErr
MF3D_RationalPoint3DWrite (
			    MF3D_FilePtr metafile,
			    MF3DRationalPoint3D point)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, point.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.w);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	Rational 4-D Point
 *==============================================================================
 */
MF3DErr
MF3D_RationalPoint4DRead (
			   MF3D_FilePtr metafile,
			   MF3DRationalPoint4D * point)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &point->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->z);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &point->w);

  return result;
}

MF3DErr
MF3D_RationalPoint4DWrite (
			    MF3D_FilePtr metafile,
			    MF3DRationalPoint4D point)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, point.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.z);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, point.w);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	3-D Vector
 *==============================================================================
 */
MF3DErr
MF3D_Vector3DRead (
		    MF3D_FilePtr metafile,
		    MF3DVector3D * vector)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &vector->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &vector->y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &vector->z);

  return result;
}

MF3DErr
MF3D_Vector3DWrite (
		     MF3D_FilePtr metafile,
		     MF3DVector3D vector)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, vector.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, vector.y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, vector.z);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	Quaternion
 *==============================================================================
 */
MF3DErr
MF3D_QuaternionRead (
		      MF3D_FilePtr metafile,
		      MF3DQuaternion * quaternion)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &quaternion->w);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &quaternion->x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &quaternion->y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &quaternion->z);

  return result;
}

MF3DErr
MF3D_QuaternionWrite (
		       MF3D_FilePtr metafile,
		       MF3DQuaternion quaternion)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, quaternion.w);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, quaternion.x);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, quaternion.y);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, quaternion.z);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	3x3 Matrix
 *==============================================================================
 */
MF3DErr
MF3D_Matrix3x3Read (
		     MF3D_FilePtr metafile,
		     MF3DMatrix3x3 matrix)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &matrix[0][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[0][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[0][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][2]);

  return result;
}

MF3DErr
MF3D_Matrix3x3Write (
		      MF3D_FilePtr metafile,
		      MF3DConstMatrix3x3 matrix)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, matrix[0][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[0][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[0][2]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][2]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][2]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	4x4 Matrix
 *==============================================================================
 */
MF3DErr
MF3D_Matrix4x4Read (
		     MF3D_FilePtr metafile,
		     MF3DMatrix4x4 matrix)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &matrix[0][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[0][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[0][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[0][3]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[1][3]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[2][3]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[3][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[3][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[3][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &matrix[3][3]);

  return result;
}

MF3DErr
MF3D_Matrix4x4Write (
		      MF3D_FilePtr metafile,
		      MF3DConstMatrix4x4 matrix)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, matrix[0][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[0][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[0][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[0][3]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[1][3]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[2][3]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[3][0]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[3][1]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[3][2]);
  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, matrix[3][3]);

  if (result == kMF3DNoErr)
    MF3D_WriteNewLine (metafile);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	RGB Color
 *==============================================================================
 */
MF3DErr
MF3D_RGBColorRead (
		    MF3D_FilePtr metafile,
		    MF3DRGBColor * color)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &color->red);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &color->green);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &color->blue);

  return result;
}

MF3DErr
MF3D_RGBColorWrite (
		     MF3D_FilePtr metafile,
		     MF3DRGBColor color)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, color.red);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, color.green);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, color.blue);

  return result;
}

/*==============================================================================
 *	Basic 3-D Types
 *	2-D Parametrization
 *==============================================================================
 */
MF3DErr
MF3D_Param2DRead (
		   MF3D_FilePtr metafile,
		   MF3DParam2D * param)
{
  MF3DErr result;

  result = MF3D_Float32Read (metafile, &param->u);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Read (metafile, &param->v);

  return result;
}

MF3DErr
MF3D_Param2DWrite (
		    MF3D_FilePtr metafile,
		    MF3DParam2D param)
{
  MF3DErr result;

  result = MF3D_Float32Write (metafile, param.u);

  if (result == kMF3DNoErr)
    result = MF3D_Float32Write (metafile, param.v);

  return result;
}

/*==============================================================================
 *	Comment Routines
 *==============================================================================
 */
MF3DErr
MF3D_CommentStrWrite (
		       MF3D_FilePtr metafile,
		       const char *data)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (MF3DIsTextFormat (metafile->dataFormat))
    {
      MF3D_ValidateWriteSize (metafile, strlen (data) + 2);
      result = MF3D_OutputText (metafile, "%s %s", kMF3D_CommentLineStr, data);

      if (result == kMF3DNoErr)
	result = MF3D_WriteNewLine (metafile);
    }

  return result;
}

MF3DErr
MF3D_CommentNumWrite (
		       MF3D_FilePtr metafile,
		       MF3DInt32 data)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (MF3DIsTextFormat (metafile->dataFormat))
    {
      MF3D_ValidateWriteSize (metafile, 2);
      result = MF3D_OutputText (metafile, kMF3D_CommentLineStr);

      if (result == kMF3DNoErr)
	result = MF3D_Int32Write (metafile, data);

      if (result == kMF3DNoErr)
	result = MF3D_WriteNewLine (metafile);
    }

  return result;
}

/*==============================================================================
 *	MF3D_PositionReferenceDisposer
 *
 *	Dispose a PositionReferencePtr
 *==============================================================================
 */
MF3DErr
MF3D_PositionReferenceDisposer (
				 MF3DConstPositionReferencePtr inPosRef)
{
  if (inPosRef != NULL && MF3DIsTextFormat (inPosRef->format) &&
      inPosRef->location.text != NULL)
    {
      MF3D_Free (inPosRef->location.text);
    }

  return kMF3DNoErr;
}

/*==============================================================================
 *	MF3D_VariableUnsRead
 *
 *	Read an unsigned number from a variable-sized array and store it into
 *	an Uns32.
 *==============================================================================
 */
MF3DErr
MF3D_VariableUnsRead (
		       MF3D_FilePtr inMetafilePtr,
		       MF3DUns32 inMaxValue,
		       MF3DUns32Ptr outValue)
{
  MF3DErr result;

#if kAllowVariableUns == TRUE
  if (inMaxValue >= 65536)
#else
  MF3D_Unused (inMaxValue);
#endif
  result = MF3D_Uns32Read (inMetafilePtr, outValue);
#if kAllowVariableUns == TRUE
  else
  if (inMaxValue >= 256)
    {
      MF3DUns16 tempValue;
      result = MF3D_Uns16Read (inMetafilePtr, &tempValue);
      if (result == kMF3DNoErr)
	*outValue = tempValue;
    }
  else
    {
      MF3DUns8 tempValue;
      result = MF3D_Uns8Read (inMetafilePtr, &tempValue);
      if (result == kMF3DNoErr)
	*outValue = tempValue;
    }
#endif

  return result;
}

/*==============================================================================
 *	MF3D_VariableUnsWrite
 *
 *	Write an unsigned number to a variable-sized array.
 *==============================================================================
 */
MF3DErr
MF3D_VariableUnsWrite (
			MF3D_FilePtr inMetafilePtr,
			MF3DUns32 inMaxValue,
			MF3DUns32 inValue)
{
  MF3DErr result;

  MFASSERT (inValue <= inMaxValue);

#if kAllowVariableUns == TRUE
  if (inMaxValue >= 65536)
#else
  MF3D_Unused (inMaxValue);
#endif
  result = MF3D_Uns32Write (inMetafilePtr, inValue);
#if kAllowVariableUns == TRUE
  else
  if (inMaxValue >= 256)
    result = MF3D_Uns16Write (inMetafilePtr, (MF3DUns16) inValue);
  else
    result = MF3D_Uns8Write (inMetafilePtr, (MF3DUns8) inValue);
#endif

  return result;
}
