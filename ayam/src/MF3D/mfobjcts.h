#ifndef MF3DOBJECTS_H
#define	MF3DOBJECTS_H
/*==============================================================================
 *
 *	File:		MFOBJCTS.H
 *
 *	Function:	Structure definitions for all Metafile objects returned
 *				by MF3DReadAnObject() or passed to MF3DWriteAnObject().
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
 *		FB3_JRK	Added UnknownBinary (ukbn)
 *		Fabio	Changed file name to 8 characters
 *		F3K_RWW	GeneralPolygonHint.
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2S_RWW	BeginGroup changes.
 *		F26_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mfsubobj.h"
#include "mfsystyp.h"
#include "mftypes.h"

/*==============================================================================
 *	Generic Object returned by MF3DReadAnObject
 *
 *	Use the objectType field to determine what kind of object this is.
 *	Then, cast the pointer appropriately.
 *==============================================================================
 */
#define	kMF3DObjInvalid							0

typedef struct MF3DVoidObj
  {
    MF3DObjHeader
  }
MF3DVoidObj, *MF3DVoidObjPtr;

/*==============================================================================
 *	Metafile Header
 *
 *	Always the first object returned by MF3DReadAnObject
 *	Always the first object passed to MF3DWriteAnObject
 *==============================================================================
 */
#define	kMF3DObjMetafile						MF3DObjectType('3','D','M','F')
#define	kMF3DObjMetafileSwapped					MF3DObjectType('F','M','D','3')
#define	kMF3DObjMetafileText					"3DMetafile"

#define	kMF3DMaximumRefSeed		0x7FFFFFFF
#define	kMF3DMinimumTypeSeed	0x80000000

enum
  {
    kMF3DFormatNormal = 0x00000000,
    kMF3DFormatStream = 0x00000001,
    kMF3DFormatDatabase = 0x00000002
  };
typedef MF3DFlagData MF3DMetafileFlags;		/* Metafile flags                               */

typedef struct MF3DMetafileObj
  {
    MF3DObjHeader
    MF3DDataFormatEnum dataFormat;	/* binary, swapped, or text    */
    MF3DUns16 majorVersion;	/* version of metafile format      */
    MF3DUns16 minorVersion;
    MF3DMetafileFlags flags;
    MF3DUns32 refSeed;		/* next available ref num              */
    MF3DInt32 typeSeed;		/* next available type num             */
    MF3DPositionReferencePtr tocLocation;	/* table of contents                       */
  }
MF3DMetafileObj, *MF3DMetafileObjPtr;

/*==============================================================================
 *	Reference
 *
 *	refID == 0 indicates an entire external file
 *	(must be followed by storageObject)
 *
 *	Optionally followed by:
 *		MF3DStorageObjPtr	storageObject;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjReference						MF3DObjectType('r','f','r','n')
#define	kMF3DObjReferenceText					"Reference"

#define	kMF3DReferenceEntireFile	0

/* Examples of an MF3DStorageObj are:
 * MF3DMacintoshPathObjPtr and MF3DUnixPathObjPtr
 */
typedef struct MF3DStorageObj
  {
    MF3DObjHeader
    MF3DCStringPtr pathName;
  }
MF3DStorageObj, *MF3DStorageObjPtr;

typedef struct MF3DReferenceObj
  {
    MF3DObjHeader
    MF3DReferenceID refID;	/* reference ID                                 */
  }
MF3DReferenceObj, *MF3DReferenceObjPtr;

/*==============================================================================
 *	Container
 *
 *	The next object will be a root object.
 *	All objects (until a matching EndContainer is returned) after the
 *		root object are subobjects (modifiers) of the root object.
 *==============================================================================
 */
#define	kMF3DObjContainer						MF3DObjectType('c','n','t','r')
#define	kMF3DObjContainerText					"Container"

typedef struct MF3DContainerObj
  {
    MF3DObjHeader
  }
MF3DContainerObj, *MF3DContainerObjPtr;

/*==============================================================================
 *	EndContainer
 *
 *	The next object will be a root object.
 *	All objects (until a matching EndContainer is returned) after the
 *		root object are subobjects (modifiers) of the root object.
 *==============================================================================
 */
/* ### NOTE: EndContainer is a phantom type (not in the QuickDraw 3D spec ### */
#define	kMF3DObjEndContainer					MF3DObjectType('e','n','d','c')

typedef struct MF3DEndContainerObj
  {
    MF3DObjHeader
  }
MF3DEndContainerObj, *MF3DEndContainerObjPtr;

/*==============================================================================
 *	BeginGroup
 *==============================================================================
 */
#define	kMF3DObjBeginGroup						MF3DObjectType('b','g','n','g')
#define	kMF3DObjBeginGroupText					"BeginGroup"

typedef MF3DVoidObjPtr MF3DAnyGroupObjPtr;

typedef struct MF3DBeginGroupObj
  {
    MF3DObjHeader
    MF3DAnyGroupObjPtr group;
    MF3DDisplayGroupStateObjPtr displayState;	/* (optional)                   */
  }
MF3DBeginGroupObj, *MF3DBeginGroupObjPtr;

/*==============================================================================
 *	Group
 *==============================================================================
 */
#define	kMF3DObjGroup							MF3DObjectType('g','r','o','p')
#define	kMF3DObjGroupText						"Group"

typedef struct MF3DGroupObj
  {
    MF3DObjHeader
  }
MF3DGroupObj, *MF3DGroupObjPtr;

/*==============================================================================
 *	DisplayGroup (contains drawable objects)
 *
 *	Optionally followed by:
 *		MF3DDisplayGroupStateObjPtr	displayGroupState;	{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjDisplayGroup					MF3DObjectType('d','s','p','g')
#define	kMF3DObjDisplayGroupText				"DisplayGroup"

typedef struct MF3DDisplayGroupObj
  {
    MF3DObjHeader
  }
MF3DDisplayGroupObj, *MF3DDisplayGroupObjPtr;

/*==============================================================================
 *	OrderedDisplayGroup
 *
 *	Optionally followed by:
 *		MF3DDisplayGroupStateObjPtr	displayGroupState;	{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjOrderedDisplayGroup				MF3DObjectType('o','r','d','g')
#define	kMF3DObjOrderedDisplayGroupText			"OrderedDisplayGroup"

typedef struct MF3DOrderedDisplayGroupObj
  {
    MF3DObjHeader
  }
MF3DOrderedDisplayGroupObj, *MF3DOrderedDisplayGroupObjPtr;

/*==============================================================================
 *	IOProxyDisplayGroup
 *
 *	Optionally followed by:
 *		MF3DDisplayGroupStateObjPtr	displayGroupState;	{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjIOProxyDisplayGroup				MF3DObjectType('i','o','p','x')
#define	kMF3DObjIOProxyDisplayGroupText			"IOProxyDisplayGroup"

typedef struct MF3DIOProxyDisplayGroupObj
  {
    MF3DObjHeader
  }
MF3DIOProxyDisplayGroupObj, *MF3DIOProxyDisplayGroupObjPtr;

/*==============================================================================
 *	InfoGroup
 *==============================================================================
 */
