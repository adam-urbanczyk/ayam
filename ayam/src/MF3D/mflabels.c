/*==============================================================================
 *
 *	File:		MFLABELS.C
 *
 *	Function:	Enum/Flags lookup routines
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
 *		F3K_RWW	GeneralPolygonHint.
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2K_RWW	File created.
 *==============================================================================
 */

#include "mflabels.h"

#include "mferrors.h"
#include "mftypes.h"
#include "mftextut.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	Enum Tables
 *==============================================================================
 */
MF3D_EnumTable gMF3D_BooleanEnum[] =
{0x00000000, "False",
 0x00000001, "True",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_ShapeHintEnum[] =
{0x00000000, "Convex",
 0x00000001, "Concave",
 0x00000002, "Complex",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_FallOffEnum[] =
{0x00000000, "None",
 0x00000001, "Linear",
 0x00000002, "Exponential",
 0x00000003, "Cosine",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_PixelTypeEnum[] =
{0x00000000, "RGB8",
 0x00000001, "RGB16",
 0x00000002, "RGB24",
 0x00000003, "RGB32",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_EndianEnum[] =
{0x00000000, "BigEndian",
 0x00000001, "LittleEndian",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_BackfacingEnum[] =
{0x00000000, "Both",
 0x00000001, "Culled",
 0x00000002, "Flipped",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_FillStyleEnum[] =
{0x00000000, "Filled",
 0x00000001, "Edges",
 0x00000002, "Points",
 0x00000003, "Empty",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_InterpolationStyleEnum[] =
{0x00000000, "None",
 0x00000001, "Vertex",
 0x00000002, "Pixel",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_OrientationStyleEnum[] =
{0x00000000, "CounterClockwise",
 0x00000001, "Clockwise",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_SubdivisionMethodEnum[] =
{0x00000000, "Constant",
 0x00000001, "WorldSpace",
 0x00000002, "ScreenSpace",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_AxisEnum[] =
{0x00000000, "X",
 0x00000001, "Y",
 0x00000002, "Z",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_ShaderUVBoundaryEnum[] =
{0x00000000, "Wrap",
 0x00000001, "Clamp",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_PackingEnum[] =
{0x00000000, "Include",
 0x00000001, "Exclude",
 kMF3D_EndTable
};

MF3D_EnumTable gMF3D_GeneralPolygonHintEnum[] =
{0x00000000, "Complex",
 0x00000001, "Concave",
 0x00000002, "Convex",
 kMF3D_EndTable
};

/*==============================================================================
 *	Flag Tables
 *==============================================================================
 */
MF3D_FlagTable gMF3D_MetafileFlags[] =
{0x00000000, "Normal",
 0x00000001, "Stream",
 0x00000002, "Database",
 kMF3D_EndTable
};

MF3D_FlagTable gMF3D_DisplayGroupStateFlags[] =
{0x00000000, "None",
 0x00000001, "Inline",
 0x00000002, "DoNotDraw",
 0x00000004, "NoBoundingBox",
 0x00000008, "NoBoundingSphere",
 0x00000010, "DoNotPick",
 kMF3D_EndTable
};

MF3D_FlagTable gMF3D_PickPartsFlags[] =
{0x00000000, "Object",
 0x00000001, "Face",
 0x00000002, "Edge",
 0x00000004, "Vertex",
 kMF3D_EndTable
};

MF3D_FlagTable gMF3D_CapsFlags[] =
{0x00000000, "None",
 0x00000001, "Bottom",
 0x00000002, "Top",
 kMF3D_EndTable
};

/*==============================================================================
 *	MF3D_MatchLabelByValue
 *
 *	Try to match inValueToMatch in the given label table.
 *	Return the matching string in outLabelText (unless outLabelText is NULL).
 *		Remember to free the string later!
 *==============================================================================
 */
MF3DErr
MF3D_MatchLabelByValue (
			 MF3D_LabelTablePtr inLabelTablePtr,
			 const MF3DUns32 inValueToMatch,
			 MF3DCStringPtr * outLabelText)
{
  MF3DErr result;

  /* Assume we will fail to match (rare pessimism) */
  result = kMF3DErrIllegalEnum;

  while (inLabelTablePtr->str != NULL)
    {
      if (inValueToMatch == inLabelTablePtr->value)
	{
	  result = kMF3DNoErr;	/* match! */
	  if (outLabelText != NULL)
	    {
	      MF3DCStringPtr str;
	      str = MF3D_DuplicateCString (inLabelTablePtr->str);
	      if (str == NULL)
		result = kMF3DErrOutOfMemory;
	      else
		*outLabelText = str;
	    }
	  break;		/* ### SUCCESSFUL LOOP EXIT ### */
	}
      ++inLabelTablePtr;
    }

  return result;
}

/*==============================================================================
 *	MF3D_MatchLabelByName
 *
 *	Try to match inLabelText in the given label table.
 *==============================================================================
 */
MF3DErr
MF3D_MatchLabelByName (
			MF3D_LabelTablePtr inLabelTablePtr,
			MF3DCStringPtr inLabelText,
			MF3DUns32 * outLabelValue)
{
  MF3D_LabelTablePtr labelPtr;
  MF3DErr result;

  /* Assume we will fail to match (rare pessimism) */
  result = kMF3DErrIllegalEnum;

  for (labelPtr = inLabelTablePtr; labelPtr->str != NULL; ++labelPtr)
    {
      if (MF3D_CmpStrInsensitive (inLabelText, labelPtr->str) == 0)
	{
	  result = kMF3DNoErr;	/* match! */
	  if (outLabelValue != NULL)
	    *outLabelValue = labelPtr->value;
	  break;		/* ### SUCCESSFUL LOOP EXIT ### */
	}
    }

  return result;
}
