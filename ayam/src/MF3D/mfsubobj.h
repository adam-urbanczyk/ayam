#ifndef MF3DSUBOBJECTS_H
#define	MF3DSUBOBJECTS_H
/*==============================================================================
 *
 *	File:		MFSUBOBJ.H
 *
 *	Function:	Structure definitions for Metafile subobjects.
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
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2R_RWW	No more complex objects.
 *		F27_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"

/*******************************************************************************
 *	Attributes
 *******************************************************************************
 */
/*==============================================================================
 *	AttributeSet
 *==============================================================================
 */
#define	kMF3DObjAttributeSet					MF3DObjectType('a','t','t','r')
#define	kMF3DObjAttributeSetText				"AttributeSet"

typedef struct MF3DAttributeSetObj
  {
    MF3DObjHeader
  }
MF3DAttributeSetObj, *MF3DAttributeSetObjPtr;

/*==============================================================================
 *	AmbientCoefficient
 *
 *	0 ó ambientCoefficient ó 1.0
 *==============================================================================
 */
#define	kMF3DObjAmbientCoefficient				MF3DObjectType('c','a','m','b')
#define	kMF3DObjAmbientCoefficientText			"AmbientCoefficient"

typedef struct MF3DAmbientCoefficientObj
  {
    MF3DObjHeader
    MF3DFloat32 ambientCoefficent;
  }
MF3DAmbientCoefficientObj, *MF3DAmbientCoefficientObjPtr;

/*==============================================================================
 *	DiffuseColor
 *==============================================================================
 */
#define	kMF3DObjDiffuseColor					MF3DObjectType('k','d','i','f')
#define	kMF3DObjDiffuseColorText				"DiffuseColor"

typedef struct MF3DDiffuseColorObj
  {
    MF3DObjHeader
    MF3DRGBColor diffuseColor;
  }
MF3DDiffuseColorObj, *MF3DDiffuseColorObjPtr;

/*==============================================================================
 *	HighlightState
 *==============================================================================
 */
#define	kMF3DObjHighlightState					MF3DObjectType('h','l','s','t')
#define	kMF3DObjHighlightStateText				"HighlightState"

#define	kDefaultHighlightState	kMF3DBooleanFalse

typedef struct MF3DHighlightStateObj
  {
    MF3DObjHeader
    MF3DBoolean highlighted;
  }
MF3DHighlightStateObj, *MF3DHighlightStateObjPtr;

/*==============================================================================
 *	Normal
 *
 *	|normal| = 1
 *==============================================================================
 */
#define	kMF3DObjNormal							MF3DObjectType('n','r','m','l')
#define	kMF3DObjNormalText						"Normal"

typedef struct MF3DNormalObj
  {
    MF3DObjHeader
    MF3DVector3D normal;
  }
MF3DNormalObj, *MF3DNormalObjPtr;

/*==============================================================================
 *	ShadingUV
 *
 *	Any UV parametrization is allowed,
 *	however, shading generally occurs with the following values. 
 *		0 <= shadingUV.u <= 1
 *		0 <= shadingUV.v <= 1
 *==============================================================================
 */
#define	kMF3DObjShadingUV						MF3DObjectType('s','h','u','v')
#define	kMF3DObjShadingUVText					"ShadingUV"

typedef struct MF3DShadingUVObj
  {
    MF3DObjHeader
    MF3DParam2D shadingUV;
  }
MF3DShadingUVObj, *MF3DShadingUVObjPtr;

/*==============================================================================
 *	SpecularColor
 *==============================================================================
 */
#define	kMF3DObjSpecularColor					MF3DObjectType('k','s','p','c')
#define	kMF3DObjSpecularColorText				"SpecularColor"

typedef struct MF3DSpecularColorObj
  {
    MF3DObjHeader
    MF3DRGBColor specularColor;
  }
MF3DSpecularColorObj, *MF3DSpecularColorObjPtr;

/*==============================================================================
 *	SpecularControl
 *
 *	0 <= specularControl
 *==============================================================================
 */
#define	kMF3DObjSpecularControl					MF3DObjectType('c','s','p','c')
#define	kMF3DObjSpecularControlText				"SpecularControl"

typedef struct MF3DSpecularControlObj
  {
    MF3DObjHeader
    MF3DFloat32 specularControl;
  }
MF3DSpecularControlObj, *MF3DSpecularControlObjPtr;

/*==============================================================================
 *	SurfaceTangent
 *==============================================================================
 */