#define	kMF3DObjInfoGroup						MF3DObjectType('i','n','f','o')
#define	kMF3DObjInfoGroupText					"InfoGroup"

typedef struct MF3DInfoGroupObj
  {
    MF3DObjHeader
  }
MF3DInfoGroupObj, *MF3DInfoGroupObjPtr;

/*==============================================================================
 *	LightGroup
 *==============================================================================
 */
#define	kMF3DObjLightGroup						MF3DObjectType('l','g','h','g')
#define	kMF3DObjLightGroupText					"LightGroup"

typedef struct MF3DLightGroupObj
  {
    MF3DObjHeader
  }
MF3DLightGroupObj, *MF3DLightGroupObjPtr;

/*==============================================================================
 *	EndGroup
 *==============================================================================
 */
#define kMF3DObjEndGroup						MF3DObjectType('e','n','d','g')
#define	kMF3DObjEndGroupText					"EndGroup"

typedef struct MF3DEndGroupObj
  {
    MF3DObjHeader
  }
MF3DEndGroupObj, *MF3DEndGroupObjPtr;

/*==============================================================================
 *	Interactive Renderer
 *==============================================================================
 */
#define kMF3DObjRendererInteractive				MF3DObjectType('c','t','w','n')
#define	kMF3DObjRendererInteractiveText			"InteractiveRenderer"

typedef struct MF3DInteractiveRendererObj
  {
    MF3DObjHeader
  }
MF3DInteractiveRendererObj, *MF3DInteractiveRendererObjPtr;

/*==============================================================================
 *	GenericRenderer (does nothing)
 *==============================================================================
 */
#define	kMF3DObjRendererGeneric					MF3DObjectType('g','n','r','r')
#define	kMF3DObjRendererGenericText				"GenericRenderer"

typedef struct MF3DGenericRendererObj
  {
    MF3DObjHeader
  }
MF3DGenericRendererObj, *MF3DGenericRendererObjPtr;

/*==============================================================================
 *	WireFrame Renderer
 *==============================================================================
 */
#define	kMF3DObjRendererWireFrame				MF3DObjectType('w','r','f','r')
#define	kMF3DObjRendererWireFrameText			"WireFrame"

typedef struct MF3DWireFrameObj
  {
    MF3DObjHeader
  }
MF3DWireFrameObj, *MF3DWireFrameObjPtr;

/*==============================================================================
 *	ZBuffer Renderer
 *==============================================================================
 */
#define	kMF3DObjRendererZBuffer					MF3DObjectType('z','b','u','f')
#define	kMF3DObjRendererZBufferText				"ZBuffer"

typedef struct MF3DZBufferObj
  {
    MF3DObjHeader
  }
MF3DZBufferObj, *MF3DZBufferObjPtr;

/*==============================================================================
 *	OrthographicCamera
 *
 *	left < right
 *	bottom < top
 *
 *	Optionally followed by:
 *		MF3DCameraPlacementObjPtr	cameraPlacement;	{ 0-1 }
 *		MF3DCameraRangeObjPtr		cameraRange;		{ 0-1 }
 *		MF3DCameraViewPortObjPtr	cameraViewPort;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjOrthographicCamera				MF3DObjectType('o','r','t','h')
#define	kMF3DObjOrthographicCameraText			"OrthographicCamera"

typedef struct MF3DOrthographicCameraObj
  {
    MF3DObjHeader
    MF3DFloat32 left;
    MF3DFloat32 top;
    MF3DFloat32 right;
    MF3DFloat32 bottom;
  }
MF3DOrthographicCameraObj, *MF3DOrthographicCameraObjPtr;

/*==============================================================================
 *	ViewAngleAspectCamera
 *
 *	0 < fieldOfView <= pi
 *	0 < aspectRatioXtoY
  *
 *	Optionally followed by:
 *		MF3DCameraPlacementObjPtr	cameraPlacement;	{ 0-1 }
 *		MF3DCameraRangeObjPtr		cameraRange;		{ 0-1 }
 *		MF3DCameraViewPortObjPtr	cameraViewPort;		{ 0-1 }
*==============================================================================
 */
#define	kMF3DObjViewAngleAspectCamera			MF3DObjectType('v','a','n','a')
#define	kMF3DObjViewAngleAspectCameraText		"ViewAngleAspectCamera"

typedef struct MF3DViewAngleAspectCameraObj
  {
    MF3DObjHeader
    MF3DFloat32 fieldOfView;
    MF3DFloat32 aspectRatioXtoY;
  }
MF3DViewAngleAspectCameraObj, *MF3DViewAngleAspectCameraObjPtr;

/*==============================================================================
 *	ViewPlaneCamera
 *
 *	0 < viewPlane
 *	0 < halfWidthAtViewPlane
 *	0 < halfHeightAtViewPlane
 *	centerXOnViewPlane, centerYOnViewPlane may be any value
 *
 *	Optionally followed by:
 *		MF3DCameraPlacementObjPtr	cameraPlacement;	{ 0-1 }
 *		MF3DCameraRangeObjPtr		cameraRange;		{ 0-1 }
 *		MF3DCameraViewPortObjPtr	cameraViewPort;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjViewPlaneCamera					MF3DObjectType('v','w','p','l')
#define	kMF3DObjViewPlaneCameraText				"ViewPlaneCamera"

typedef struct MF3DViewPlaneCameraObj
  {
    MF3DObjHeader
    MF3DFloat32 viewPlane;
    MF3DFloat32 halfWidthAtViewPlane;
    MF3DFloat32 halfHeightAtViewPlane;
    MF3DFloat32 centerXOnViewPlane;
    MF3DFloat32 centerYOnViewPlane;
  }
MF3DViewPlaneCameraObj, *MF3DViewPlaneCameraObjPtr;

/*==============================================================================
 *	Box
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # orientation
 *		0 1 0 # majorAxis
 *		0 0 1 # minorAxis
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DFaceAttributeSetListObjPtr	faceAttributes;	{ 0-1, nFaces = 6 }
 *		MF3DAttributeSetListObjPtr		attributes;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjBox								MF3DObjectType('b','o','x',' ')
#define	kMF3DObjBoxText							"Box"

typedef struct MF3DBoxData
  {
    MF3DVector3D orientation;
    MF3DVector3D majorAxis;
    MF3DVector3D minorAxis;
    MF3DPoint3D origin;
  }
MF3DBoxData, *MF3DBoxDataPtr;

typedef struct MF3DBoxObj
  {
    MF3DObjHeader
    MF3DBoxDataPtr box;		/* (optional)                   */
  }
