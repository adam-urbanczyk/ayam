#ifndef	MF3D_OBJECTS_H
#define	MF3D_OBJECTS_H
/*==============================================================================
 *
 *	File:		MFOBJECT.H
 *
 *	Function:	Object read/write
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
 *		FB3_JRK	Added UnknownBinary (ukbn)
 *		Fabio	Changed file name to 8 characters
 *		F3K_RWW	GeneralPolygonHint.
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2S_RWW	BeginGroup changes.
 *		F2R_RWW	Change to simple object theory.
 *		F2H_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mftypes.h"
#include "mfintobj.h"

MF3DErr MF3D_IntReadObject (MF3D_FilePtr inMF3DFilePtr,
			    MF3DVoidObjPtr * outMF3DObjPtr);

MF3DErr MF3D_ObjMetafileReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMetafileWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMetafileDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjContainerReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjContainerWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjContainerDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEndContainerWriter (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEndContainerDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBeginGroupReader (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjBeginGroupWriter (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBeginGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEndGroupReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjEndGroupWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEndGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjReferenceReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjReferenceWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjReferenceDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTableOfContentsReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
/* MF3D_ObjTableOfContentsWriter is called only by MF3DClose */
MF3DErr MF3D_ObjTableOfContentsWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTableOfContentsDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTypeReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTypeWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTypeDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFaceAttributeSetListReader (MF3D_FilePtr inMetafilePtr,
					    MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjFaceAttributeSetListWriter (MF3D_FilePtr inMetafilePtr,
					    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFaceAttributeSetListDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeometryAttributeSetListReader (MF3D_FilePtr inMetafilePtr,
						MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjGeometryAttributeSetListWriter (MF3D_FilePtr inMetafilePtr,
						MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeometryAttributeSetListDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjVertexAttributeSetListReader (MF3D_FilePtr inMetafilePtr,
					      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjVertexAttributeSetListWriter (MF3D_FilePtr inMetafilePtr,
					      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjVertexAttributeSetListDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraPlacementReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCameraPlacementWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraPlacementDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraRangeReader (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCameraRangeWriter (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraRangeDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraViewPortReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCameraViewPortWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCameraViewPortDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBottomCapAttributeSetReader (MF3D_FilePtr inMetafilePtr,
					     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjBottomCapAttributeSetWriter (MF3D_FilePtr inMetafilePtr,
					     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBottomCapAttributeSetDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCapsReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCapsWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCapsDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFaceCapAttributeSetReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjFaceCapAttributeSetWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFaceCapAttributeSetDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTopCapAttributeSetReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTopCapAttributeSetWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTopCapAttributeSetDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDisplayGroupStateReader (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjDisplayGroupStateWriter (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDisplayGroupStateDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLightDataReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjLightDataWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLightDataDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshCornersReader (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMeshCornersWriter (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshCornersDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshEdgesReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMeshEdgesWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshEdgesDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBCurve2DReader (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjNURBCurve2DWriter (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBCurve2DDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderDataReader (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjShaderDataWriter (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderDataDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderTransformReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjShaderTransformWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderUVTransformReader (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjShaderUVTransformWriter (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShaderUVTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTrimCurvesReader (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTrimCurvesWriter (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTrimCurvesDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageClearColorReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjImageClearColorWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageClearColorDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageDimensionsReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjImageDimensionsWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageDimensionsDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageMaskReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjImageMaskWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjImageMaskDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAmbientCoefficientReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjAmbientCoefficientWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAmbientCoefficientDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDiffuseColorReader (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjDiffuseColorWriter (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDiffuseColorDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjHighlightStateReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjHighlightStateWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjHighlightStateDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNormalReader (MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjNormalWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNormalDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShadingUVReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjShadingUVWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjShadingUVDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpecularColorReader (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSpecularColorWriter (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpecularColorDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpecularControlReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSpecularControlWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpecularControlDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSurfaceTangentReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSurfaceTangentWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSurfaceTangentDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSurfaceUVReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSurfaceUVWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSurfaceUVDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTransparencyColorReader (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTransparencyColorWriter (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTransparencyColorDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererInteractiveReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRendererInteractiveWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererInteractiveDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererGenericReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRendererGenericWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererGenericDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererWireFrameReader (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRendererWireFrameWriter (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererWireFrameDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererZBufferReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRendererZBufferWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRendererZBufferDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAttributeSetReader (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjAttributeSetWriter (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAttributeSetDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrthographicCameraReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjOrthographicCameraWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrthographicCameraDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewAngleAspectCameraReader (MF3D_FilePtr inMetafilePtr,
					     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjViewAngleAspectCameraWriter (MF3D_FilePtr inMetafilePtr,
					     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewAngleAspectCameraDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewPlaneCameraReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjViewPlaneCameraWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewPlaneCameraDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBoxReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjBoxWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBoxDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjConeReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjConeWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjConeDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCylinderReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCylinderWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCylinderDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDiskReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjDiskWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDiskDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEllipseReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjEllipseWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEllipseDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEllipsoidReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjEllipsoidWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjEllipsoidDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeneralPolygonHintReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjGeneralPolygonHintWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeneralPolygonHintDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeneralPolygonReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjGeneralPolygonWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGeneralPolygonDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLineReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjLineWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLineDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMarkerReader (MF3D_FilePtr inMetafilePtr,
			      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMarkerWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMarkerDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMeshWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMeshDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBCurveReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjNURBCurveWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBCurveDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBPatchReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjNURBPatchWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjNURBPatchDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPointReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPointWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPointDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPolygonReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPolygonWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPolygonDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPolyLineReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPolyLineWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPolyLineDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTorusReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTorusWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTorusDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTriangleReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTriangleWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTriangleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTriGridReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTriGridWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTriGridDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGroupReader (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjGroupWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDisplayGroupReader (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjDisplayGroupWriter (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDisplayGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjIOProxyDisplayGroupReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjIOProxyDisplayGroupWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjIOProxyDisplayGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrderedDisplayGroupReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjOrderedDisplayGroupWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrderedDisplayGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjInfoGroupReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjInfoGroupWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjInfoGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLightGroupReader (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjLightGroupWriter (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLightGroupDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAmbientLightReader (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjAmbientLightWriter (MF3D_FilePtr inMetafilePtr,
				    MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjAmbientLightDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDirectionalLightReader (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjDirectionalLightWriter (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjDirectionalLightDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPointLightReader (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPointLightWriter (MF3D_FilePtr inMetafilePtr,
				  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPointLightDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpotLightReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSpotLightWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSpotLightDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLambertIlluminationReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjLambertIlluminationWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjLambertIlluminationDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPhongIlluminationReader (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPhongIlluminationWriter (MF3D_FilePtr inMetafilePtr,
					 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPhongIlluminationDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTextureShaderReader (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTextureShaderWriter (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTextureShaderDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBackfacingStyleReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjBackfacingStyleWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjBackfacingStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFillStyleReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjFillStyleWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjFillStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjHighlightStyleReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjHighlightStyleWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjHighlightStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjInterpolationStyleReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjInterpolationStyleWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjInterpolationStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrientationStyleReader (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjOrientationStyleWriter (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjOrientationStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPickIDStyleReader (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPickIDStyleWriter (MF3D_FilePtr inMetafilePtr,
				   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPickIDStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPickPartsStyleReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPickPartsStyleWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPickPartsStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjReceiveShadowsStyleReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjReceiveShadowsStyleWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjReceiveShadowsStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSubdivisionStyleReader (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjSubdivisionStyleWriter (MF3D_FilePtr inMetafilePtr,
					MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjSubdivisionStyleDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMatrixTransformReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMatrixTransformWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMatrixTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjQuaternionTransformReader (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjQuaternionTransformWriter (MF3D_FilePtr inMetafilePtr,
					   MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjQuaternionTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateTransformReader (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRotateTransformWriter (MF3D_FilePtr inMetafilePtr,
				       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateAboutAxisTransformReader (MF3D_FilePtr inMetafilePtr,
						MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRotateAboutAxisTransformWriter (MF3D_FilePtr inMetafilePtr,
						MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateAboutAxisTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateAboutPointTransformReader (MF3D_FilePtr inMetafilePtr,
						 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjRotateAboutPointTransformWriter (MF3D_FilePtr inMetafilePtr,
						 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjRotateAboutPointTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjScaleTransformReader (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjScaleTransformWriter (MF3D_FilePtr inMetafilePtr,
				      MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjScaleTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTranslateTransformReader (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjTranslateTransformWriter (MF3D_FilePtr inMetafilePtr,
					  MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjTranslateTransformDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMacintoshPathReader (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjMacintoshPathWriter (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjMacintoshPathDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnixPathReader (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjUnixPathWriter (MF3D_FilePtr inMetafilePtr,
				MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnixPathDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCStringReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjCStringWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjCStringDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnicodeReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjUnicodeWriter (MF3D_FilePtr inMetafilePtr, MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnicodeDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPixmapTextureReader (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjPixmapTextureWriter (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjPixmapTextureDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewHintsReader (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjViewHintsWriter (MF3D_FilePtr inMetafilePtr,
				 MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjViewHintsDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnknownBinaryReader (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjUnknownBinaryWriter (MF3D_FilePtr inMetafilePtr,
				     MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnknownBinaryDisposer (MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnknownReader (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr * outObj);
MF3DErr MF3D_ObjUnknownWriter (MF3D_FilePtr inMetafilePtr,
			       MF3DVoidObjPtr inObj);
MF3DErr MF3D_ObjUnknownDisposer (MF3DVoidObjPtr inObj);

#endif