#define	kMF3DObjSurfaceTangent					MF3DObjectType('s','r','t','n')
#define	kMF3DObjSurfaceTangentText				"SurfaceTangent"

typedef struct MF3DSurfaceTangentObj
  {
    MF3DObjHeader
    MF3DVector3D paramU;
    MF3DVector3D paramV;
  }
MF3DSurfaceTangentObj, *MF3DSurfaceTangentObjPtr;

/*==============================================================================
 *	SurfaceUV
 *
 *	Any UV parametrization is allowed,
 *	however, shading generally occurs with the following values. 
 *		0 <= shadingUV.u <= 1
 *		0 <= shadingUV.v <= 1
 *==============================================================================
 */
#define	kMF3DObjSurfaceUV						MF3DObjectType('s','r','u','v')
#define	kMF3DObjSurfaceUVText					"SurfaceUV"

typedef struct MF3DSurfaceUVObj
  {
    MF3DObjHeader
    MF3DParam2D surfaceUV;
  }
MF3DSurfaceUVObj, *MF3DSurfaceUVObjPtr;

/*==============================================================================
 *	SurfaceShader
 *==============================================================================
 */
#define	kDefaultSurfaceShader	NULL

/*==============================================================================
 *	TransparencyColor
 *==============================================================================
 */
#define	kMF3DObjTransparencyColor				MF3DObjectType('k','x','p','r')
#define	kMF3DObjTransparencyColorText			"TransparencyColor"

typedef struct MF3DTransparencyColorObj
  {
    MF3DObjHeader
    MF3DRGBColor transparency;
  }
MF3DTransparencyColorObj, *MF3DTransparencyColorObjPtr;

/*******************************************************************************
 *	Camera attributes:	CameraPlacement, CameraRange, CameraViewPort
 *
 *	For:
 *		OrthographicCamera
 *		ViewAngleAspectCamera
 *		ViewPlaneCamera
 *******************************************************************************
 */
/*==============================================================================
 *	CameraPlacement
 *
 *	upVector ^ (pointOfInterest - location)
 *
 *	|upVector| = 1.0
 *
 *	Default Values:
 *		0 0 1 # location
 *		0 0 0 # pointOfInterest
 *		0 1 0 # upVector
 *==============================================================================
 */
#define	kMF3DObjCameraPlacement					MF3DObjectType('c','m','p','l')
#define	kMF3DObjCameraPlacementText				"CameraPlacement"

typedef struct MF3DCameraPlacementObj
  {
    MF3DObjHeader
    MF3DPoint3D location;
    MF3DPoint3D pointOfInterest;
    MF3DVector3D upVector;
  }
MF3DCameraPlacementObj, *MF3DCameraPlacementObjPtr;

/*==============================================================================
 *	CameraRange
 *
 *	0 < hither <= yon
 *	default is:
 *		hither  epsilon
 *		yon     infinity
 *==============================================================================
 */
#define	kMF3DObjCameraRange						MF3DObjectType('c','m','r','g')
#define	kMF3DObjCameraRangeText					"CameraRange"

typedef struct MF3DCameraRangeObj
  {
    MF3DObjHeader
    MF3DFloat32 hither;
    MF3DFloat32 yon;
  }
MF3DCameraRangeObj, *MF3DCameraRangeObjPtr;

/*==============================================================================
 *	CameraViewPort
 *
 *	-1 <= origin.x <= 1
 *	-1 <= origin.y <= 1
 *	0 < width <= 2
 *	0 < height <= 2
 *	Default is:
 *		-1  1  # origin
 *		2      # width
 *		2      # height
 *==============================================================================
 */
#define	kMF3DObjCameraViewPort					MF3DObjectType('c','m','v','p')
#define	kMF3DObjCameraViewPortText				"CameraViewPort"

typedef struct MF3DCameraViewPortObj
  {
    MF3DObjHeader
    MF3DPoint2D origin;
    MF3DFloat32 width;
    MF3DFloat32 height;
  }
MF3DCameraViewPortObj, *MF3DCameraViewPortObjPtr;

/*==============================================================================
 *	FaceAttributeSetList
 *==============================================================================
 */
#define	kMF3DObjFaceAttributeSetList			MF3DObjectType('f','a','s','l')
#define	kMF3DObjFaceAttributeSetListText		"FaceAttributeSetList"

enum
  {
    MF3DPackingInclude = 0x00000000,
    MF3DPackingExclude = 0x00000001
  };