MF3DBoxObj, *MF3DBoxObjPtr;

/*==============================================================================
 *	Cone
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # orientation
 *		0 1 0 # majorAxis
 *		0 0 1 # minorAxis
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DCapsObjPtr						caps;					{ 0-1 }
 *		MF3DFaceCapAttributeSetObjPtr		faceCapAttributes;		{ 0-1 }
 *		MF3DBottomCapAttributeSetObjPtr		bottomCapAttributes;	{ 0-1 }
 *		MF3DAttributeSetListObjPtr			attributes;				{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjCone							MF3DObjectType('c','o','n','e')
#define	kMF3DObjConeText						"Cone"

typedef struct MF3DConeData
  {
    MF3DVector3D orientation;
    MF3DVector3D majorAxis;
    MF3DVector3D minorAxis;
    MF3DPoint3D origin;
  }
MF3DConeData, *MF3DConeDataPtr;

typedef struct MF3DConeObj
  {
    MF3DObjHeader
    MF3DConeDataPtr cone;	/* (optional)           */
  }
MF3DConeObj, *MF3DConeObjPtr;

/*==============================================================================
 *	Cylinder
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # orientation
 *		0 1 0 # majorAxis
 *		0 0 1 # minorAxis
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DCapsObjPtr						caps;					{ 0-1 }
 *		MF3DTopCapAttributeSetObjPtr		topCapAttributes;		{ 0-1 }
 *		MF3DFaceCapAttributeSetObjPtr		faceCapAttributes;		{ 0-1 }
 *		MF3DBottomCapAttributeSetObjPtr		bottomCapAttributes;	{ 0-1 }
 *		MF3DAttributeSetListObjPtr			attributes;				{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjCylinder						MF3DObjectType('c','y','l','n')
#define	kMF3DObjCylinderText					"Cylinder"

typedef struct MF3DCylinderData
  {
    MF3DVector3D orientation;
    MF3DVector3D majorAxis;
    MF3DVector3D minorAxis;
    MF3DPoint3D origin;
  }
MF3DCylinderData, *MF3DCylinderDataPtr;

typedef struct MF3DCylinderObj
  {
    MF3DObjHeader
    MF3DCylinderDataPtr cylinder;	/* (optional)           */
  }
MF3DCylinderObj, *MF3DCylinderObjPtr;

/*==============================================================================
 *	Disk
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # majorRadius
 *		0 1 0 # minorRadius
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjDisk							MF3DObjectType('d','i','s','k')
#define	kMF3DObjDiskText						"Disk"

typedef struct MF3DDiskData
  {
    MF3DObjHeader
    MF3DVector3D majorRadius;
    MF3DVector3D minorRadius;
    MF3DPoint3D origin;
  }
MF3DDiskData, *MF3DDiskDataPtr;

typedef struct MF3DDiskObj
  {
    MF3DObjHeader
    MF3DDiskDataPtr disk;	/* (optional)           */
  }
MF3DDiskObj, *MF3DDiskObjPtr;

/*==============================================================================
 *	Ellipse
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # majorAxis
 *		0 1 0 # minorAxis
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjEllipse							MF3DObjectType('e','l','p','s')
#define	kMF3DObjEllipseText						"Ellipse"

typedef struct MF3DEllipseData
  {
    MF3DVector3D majorAxis;
    MF3DVector3D minorAxis;
    MF3DPoint3D origin;
  }
MF3DEllipseData, *MF3DEllipseDataPtr;

typedef struct MF3DEllipseObj
  {
    MF3DObjHeader
    MF3DEllipseDataPtr ellipse;	/* (optional)           */
  }
MF3DEllipseObj, *MF3DEllipseObjPtr;

/*==============================================================================
 *	Ellipsoid
 *
 *	For 0-sized objects, default is:
 *		1 0 0 # orientation
 *		0 1 0 # majorAxis
 *		0 0 1 # minorAxis
 *		0 0 0 # origin
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjEllipsoid						MF3DObjectType('e','l','p','d')
#define	kMF3DObjEllipsoidText					"Ellipsoid"

typedef struct MF3DEllipsoidData
  {
    MF3DVector3D orientation;
    MF3DVector3D majorRadius;
    MF3DVector3D minorRadius;
    MF3DPoint3D origin;
  }
MF3DEllipsoidData, *MF3DEllipsoidDataPtr;

typedef struct MF3DEllipsoidObj
  {
    MF3DObjHeader
    MF3DEllipsoidDataPtr ellipsoid;	/* (optional)           */
  }
MF3DEllipsoidObj, *MF3DEllipsoidObjPtr;

/*==============================================================================
 *	GeneralPolygonHint
 *==============================================================================
 */
#define	kMF3DObjGeneralPolygonHint				MF3DObjectType('g','p','l','h')
#define	kMF3DObjGeneralPolygonHintText			"GeneralPolygonHint"

enum
  {
    MF3DShapeComplex = 0x00000000,
    MF3DShapeConcave = 0x00000001,
    MF3DShapeConvex = 0x00000002,
    MF3DShapeUnknown = MF3DShapeComplex
  };
typedef MF3DEnumData MF3DGeneralPolygonHintEnum;

typedef struct MF3DGeneralPolygonHintObj
  {
    MF3DObjHeader
    MF3DGeneralPolygonHintEnum shapeHint;
  }
MF3DGeneralPolygonHintObj, *MF3DGeneralPolygonHintObjPtr;

/*==============================================================================
 *	GeneralPolygon
 *
 *	0 < nContours
 *	2 < nVertices
 *
 *	Optionally followed by:
 *		{ nVertices = sum(nContours.nVertices }
 *		MF3DVertexAttributeSetListObjPtr	vertexAttributes;	{ 0-1 }
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *		MF3DGeneralPolygonHintObjPtr		polygonHint;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjGeneralPolygon					MF3DObjectType('g','p','g','n')
#define	kMF3DObjGeneralPolygonText				"GeneralPolygon"

typedef struct MF3DPolygonData
  {
    MF3DUns32 nVertices;
    MF3DPoint3DPtr vertices;	/* size: nVertices                      */
  }
MF3DPolygonData, *MF3DPolygonDataPtr;

typedef struct MF3DGeneralPolygonObj
  {
    MF3DObjHeader
    MF3DUns32 nContours;
    MF3DPolygonDataPtr polygons;	/* size: nContours                      */
  }
MF3DGeneralPolygonObj, *MF3DGeneralPolygonObjPtr;

