#ifndef	MF3D_LABELS_H
#define	MF3D_LABELS_H
/*==============================================================================
 *
 *	File:		MFLABELS.H
 *
 *	Function:	Enum/Flags lookup tables
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
 *		F3K_RWW	GeneralPolygonHint.
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2E_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include <stddef.h>

#include "mfsystyp.h"
#include "mftypes.h"

/* Really only needs to be sufficiently large enough to read our text labels */
#define	kMF3D_MaxLabelLength	127

typedef struct MF3D_LabelTable
  {
    MF3DEnumData value;
    MF3DCStringPtr str;
  }
MF3D_LabelTable, *MF3D_LabelTablePtr;

typedef MF3D_LabelTable MF3D_EnumTable, *MF3D_EnumTablePtr;
typedef MF3D_LabelTable MF3D_FlagTable, *MF3D_FlagTablePtr;

#define	kMF3D_EndTable	0x00000000, NULL

/*==============================================================================
 *	Enum Tables
 *==============================================================================
 */
extern MF3D_EnumTable gMF3D_BooleanEnum[];
extern MF3D_EnumTable gMF3D_ShapeHintEnum[];
extern MF3D_EnumTable gMF3D_FallOffEnum[];
extern MF3D_EnumTable gMF3D_PixelTypeEnum[];
extern MF3D_EnumTable gMF3D_EndianEnum[];
extern MF3D_EnumTable gMF3D_BackfacingEnum[];
extern MF3D_EnumTable gMF3D_FillStyleEnum[];
extern MF3D_EnumTable gMF3D_InterpolationStyleEnum[];
extern MF3D_EnumTable gMF3D_OrientationStyleEnum[];
extern MF3D_EnumTable gMF3D_SubdivisionMethodEnum[];
extern MF3D_EnumTable gMF3D_AxisEnum[];
extern MF3D_EnumTable gMF3D_ShaderUVBoundaryEnum[];
extern MF3D_EnumTable gMF3D_PackingEnum[];
extern MF3D_EnumTable gMF3D_GeneralPolygonHintEnum[];

/*==============================================================================
 *	Flag Tables
 *==============================================================================
 */
extern MF3D_FlagTable gMF3D_MetafileFlags[];
extern MF3D_FlagTable gMF3D_DisplayGroupStateFlags[];
extern MF3D_FlagTable gMF3D_PickPartsFlags[];
extern MF3D_FlagTable gMF3D_CapsFlags[];

/*==============================================================================
 *	Label lookup routines
 *==============================================================================
 */
MF3DErr
MF3D_MatchLabelByValue (
			 MF3D_LabelTablePtr inLabelTablePtr,
			 const MF3DUns32 inValueToMatch,
			 MF3DCStringPtr * outLabelText);

MF3DErr
MF3D_MatchLabelByName (
			MF3D_LabelTablePtr inLabelTablePtr,
			MF3DCStringPtr inLabelText,
			MF3DUns32 * outLabelValue);

#endif