typedef MF3DEnumData MF3DPackingEnum;

typedef struct MF3DFaceAttributeSetListObj
  {
    MF3DObjHeader
    MF3DUns32 nObjects;
    MF3DPackingEnum packing;
    MF3DUns32 nIndices;
    MF3DUns32Ptr indices;	/* size: nIndices                       */
  }
MF3DFaceAttributeSetListObj, *MF3DFaceAttributeSetListObjPtr;

/*==============================================================================
 *	GeometryAttributeSetList
 *==============================================================================
 */
#define	kMF3DObjGeometryAttributeSetList		MF3DObjectType('g','a','s','l')
#define	kMF3DObjGeometryAttributeSetListText	"GeometryAttributeSetList"

typedef struct MF3DGeometryAttributeSetListObj
  {
    MF3DObjHeader
    MF3DUns32 nObjects;
    MF3DPackingEnum packing;
    MF3DUns32 nIndices;
    MF3DUns32Ptr indices;	/* size: nIndices                       */
  }
MF3DGeometryAttributeSetListObj, *MF3DGeometryAttributeSetListObjPtr;

/*==============================================================================
 *	VertexAttributeSetList
 *==============================================================================
 */
#define	kMF3DObjVertexAttributeSetList			MF3DObjectType('v','a','s','l')
#define	kMF3DObjVertexAttributeSetListText		"VertexAttributeSetList"

typedef struct MF3DVertexAttributeSetListObj
  {
    MF3DObjHeader
    MF3DUns32 nObjects;
    MF3DPackingEnum packing;
    MF3DUns32 nIndices;
    MF3DUns32Ptr indices;	/* size: nIndices                       */
  }
MF3DVertexAttributeSetListObj, *MF3DVertexAttributeSetListObjPtr;

/*==============================================================================
 *	BottomCapAttributeSet
 *==============================================================================
 */
#define	kMF3DObjBottomCapAttributeSet			MF3DObjectType('b','c','a','s')
#define	kMF3DObjBottomCapAttributeSetText		"BottomCapAttributeSet"

typedef struct MF3DBottomCapAttributeSetObj
  {
    MF3DObjHeader
  }
MF3DBottomCapAttributeSetObj, *MF3DBottomCapAttributeSetObjPtr;

/*==============================================================================
 *	Caps
 *==============================================================================
 */
#define	kMF3DObjCaps							MF3DObjectType('c','a','p','s')
#define	kMF3DObjCapsText						"Caps"

enum
  {
    kCapsNone = 0x00000000,
    kCapsBottom = 0x00000001,
    kCapsTop = 0x00000002,
    kCapsDefault = kCapsNone
  };
typedef MF3DFlagData MF3DCapsFlags;

typedef struct MF3DCapsObj
  {
    MF3DObjHeader
    MF3DCapsFlags caps;
  }
MF3DCapsObj, *MF3DCapsObjPtr;

/*==============================================================================
 *	FaceCapAttributeSet
 *==============================================================================
 */
#define	kMF3DObjFaceCapAttributeSet				MF3DObjectType('f','c','a','s')
#define	kMF3DObjFaceCapAttributeSetText			"FaceCapAttributeSet"

typedef struct MF3DFaceCapAttributeSetObj
  {
    MF3DObjHeader
  }
MF3DFaceCapAttributeSetObj, *MF3DFaceCapAttributeSetObjPtr;

/*==============================================================================
 *	TopCapAttributeSet
 *==============================================================================
 */
#define	kMF3DObjTopCapAttributeSet				MF3DObjectType('t','c','a','s')
#define	kMF3DObjTopCapAttributeSetText			"TopCapAttributeSet"

typedef struct MF3DTopCapAttributeSetObj
  {
    MF3DObjHeader
  }
MF3DTopCapAttributeSetObj, *MF3DTopCapAttributeSetObjPtr;

/*==============================================================================
 *	DisplayGroupState
 *==============================================================================
 */
#define	kMF3DObjDisplayGroupState				MF3DObjectType('d','g','s','t')
#define	kMF3DObjDisplayGroupStateText			"DisplayGroupState"

enum
  {
    MF3DDisplayGroupNone = 0x00000000,
    MF3DDisplayGroupInline = 0x00000001,
    MF3DDisplayGroupDoNotDraw = 0x00000002,
    MF3DDisplayGroupNoBoundingBox = 0x00000004,
    MF3DDisplayGroupNoBoundingSphere = 0x00000008,
    MF3DDisplayGroupDoNotPick = 0x00000010,
    MF3DDisplayGroupDefaultFlags = MF3DDisplayGroupNone
  };