/*==============================================================================
 *	Line
 *
 *	Optionally followed by:
 *		MF3DVertexAttributeSetListObjPtr	vertexAttributes;	{ 0-1, nV = 2 }
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjLine							MF3DObjectType('l','i','n','e')
#define	kMF3DObjLineText						"Line"

typedef struct MF3DLineObj
  {
    MF3DObjHeader
    MF3DPoint3D start;
    MF3DPoint3D end;
  }
MF3DLineObj, *MF3DLineObjPtr;

/*==============================================================================
 *	Marker
 *
 *	0 < width
 *	0 < height
 *	(((width / 8) + ((width & 7) > 0)) ó rowBytes
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjMarker							MF3DObjectType('m','r','k','r')
#define	kMF3DObjMarkerText						"Marker"

typedef struct MF3DMarkerObj
  {
    MF3DObjHeader
    MF3DPoint3D location;
    MF3DUns32 width;
    MF3DUns32 height;
    MF3DUns32 rowBytes;
    MF3DInt32 xOffset;
    MF3DInt32 yOffset;
    MF3DRawDataPtr data;	/* size: height*rowBytes */
  }
MF3DMarkerObj, *MF3DMarkerObjPtr;

/*==============================================================================
 *	Mesh
 *
 *	3 <= nVertices
 *	3 <= nFaceVertexIndices
 *
 *	Optionally followed by:
 *		MF3DFaceAttributeSetListObjPtr	 faceAttributes;	{ 0-1, nFaces }
 *		MF3DVertexAttributeSetListObjPtr vertexAttributes;	{ 0-1, nVertices }
 *		MF3DMeshCornersObjPtr			 meshCorners;		{ 0-1 }
 *		MF3DMeshEdgesObjPtr				 meshEdges;			{ 0-1 }
 *		MF3DAttributeSetListObjPtr		 attributes;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjMesh							MF3DObjectType('m','e','s','h')
#define	kMF3DObjMeshText						"Mesh"

typedef struct MF3DMeshFace
  {
    MF3DUns32 nFaceVertexIndices;
    MF3DUns32Ptr faceVertexIndices;	/* size: nFaceVertexIndices     */
  }
MF3DMeshFace, *MF3DMeshFacePtr;

typedef struct MF3DMeshObj
  {
    MF3DObjHeader
    MF3DUns32 nVertices;
    MF3DPoint3DPtr vertices;	/* size: nVertices                      */
    MF3DUns32 nFaces;
    MF3DUns32 nContours;
    MF3DMeshFacePtr faces;	/* size: nFaces + nContours     */
  }
MF3DMeshObj, *MF3DMeshObjPtr;

/*==============================================================================
 *	NURBCurve
 *
 *	2 <= order
 *	2 <= nPoints
 *	0 < points[...].w (weights of points)
 *==============================================================================
 */
#define	kMF3DObjNURBCurve						MF3DObjectType('n','r','b','c')
#define	kMF3DObjNURBCurveText					"NURBCurve"

typedef MF3DFloat32 MF3DKnot, *MF3DKnotPtr;

typedef struct MF3DNURBCurveObj
  {
    MF3DObjHeader
    MF3DUns32 order;
    MF3DUns32 nPoints;
    MF3DRationalPoint4DPtr points;	/* size: nPoints                        */
    MF3DKnotPtr knots;		/* size: order + nPoints        */
  }
MF3DNURBCurveObj, *MF3DNURBCurveObjPtr;

/*==============================================================================
 *	NURBCurve2D
 *
 *	2 ó order
 *	2 ó nPoints
 *	0 < points[...].w (weights of points)
 *==============================================================================
 */
#define	kMF3DObjNURBCurve2D						MF3DObjectType('n','b','2','c')
#define	kMF3DObjNURBCurve2DText					"NURBCurve2D"

typedef struct MF3DNURBCurve2DObj
  {
    MF3DObjHeader
    MF3DUns32 order;
    MF3DUns32 nPoints;
    MF3DRationalPoint3DPtr points;	/* size: nPoints                        */
    MF3DKnotPtr knots;		/* size: order + nPoints        */
  }
MF3DNURBCurve2DObj, *MF3DNURBCurve2DObjPtr;

/*==============================================================================
 *	NurbPatch
 *
 *	2 <= uOrder
 *	2 <= vOrder
 *	2 <= numColumns
 *	2 <= numRows
 *	0 < points[...].w (weights of points)
 *
 *	Optionally followed by:
 *		MF3DTrimCurvesObjPtr	trimCurves;		{ 0-1, nTrimCurves = 1 to many }
 *==============================================================================
 */
#define	kMF3DObjNURBPatch						MF3DObjectType('n','r','b','p')
#define	kMF3DObjNURBPatchText					"NURBPatch"

typedef struct MF3DNurbPatchObj
  {
    MF3DObjHeader
    MF3DUns32 uOrder;
    MF3DUns32 vOrder;
    MF3DUns32 numColumns;
    MF3DUns32 numRows;
    MF3DRationalPoint4DPtr points;	/* size: numColumns*numRows     */
    MF3DKnotPtr uKnots;		/* size: uOrder + numColumn     */
    MF3DKnotPtr vKnots;		/* size: vOrder + numRows       */
  }
MF3DNurbPatchObj, *MF3DNurbPatchObjPtr;

/*==============================================================================
 *	Point
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjPoint							MF3DObjectType('p','n','t',' ')
#define	kMF3DObjPointText						"Point"

typedef struct MF3DPointObj
  {
    MF3DObjHeader
    MF3DPoint3D point;
  }
MF3DPointObj, *MF3DPointObjPtr;

/*==============================================================================
 *	Polygon
 *
 *	2 <= nVertices
 *
 *	Optionally followed by:
 *		MF3DVertexAttributeSetListObjPtr vertexAttributes;	{ 0-1, nVertices }
 *		MF3DAttributeSetListObjPtr		 attributes;		{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjPolygon							MF3DObjectType('p','l','y','g')
#define	kMF3DObjPolygonText						"Polygon"

typedef struct MF3DPolygonObj
  {
    MF3DObjHeader
    MF3DUns32 nVertices;
    MF3DPoint3DPtr vertices;	/* size: nVertices      */
  }
MF3DPolygonObj, *MF3DPolygonObjPtr;

/*==============================================================================
 *	PolyLine
 *
 *	2 <= nVertices
 *
 *	Optionally followed by:
 *		{ nVertices }
 *		MF3DVertexAttributeSetListObjPtr	vertexAttributes;	{ 0-1 }
 *		{ nGeometries = nVertices - 1 }
 *		MF3DGeometryAttributeSetListObjPtr	geometryAttributes;	{ 0-1 }
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjPolyLine						MF3DObjectType('p','l','y','l')
#define	kMF3DObjPolyLineText					"PolyLine"

typedef struct MF3DPolyLineObj
  {
    MF3DObjHeader
    MF3DUns32 nVertices;
    MF3DPoint3DPtr vertices;	/* size: nVertices      */
  }
MF3DPolyLineObj, *MF3DPolyLineObjPtr;

/*==============================================================================
 *	Torus
 *
 *	For 0-sized objects, default is:
 *
 *	1 0 0 # orientation
 *	0 1 0 # majorAxis
 *	0 0 1 # minorAxis
 *	0 0 0 # origin
 *	1     # ratio
 *
 *	Optionally followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjTorus							MF3DObjectType('t','o','r','s')
#define	kMF3DObjTorusText						"Torus"

typedef struct MF3DTorusData
  {
    MF3DVector3D orientation;
    MF3DVector3D majorAxis;
    MF3DVector3D minorAxis;
    MF3DPoint3D origin;
    MF3DFloat32 ratio;
  }
MF3DTorusData, *MF3DTorusDataPtr;

typedef struct MF3DTorusObj
  {
    MF3DObjHeader
    MF3DTorusDataPtr torus;	/* (optional)                           */
  }
MF3DTorusObj, *MF3DTorusObjPtr;

/*==============================================================================
 *	Triangle
 *
 *	Optionally followed by:
 *		MF3DVertexAttributeSetListObjPtr	vertexAttributes;	{ 0-1, n = 3 }
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjTriangle						MF3DObjectType('t','r','n','g')
#define	kMF3DObjTriangleText					"Triangle"

typedef struct MF3DTriangleObj
  {
    MF3DObjHeader
    MF3DPoint3D vertex1;
    MF3DPoint3D vertex2;
    MF3DPoint3D vertex3;
  }
MF3DTriangleObj, *MF3DTriangleObjPtr;

/*==============================================================================
 *	TriGrid
 *
 *	2 ò numRows
 *	2 ò numColumns
 *
 *	vertices are specified in row-major order. For example:
 *
 *	numRows = 2, numColumns = 4
 *
 *        col0 col1 col2 col3
 *	row0   p1   p2   p3   p4
 *  row1   p5   p6   p7   p8
 *
 *	In the metafile, vertices are specified
 *	p1 p2 p3 p4 p5 p6 p7 p8
 *
 *	Optionally followed by:
 *		{ nFaces = (numRows - 1) * (numColumns - 1) * 2 }
 *		MF3DFaceAttributeSetListObjPtr		faceAttributes;		{ 0-1 }
 *		{ nVertices = numRows * numColumns }
 *		MF3DVertexAttributeSetListObjPtr	vertexAttributes;	{ 0-1 }
 *		MF3DAttributeSetListObjPtr			attributes;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjTriGrid							MF3DObjectType('t','r','i','g')
#define	kMF3DObjTriGridText						"TriGrid"

typedef struct MF3DTriGridObj
  {
    MF3DObjHeader
    MF3DUns32 numRows;
    MF3DUns32 numColumns;
    MF3DPoint3DPtr vertices;	/* size: numColumns * numRows */
  }
MF3DTriGridObj, *MF3DTriGridObjPtr;

/*==============================================================================
 *	LightData
 *
 *	0 ó intensity ó 1
 *==============================================================================
 */
#define	kMF3DObjLightData						MF3DObjectType('l','g','h','t')
#define	kMF3DObjLightDataText					"LightData"

#define	MF3DLightDataIsOnDefault		true
#define	MF3DLightDataIntensityDefault	1.0
#define	MF3DLightDataColorDefault		{ 1, 1, 1 }

typedef struct MF3DLightDataObj
  {
    MF3DObjHeader
    MF3DBoolean isOn;
    MF3DFloat32 intensity;	/* 0 <= intensity <= 1  */
    MF3DRGBColor color;
  }
MF3DLightDataObj, *MF3DLightDataObjPtr;

/*==============================================================================
 *	AmbientLight
 *
 *	Optionally followed by:
 *		MF3DLightDataObjPtr		lightData;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjAmbientLight					MF3DObjectType('a','m','b','n')
#define	kMF3DObjAmbientLightText				"AmbientLight"

typedef struct MF3DAmbientLightObj
  {
    MF3DObjHeader
  }
MF3DAmbientLightObj, *MF3DAmbientLightObjPtr;

/*==============================================================================
 *	DirectionalLight
 *
 *	|direction| = 1.0
 *
 *	Optionally followed by:
 *		MF3DLightDataObjPtr		lightData;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjDirectionalLight				MF3DObjectType('d','r','c','t')
#define	kMF3DObjDirectionalLightText			"DirectionalLight"

typedef struct MF3DDirectionalLightObj
  {
    MF3DObjHeader
    MF3DVector3D direction;
    MF3DBoolean castsShadows;
  }
MF3DDirectionalLightObj, *MF3DDirectionalLightObjPtr;

/*==============================================================================
 *	PointLight
 *
 *	attenuation = 1/(c0 + c1*d + c2 + d^2), d is the distance from location
 *	0 < c0, c1,c2
 *	attenuation is not clamped to [0,1] to allow for lighting washout
 *
 *	Optionally followed by:
 *		MF3DLightDataObjPtr		lightData;			{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjPointLight						MF3DObjectType('p','n','t','l')
#define	kMF3DObjPointLightText					"PointLight"

typedef struct MF3DAttenuation
  {
    MF3DFloat32 c0;
    MF3DFloat32 c1;
    MF3DFloat32 c2;
  }
MF3DAttenuation;

typedef struct MF3DPointLightObj
  {
    MF3DObjHeader
    MF3DPoint3D location;
    MF3DAttenuation attenuation;
    MF3DBoolean castsShadows;
  }
MF3DPointLightObj, *MF3DPointLightObjPtr;

/*==============================================================================
 *	SpotLight
 *
 *	|orientation| = 1
 *	0 < hotAngle <= outerAngle <= pi
 *==============================================================================
 */
#define	kMF3DObjSpotLight						MF3DObjectType('s','p','o','t')
#define	kMF3DObjSpotLightText					"SpotLight"

enum
  {
    MF3DFallOffNone = 0x00000000,
    MF3DFallOffLinear = 0x00000001,
    MF3DFallOffExponential = 0x00000002,
    MF3DFallOffCosine = 0x00000003
  };
typedef MF3DEnumData MF3DFallOffEnum;

typedef struct MF3DSpotLightObj
  {
    MF3DObjHeader
    MF3DPoint3D location;
    MF3DVector3D orientation;
    MF3DBoolean castsShadows;
    MF3DAttenuation attenuation;
    MF3DFloat32 hotAngle;
    MF3DFloat32 outerAngle;
    MF3DFallOffEnum fallOff;
  }