typedef MF3DFlagData MF3DDisplayGroupStateFlags;

typedef struct MF3DDisplayGroupStateObj
  {
    MF3DObjHeader
    MF3DDisplayGroupStateFlags traversalFlags;
  }
MF3DDisplayGroupStateObj, *MF3DDisplayGroupStateObjPtr;

/*==============================================================================
 *	MeshCorners
 *==============================================================================
 */
#define	kMF3DObjMeshCorners						MF3DObjectType('c','r','n','r')
#define	kMF3DObjMeshCornersText					"MeshCorners"

typedef struct MF3DMeshCorner
  {
    MF3DUns32 vertexIndex;
    MF3DUns32 nFaces;		/* 0 < nFaces                           */
    MF3DUns32Ptr faces;		/* size: nFaces                         */
  }
MF3DMeshCorner, *MF3DMeshCornerPtr;

typedef struct MF3DMeshCornersObj
  {
    MF3DObjHeader
    MF3DUns32 nCorners;		/* 0 < nCorners                         */
    MF3DMeshCornerPtr corners;	/* size: nCorners                       */
  }
MF3DMeshCornersObj, *MF3DMeshCornersObjPtr;

/*==============================================================================
 *	MeshEdges
 *==============================================================================
 */
#define	kMF3DObjMeshEdges						MF3DObjectType('e','d','g','e')
#define	kMF3DObjMeshEdgesText					"MeshEdges"

typedef struct MF3DMeshEdge
  {
    MF3DUns32 vertexIndex1;
    MF3DUns32 vertexIndex2;
  }
MF3DMeshEdge, *MF3DMeshEdgePtr;

typedef struct MF3DMeshEdgesObj
  {
    MF3DObjHeader
    MF3DUns32 nEdges;		/* 0 < nEdges                           */
    MF3DMeshEdgePtr edges;	/* size: nEdges                         */
  }
MF3DMeshEdgesObj, *MF3DMeshEdgesObjPtr;

/*==============================================================================
 *	ShaderData
 *==============================================================================
 */
#define	kMF3DObjShaderData						MF3DObjectType('s','h','d','r')
#define	kMF3DObjShaderDataText					"ShaderData"

enum
  {
    MF3DShaderUVBoundaryWrap = 0x00000000,
    MF3DShaderUVBoundaryClamp = 0x00000001,
    MF3DShaderUVBoundaryDefault = MF3DShaderUVBoundaryWrap
  };
typedef MF3DEnumData MF3DShaderUVBoundaryEnum;

typedef struct MF3DShaderDataObj
  {
    MF3DObjHeader
    MF3DShaderUVBoundaryEnum uBounds;
    MF3DShaderUVBoundaryEnum vBounds;
  }
MF3DShaderDataObj, *MF3DShaderDataObjPtr;

/*==============================================================================
 *	ShaderTransform
 *==============================================================================
 */
#define	kMF3DObjShaderTransform					MF3DObjectType('s','d','x','f')
#define	kMF3DObjShaderTransformText				"ShaderTransform"

typedef struct MF3DShaderTransformObj
  {
    MF3DObjHeader
    MF3DMatrix4x4 shaderTransform;
  }
MF3DShaderTransformObj, *MF3DShaderTransformObjPtr;

/*==============================================================================
 *	ShaderUVTransform
 *==============================================================================
 */
#define	kMF3DObjShaderUVTransform				MF3DObjectType('s','d','u','v')
#define	kMF3DObjShaderUVTransformText			"ShaderUVTransform"

typedef struct MF3DShaderUVTransformObj
  {
    MF3DObjHeader
    MF3DMatrix3x3 matrix;
  }
MF3DShaderUVTransformObj, *MF3DShaderUVTransformObjPtr;

/*==============================================================================
 *	TrimCurves
 *
 *	Followed by:
 *		MF3DVoidObjPtr				twoDCurve;				{ 1-many }
 *					twoDCurve might be a MF3DNurbCurve2DObjPtr, for example
 *==============================================================================
 */
#define	kMF3DObjTrimCurves						MF3DObjectType('t','r','m','c')
#define	kMF3DObjTrimCurvesText					"TrimCurves"

typedef struct MF3DTrimCurvesObj
  {
    MF3DObjHeader
  }
MF3DTrimCurvesObj, *MF3DTrimCurvesObjPtr;

#endif