MF3DSpotLightObj, *MF3DSpotLightObjPtr;

/*==============================================================================
 *	LambertIllumination
 *==============================================================================
 */
#define	kMF3DObjLambertIllumination				MF3DObjectType('l','m','i','l')
#define	kMF3DObjLambertIlluminationText			"LambertIllumination"

typedef struct MF3DLambertIlluminationObj
  {
    MF3DObjHeader
  }
MF3DLambertIlluminationObj, *MF3DLambertIlluminationObjPtr;

/*==============================================================================
 *	PhongIllumination
 *==============================================================================
 */
#define	kMF3DObjPhongIllumination				MF3DObjectType('p','h','i','l')
#define	kMF3DObjPhongIlluminationText			"PhongIllumination"

typedef struct MF3DPhongIlluminationObj
  {
    MF3DObjHeader
  }
MF3DPhongIlluminationObj, *MF3DPhongIlluminationObjPtr;

/*==============================================================================
 *	PixmapTexture
 *
 *	0 < width
 *	0 < height
 *	0 < pixelSize < 32
 *	width * pixelSize ó rowBytes
 *==============================================================================
 */
#define	kMF3DObjPixmapTexture					MF3DObjectType('t','x','p','m')
#define	kMF3DObjPixmapTextureText				"PixmapTexture"

enum
  {
    MF3DPixelRGB8 = 0x00000000,
    MF3DPixelRGB16 = 0x00000001,
    MF3DPixelRGB24 = 0x00000002,
    MF3DPixelRGB32 = 0x00000003
  };
typedef MF3DEnumData MF3DPixelTypeEnum;

enum
  {
    MF3DEndianBigEndian = 0x00000000,
    MF3DEndianLittleEndian = 0x00000001
  };
typedef MF3DEnumData MF3DEndianEnum;

typedef struct MF3DPixmapTextureObj
  {
    MF3DObjHeader
    MF3DUns32 width;
    MF3DUns32 height;
    MF3DUns32 rowBytes;
    MF3DUns32 pixelSize;
    MF3DPixelTypeEnum pixelType;
    MF3DEndianEnum bitOrder;
    MF3DEndianEnum byteOrder;
    MF3DRawDataPtr image;	/* size: rowBytes * height */
  }
MF3DPixmapTextureObj, *MF3DPixmapTextureObjPtr;

/*==============================================================================
 *	TextureShader
 *
 *	Always followed by:
 *		MF3DPixmapTextureObjPtr	pixmapTexture;		{ 1 }
 *==============================================================================
 */
#define	kMF3DObjTextureShader					MF3DObjectType('t','x','s','u')
#define	kMF3DObjTextureShaderText				"TextureShader"

typedef struct MF3DTextureShaderObj
  {
    MF3DObjHeader
  }
MF3DTextureShaderObj, *MF3DTextureShaderObjPtr;

/*==============================================================================
 *	BackfacingStyle
 *==============================================================================
 */
#define	kMF3DObjBackfacingStyle					MF3DObjectType('b','c','k','f')
#define	kMF3DObjBackfacingStyleText				"BackfacingStyle"

enum
  {
    MF3DBackfacingBoth = 0x00000000,
    MF3DBackfacingCulled = 0x00000001,
    MF3DBackfacingFlipped = 0x00000002
  };
typedef MF3DEnumData MF3DBackfacingEnum;

typedef struct MF3DBackfacingStyleObj
  {
    MF3DObjHeader
    MF3DBackfacingEnum backfacing;
  }
MF3DBackfacingStyleObj, *MF3DBackfacingStyleObjPtr;

/*==============================================================================
 *	FillStyle
 *==============================================================================
 */
#define	kMF3DObjFillStyle						MF3DObjectType('f','i','s','t')
#define	kMF3DObjFillStyleText					"FillStyle"

enum
  {
    MF3DFillStyleFilled = 0x00000000,
    MF3DFillStyleEdges = 0x00000001,
    MF3DFillStylePoints = 0x00000002,
    MF3DFillStyleEmpty = 0x00000003
  };
typedef MF3DEnumData MF3DFillStyleEnum;

typedef struct MF3DFillStyleObj
  {
    MF3DObjHeader
    MF3DFillStyleEnum fillStyle;
  }
MF3DFillStyleObj, *MF3DFillStyleObjPtr;

/*==============================================================================
 *	HighlightStyle
 *
 *	Always followed by:
 *		MF3DAttributeSetListObjPtr			attributes;			{ 1 }
 *==============================================================================
 */
#define	kMF3DObjHighlightStyle					MF3DObjectType('h','i','g','h')
#define	kMF3DObjHighlightStyleText				"HighlightStyle"

typedef struct MF3DHighlightStyleObj
  {
    MF3DObjHeader
  }
MF3DHighlightStyleObj, *MF3DHighlightStyleObjPtr;

/*==============================================================================
 *	InterpolationStyle
 *==============================================================================
 */
#define	kMF3DObjInterpolationStyle				MF3DObjectType('i','n','t','p')
#define	kMF3DObjInterpolationStyleText			"InterpolationStyle"

enum
  {
    MF3DInterpolationStyleNone = 0x00000000,
    MF3DInterpolationStyleVertex = 0x00000001,
    MF3DInterpolationStylePixel = 0x00000002
  };
typedef MF3DEnumData MF3DInterpolationStyleEnum;

typedef struct MF3DInterpolationStyleObj
  {
    MF3DObjHeader
    MF3DInterpolationStyleEnum interpolationStyle;
  }
MF3DInterpolationStyleObj, *MF3DInterpolationStyleObjPtr;

/*==============================================================================
 *	OrientationStyle
 *==============================================================================
 */
#define	kMF3DObjOrientationStyle				MF3DObjectType('o','r','n','t')
#define	kMF3DObjOrientationStyleText			"OrientationStyle"

enum
  {
    MF3DOrientationCCW = 0x00000000,
    MF3DOrientationCW = 0x00000001
  };
typedef MF3DEnumData MF3DOrientationStyleEnum;

typedef struct MF3DOrientationStyleObj
  {
    MF3DObjHeader
    MF3DOrientationStyleEnum orientationStyle;
  }
MF3DOrientationStyleObj, *MF3DOrientationStyleObjPtr;

/*==============================================================================
 *	PickIDStyle
 *==============================================================================
 */
#define	kMF3DObjPickIDStyle						MF3DObjectType('p','k','i','d')
#define	kMF3DObjPickIDStyleText					"PickIDStyle"

typedef struct MF3DPickIDStyleObj
  {
    MF3DObjHeader
    MF3DUns32 id;
  }
MF3DPickIDStyleObj, *MF3DPickIDStyleObjPtr;

/*==============================================================================
 *	PickPartsStyle
 *==============================================================================
 */
#define	kMF3DObjPickPartsStyle					MF3DObjectType('p','k','p','t')
#define	kMF3DObjPickPartsStyleText				"PickPartsStyle"

enum
  {
    MF3DPickPartsObject = 0x00000000,
    MF3DPickPartsFace = 0x00000001,
    MF3DPickPartsEdge = 0x00000002,
    MF3DPickPartsVertex = 0x00000004,
    MF3DPickPartsDefault = MF3DPickPartsObject
  };
typedef MF3DFlagData MF3DPickPartsFlags;

typedef struct MF3DPickPartsStyleObj
  {
    MF3DObjHeader
    MF3DPickPartsFlags pickParts;
  }
MF3DPickPartsStyleObj, *MF3DPickPartsStyleObjPtr;

/*==============================================================================
 *	ReceiveShadowsStyle
 *==============================================================================
 */
#define	kMF3DObjReceiveShadowsStyle				MF3DObjectType('r','c','s','h')
#define	kMF3DObjReceiveShadowsStyleText			"ReceiveShadowsStyle"

typedef struct MF3DReceiveShadowsStyleObj
  {
    MF3DObjHeader
    MF3DBoolean receiveShadows;
  }
MF3DReceiveShadowsStyleObj, *MF3DReceiveShadowsStyleObjPtr;

/*==============================================================================
 *	SubdivisionStyle
 *==============================================================================
 */
#define	kMF3DObjSubdivisionStyle				MF3DObjectType('s','b','d','v')
#define	kMF3DObjSubdivisionStyleText			"SubdivisionStyle"

enum
  {
    MF3DSubdivisionConstant = 0x00000000,
    MF3DSubdivisionWorldSpace = 0x00000001,
    MF3DSubdivisionScreenSpace = 0x00000002
  };
typedef MF3DEnumData MF3DSubdivisionMethodEnum;

typedef struct MF3DSubdivisionConstantData
  {
    MF3DUns32 value1;
    MF3DUns32 value2;
  }
MF3DSubdivisionConstantData;

typedef struct MF3DSubdivisionSpaceData
  {
    MF3DFloat32 value1;
  }
MF3DSubdivisionSpaceData;

typedef union
  {
    MF3DSubdivisionConstantData constant;
    MF3DSubdivisionSpaceData space;
  }
MF3DSubdivisionData;

typedef struct MF3DSubdivisionStyleObj
  {
    MF3DObjHeader
    MF3DSubdivisionMethodEnum subdivisionMethod;
    MF3DSubdivisionData subdivisionData;
  }
MF3DSubdivisionStyleObj, *MF3DSubdivisionStyleObjPtr;

/*==============================================================================
 *	Matrix
 *
 *	matrix is invertible
 *==============================================================================
 */
#define	kMF3DObjMatrixTransform					MF3DObjectType('m','t','r','x')
#define	kMF3DObjMatrixTransformText				"Matrix"

typedef struct MF3DMatrixObj
  {
    MF3DObjHeader
    MF3DMatrix4x4 matrix;
  }
MF3DMatrixObj, *MF3DMatrixObjPtr;

/*==============================================================================
 *	Quaternion
 *==============================================================================
 */
#define	kMF3DObjQuaternionTransform				MF3DObjectType('q','t','r','n')
#define	kMF3DObjQuaternionTransformText			"Quaternion"

typedef struct MF3DQuaternion
  {
    MF3DFloat32 w;
    MF3DFloat32 x;
    MF3DFloat32 y;
    MF3DFloat32 z;
  }
MF3DQuaternion;

typedef struct MF3DQuaternionObj
  {
    MF3DObjHeader
    MF3DQuaternion quaternion;
  }
MF3DQuaternionObj, *MF3DQuaternionObjPtr;

/*==============================================================================
 *	Rotate
 *==============================================================================
 */
#define	kMF3DObjRotateTransform					MF3DObjectType('r','o','t','t')
#define	kMF3DObjRotateTransformText				"Rotate"

enum
  {
    MF3DAxisX = 0x00000000,
    MF3DAxisY = 0x00000001,
    MF3DAxisZ = 0x00000002
  };
typedef MF3DEnumData MF3DAxisEnum;

typedef struct MF3DRotateObj
  {
    MF3DObjHeader
    MF3DAxisEnum axis;
    MF3DFloat32 radians;
  }
MF3DRotateObj, *MF3DRotateObjPtr;

/*==============================================================================
 *	RotateAboutAxis
 *
 *	|orientation| = 1
 *==============================================================================
 */
#define	kMF3DObjRotateAboutAxisTransform		MF3DObjectType('r','t','a','a')
#define	kMF3DObjRotateAboutAxisTransformText	"RotateAboutAxis"

typedef struct MF3DRotateAboutAxisObj
  {
    MF3DObjHeader
    MF3DPoint3D origin;
    MF3DVector3D orientation;
    MF3DFloat32 radians;
  }
MF3DRotateAboutAxisObj, *MF3DRotateAboutAxisObjPtr;

/*==============================================================================
 *	RotateAboutPoint
 *==============================================================================
 */
#define	kMF3DObjRotateAboutPointTransform		MF3DObjectType('r','t','a','p')
#define	kMF3DObjRotateAboutPointTransformText	"RotateAboutPoint"

typedef struct MF3DRotateAboutPointObj
  {
    MF3DObjHeader
    MF3DAxisEnum axis;
    MF3DFloat32 radians;
    MF3DPoint3D origin;
  }
MF3DRotateAboutPointObj, *MF3DRotateAboutPointObjPtr;

/*==============================================================================
 *	Scale
 *==============================================================================
 */
#define	kMF3DObjScaleTransform					MF3DObjectType('s','c','a','l')
#define	kMF3DObjScaleTransformText				"Scale"

typedef struct MF3DScaleObj
  {
    MF3DObjHeader
    MF3DVector3D scale;
  }
MF3DScaleObj, *MF3DScaleObjPtr;

/*==============================================================================
 *	Translate
 *==============================================================================
 */
#define	kMF3DObjTranslateTransform				MF3DObjectType('t','r','n','s')
#define	kMF3DObjTranslateTransformText			"Translate"

typedef struct MF3DTranslateObj
  {
    MF3DObjHeader
    MF3DVector3D translate;
  }
MF3DTranslateObj, *MF3DTranslateObjPtr;

/*==============================================================================
 *	MacintoshPath
 *==============================================================================
 */
#define	kMF3DObjMacintoshPath					MF3DObjectType('a','l','i','s')
#define	kMF3DObjMacintoshPathText				"MacintoshPath"

typedef struct MF3DMacintoshPathObj
  {
    MF3DObjHeader
    MF3DCStringPtr pathName;
  }
MF3DMacintoshPathObj, *MF3DMacintoshPathObjPtr;

/*==============================================================================
 *	UnixPath
 *==============================================================================
 */
#define	kMF3DObjUnixPath						MF3DObjectType('u','n','i','x')
#define	kMF3DObjUnixPathText					"UnixPath"

typedef struct MF3DUnixPathObj
  {
    MF3DObjHeader
    MF3DCStringPtr pathName;
  }
MF3DUnixPathObj, *MF3DUnixPathObjPtr;

/*==============================================================================
 *	CString
 *==============================================================================
 */
#define	kMF3DObjCString							MF3DObjectType('s','t','r','c')
#define	kMF3DObjCStringText						"CString"

typedef struct MF3DCStringObj
  {
    MF3DObjHeader
    MF3DCStringPtr cString;
  }
MF3DCStringObj, *MF3DCStringObjPtr;

/*==============================================================================
 *	Unicode
 *==============================================================================
 */
#define	kMF3DObjUnicode							MF3DObjectType('u','n','c','d')
#define	kMF3DObjUnicodeText						"Unicode"

typedef struct MF3DUnicodeObj
  {
    MF3DObjHeader
    MF3DUns32 length;
    MF3DRawDataPtr unicode;	/* size: length * 2     */
  }
MF3DUnicodeObj, *MF3DUnicodeObjPtr;

/*==============================================================================
 *	ImageDimensions
 *
 *	0 < width
 *	0 < height
 *==============================================================================
 */
#define	kMF3DObjImageDimensions					MF3DObjectType('i','m','d','m')
#define	kMF3DObjImageDimensionsText				"ImageDimensions"

typedef struct MF3DImageDimensionsObj
  {
    MF3DObjHeader
    MF3DUns32 width;
    MF3DUns32 height;
  }
MF3DImageDimensionsObj, *MF3DImageDimensionsObjPtr;

/*==============================================================================
 *	ImageMask

 *	width, height in bits
 *	0 < width 
 *	0 < height
 *	((width >> 3) + ((width & 0x7) ? 1 : 0)) ó rowBytes
 *==============================================================================
 */
#define	kMF3DObjImageMask						MF3DObjectType('i','m','m','k')
#define	kMF3DObjImageMaskText					"ImageMask"

typedef struct MF3DImageMaskObj
  {
    MF3DObjHeader
    MF3DUns32 width;
    MF3DUns32 height;
    MF3DUns32 rowBytes;
    MF3DRawDataPtr image;	/* size: rowBytes * height              */
  }
MF3DImageMaskObj, *MF3DImageMaskObjPtr;

/*==============================================================================
 *	ImageClearColor
 *==============================================================================
 */
#define	kMF3DObjImageClearColor					MF3DObjectType('i','m','c','c')
#define	kMF3DObjImageClearColorText				"ImageClearColor"

typedef struct MF3DImageClearColorObj
  {
    MF3DObjHeader
    MF3DRGBColor clearColor;
  }
MF3DImageClearColorObj, *MF3DImageClearColorObjPtr;

/*==============================================================================
 *	ViewHints
 *
 *	Optionally followed by:
 *		MF3DRendererObjPtr			renderer;			{ 0-1 }
 *		MF3DCameraObjPtr			camera;				{ 0-1 }
 *		MF3DLightsObjPtr			lights;				{ 0-many }
 *		MF3DAttributeSetListObjPtr	attributes;			{ 0-1 }
 *		MF3DImageDimensionsObjPtr	imageDimensions;	{ 0-1 }
 *		MF3DImageMaskObjPtr			imageMask;			{ 0-1 }
 *		MF3DImageClearColorObjPtr	imageClearColor;	{ 0-1 }
 *==============================================================================
 */
#define	kMF3DObjViewHints						MF3DObjectType('v','w','h','n')
#define	kMF3DObjViewHintsText					"ViewHints"

typedef MF3DVoidObjPtr MF3DRendererObjPtr;
typedef MF3DVoidObjPtr MF3DCameraObjPtr;
typedef MF3DVoidObjPtr MF3DLightsObjPtr;

typedef struct MF3DViewHintsObj
  {
    MF3DObjHeader
  }
MF3DViewHintsObj, *MF3DViewHintsObjPtr;

/*==============================================================================
 *	Shader
 *
 *	Optionally followed by:
 *		MF3DShaderDataObjPtr		shaderData;			{ 0-1 }
 *		MF3DShaderTransformObjPtr	shaderTransform;	{ 0-1 }
 *		MF3DShaderUVTransformObjPtr	shaderUVTransform;	{ 0-1 }
 *==============================================================================
 */
typedef MF3DVoidObjPtr MF3DShaderDataPtr;

/*==============================================================================
 *	UnknownBinary
 *==============================================================================
 */
#define	kMF3DObjUnknownBinary					MF3DObjectType('u','k','b','n')
#define	kMF3DObjUnknownBinaryText				"UnknownBinary"

typedef struct MF3DUnknownBinaryObj
  {
    MF3DObjHeader
    MF3DInt32 realObjectType;
    MF3DUns32 realObjectSize;
    MF3DEndianEnum realObjectOrder;	/* big endian or little endian  */
    MF3DRawDataPtr realObjectData;	/* size: realObjectSize                 */
  }
MF3DUnknownBinaryObj, *MF3DUnknownBinaryObjPtr;

/*==============================================================================
 *	Unknown Type Object
 *
 *	Unrecognized objects are just stored as a stream of data.
 *	We use this object to return User-defined types. We could be even more
 *	adventurous and use it for any type we do not recognize.
 *==============================================================================
 */

/* Not an official type; so pick a value most likely not to
 * conflict with other types.
 */
#define	kMF3DObjUnknownType						(MF3DObjType)0x80000000

/* objectType is kMF3DObjUnknownType.                                                                           */
typedef struct MF3DUnknownObj
  {
    MF3DObjHeader
    MF3DObjType realObjectType;
    MF3DCStringPtr realObjectName;	/* (optional)                           */
    MF3DDataFormatEnum dataFormat;	/* binary, swapped, or text     */
    MF3DSize size;
    MF3DRawDataPtr data;	/* size: size                           */
  }
MF3DUnknownObj, *MF3DUnknownObjPtr;

/*==============================================================================
 *	Unrecognized Subobject List
 *
 *	Future versions of the metafile may support new subobject types.
 *	Unrecognized subobjects are stored in an array.
 *==============================================================================
 */
struct MF3DUnknownSubobjList
  {
    MF3DUns32 nSubObjects;
    MF3DUnknownObjPtr subObjects;	/* size: nSubObjects                    */
  };

#endif
