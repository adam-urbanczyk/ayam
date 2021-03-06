#ifndef	MF3D_OBJECTTYPE_H
#define	MF3D_OBJECTTYPE_H
/*==============================================================================
 *
 *	File:		MFOBJTYP.H
 *
 *	Function:	Object name/type lookup
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
 *		F3A_RWW	MeshEdges and OrientationStyle.
 *		F2S_RWW	BeginGroup
 *		F2R_RWW	Change to simple object theory.
 *		F2H_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include <stddef.h>

#include "mfobjcts.h"
#include "mfobject.h"

typedef struct MF3D_ObjStuff
  {
    MF3DObjType type;
    MF3DCStringPtr name;
      MF3DErr (*reader) (MF3D_FilePtr inMetafile,
			 MF3DVoidObjPtr * outObj);
      MF3DErr (*writer) (MF3D_FilePtr inMetafile,
			 MF3DVoidObjPtr inObj);
      MF3DErr (*disposer) (MF3DVoidObjPtr inObj);
  }
MF3D_ObjStuff, *MF3D_ObjStuffPtr;

/*==============================================================================
 *	Object Data and Method Tables
 *==============================================================================
 */
#ifndef MF3D_Extern
extern MF3D_ObjStuff gMF3D_ObjStuffTable[];
#else
MF3D_ObjStuff gMF3D_ObjStuffTable[] =
{kMF3DObjMetafile,
 kMF3DObjMetafileText,
 MF3D_ObjMetafileReader,
 MF3D_ObjMetafileWriter,
 MF3D_ObjMetafileDisposer,
 kMF3DObjContainer,
 kMF3DObjContainerText,
 MF3D_ObjContainerReader,
 MF3D_ObjContainerWriter,
 MF3D_ObjContainerDisposer,
 kMF3DObjEndContainer,
 NULL,
 NULL,
 MF3D_ObjEndContainerWriter,
 MF3D_ObjEndContainerDisposer,
 kMF3DObjBeginGroup,
 kMF3DObjBeginGroupText,
 MF3D_ObjBeginGroupReader,
 MF3D_ObjBeginGroupWriter,
 MF3D_ObjBeginGroupDisposer,
 kMF3DObjEndGroup,
 kMF3DObjEndGroupText,
 MF3D_ObjEndGroupReader,
 MF3D_ObjEndGroupWriter,
 MF3D_ObjEndGroupDisposer,
 kMF3DObjReference,
 kMF3DObjReferenceText,
 MF3D_ObjReferenceReader,
 MF3D_ObjReferenceWriter,
 MF3D_ObjReferenceDisposer,
 kMF3DObjTableOfContents,
 kMF3DObjTableOfContentsText,
 MF3D_ObjTableOfContentsReader,
 MF3D_ObjTableOfContentsWriter,
 MF3D_ObjTableOfContentsDisposer,
 kMF3DObjType,
 kMF3DObjTypeText,
 MF3D_ObjTypeReader,
 MF3D_ObjTypeWriter,
 MF3D_ObjTypeDisposer,
 kMF3DObjFaceAttributeSetList,
 kMF3DObjFaceAttributeSetListText,
 MF3D_ObjFaceAttributeSetListReader,
 MF3D_ObjFaceAttributeSetListWriter,
 MF3D_ObjFaceAttributeSetListDisposer,
 kMF3DObjGeometryAttributeSetList,
 kMF3DObjGeometryAttributeSetListText,
 MF3D_ObjGeometryAttributeSetListReader,
 MF3D_ObjGeometryAttributeSetListWriter,
 MF3D_ObjGeometryAttributeSetListDisposer,
 kMF3DObjVertexAttributeSetList,
 kMF3DObjVertexAttributeSetListText,
 MF3D_ObjVertexAttributeSetListReader,
 MF3D_ObjVertexAttributeSetListWriter,
 MF3D_ObjVertexAttributeSetListDisposer,
 kMF3DObjCameraPlacement,
 kMF3DObjCameraPlacementText,
 MF3D_ObjCameraPlacementReader,
 MF3D_ObjCameraPlacementWriter,
 MF3D_ObjCameraPlacementDisposer,
 kMF3DObjCameraRange,
 kMF3DObjCameraRangeText,
 MF3D_ObjCameraRangeReader,
 MF3D_ObjCameraRangeWriter,
 MF3D_ObjCameraRangeDisposer,
 kMF3DObjCameraViewPort,
 kMF3DObjCameraViewPortText,
 MF3D_ObjCameraViewPortReader,
 MF3D_ObjCameraViewPortWriter,
 MF3D_ObjCameraViewPortDisposer,
 kMF3DObjBottomCapAttributeSet,
 kMF3DObjBottomCapAttributeSetText,
 MF3D_ObjBottomCapAttributeSetReader,
 MF3D_ObjBottomCapAttributeSetWriter,
 MF3D_ObjBottomCapAttributeSetDisposer,
 kMF3DObjCaps,
 kMF3DObjCapsText,
 MF3D_ObjCapsReader,
 MF3D_ObjCapsWriter,
 MF3D_ObjCapsDisposer,
 kMF3DObjFaceCapAttributeSet,
 kMF3DObjFaceCapAttributeSetText,
 MF3D_ObjFaceCapAttributeSetReader,
 MF3D_ObjFaceCapAttributeSetWriter,
 MF3D_ObjFaceCapAttributeSetDisposer,
 kMF3DObjTopCapAttributeSet,
 kMF3DObjTopCapAttributeSetText,
 MF3D_ObjTopCapAttributeSetReader,
 MF3D_ObjTopCapAttributeSetWriter,
 MF3D_ObjTopCapAttributeSetDisposer,
 kMF3DObjDisplayGroupState,
 kMF3DObjDisplayGroupStateText,
 MF3D_ObjDisplayGroupStateReader,
 MF3D_ObjDisplayGroupStateWriter,
 MF3D_ObjDisplayGroupStateDisposer,
 kMF3DObjLightData,
 kMF3DObjLightDataText,
 MF3D_ObjLightDataReader,
 MF3D_ObjLightDataWriter,
 MF3D_ObjLightDataDisposer,
 kMF3DObjMeshCorners,
 kMF3DObjMeshCornersText,
 MF3D_ObjMeshCornersReader,
 MF3D_ObjMeshCornersWriter,
 MF3D_ObjMeshCornersDisposer,
 kMF3DObjMeshEdges,
 kMF3DObjMeshEdgesText,
 MF3D_ObjMeshEdgesReader,
 MF3D_ObjMeshEdgesWriter,
 MF3D_ObjMeshEdgesDisposer,
 kMF3DObjNURBCurve2D,
 kMF3DObjNURBCurve2DText,
 MF3D_ObjNURBCurve2DReader,
 MF3D_ObjNURBCurve2DWriter,
 MF3D_ObjNURBCurve2DDisposer,
 kMF3DObjShaderData,
 kMF3DObjShaderDataText,
 MF3D_ObjShaderDataReader,
 MF3D_ObjShaderDataWriter,
 MF3D_ObjShaderDataDisposer,
 kMF3DObjShaderTransform,
 kMF3DObjShaderTransformText,
 MF3D_ObjShaderTransformReader,
 MF3D_ObjShaderTransformWriter,
 MF3D_ObjShaderTransformDisposer,
 kMF3DObjShaderUVTransform,
 kMF3DObjShaderUVTransformText,
 MF3D_ObjShaderUVTransformReader,
 MF3D_ObjShaderUVTransformWriter,
 MF3D_ObjShaderUVTransformDisposer,
 kMF3DObjTrimCurves,
 kMF3DObjTrimCurvesText,
 MF3D_ObjTrimCurvesReader,
 MF3D_ObjTrimCurvesWriter,
 MF3D_ObjTrimCurvesDisposer,
 kMF3DObjImageClearColor,
 kMF3DObjImageClearColorText,
 MF3D_ObjImageClearColorReader,
 MF3D_ObjImageClearColorWriter,
 MF3D_ObjImageClearColorDisposer,
 kMF3DObjImageDimensions,
 kMF3DObjImageDimensionsText,
 MF3D_ObjImageDimensionsReader,
 MF3D_ObjImageDimensionsWriter,
 MF3D_ObjImageDimensionsDisposer,
 kMF3DObjImageMask,
 kMF3DObjImageMaskText,
 MF3D_ObjImageMaskReader,
 MF3D_ObjImageMaskWriter,
 MF3D_ObjImageMaskDisposer,
 kMF3DObjAmbientCoefficient,
 kMF3DObjAmbientCoefficientText,
 MF3D_ObjAmbientCoefficientReader,
 MF3D_ObjAmbientCoefficientWriter,
 MF3D_ObjAmbientCoefficientDisposer,
 kMF3DObjDiffuseColor,
 kMF3DObjDiffuseColorText,
 MF3D_ObjDiffuseColorReader,
 MF3D_ObjDiffuseColorWriter,
 MF3D_ObjDiffuseColorDisposer,
 kMF3DObjHighlightState,
 kMF3DObjHighlightStateText,
 MF3D_ObjHighlightStateReader,
 MF3D_ObjHighlightStateWriter,
 MF3D_ObjHighlightStateDisposer,
 kMF3DObjNormal,
 kMF3DObjNormalText,
 MF3D_ObjNormalReader,
 MF3D_ObjNormalWriter,
 MF3D_ObjNormalDisposer,
 kMF3DObjShadingUV,
 kMF3DObjShadingUVText,
 MF3D_ObjShadingUVReader,
 MF3D_ObjShadingUVWriter,
 MF3D_ObjShadingUVDisposer,
 kMF3DObjSpecularColor,
 kMF3DObjSpecularColorText,
 MF3D_ObjSpecularColorReader,
 MF3D_ObjSpecularColorWriter,
 MF3D_ObjSpecularColorDisposer,
 kMF3DObjSpecularControl,
 kMF3DObjSpecularControlText,
 MF3D_ObjSpecularControlReader,
 MF3D_ObjSpecularControlWriter,
 MF3D_ObjSpecularControlDisposer,
 kMF3DObjSurfaceTangent,
 kMF3DObjSurfaceTangentText,
 MF3D_ObjSurfaceTangentReader,
 MF3D_ObjSurfaceTangentWriter,
 MF3D_ObjSurfaceTangentDisposer,
 kMF3DObjSurfaceUV,
 kMF3DObjSurfaceUVText,
 MF3D_ObjSurfaceUVReader,
 MF3D_ObjSurfaceUVWriter,
 MF3D_ObjSurfaceUVDisposer,
 kMF3DObjTransparencyColor,
 kMF3DObjTransparencyColorText,
 MF3D_ObjTransparencyColorReader,
 MF3D_ObjTransparencyColorWriter,
 MF3D_ObjTransparencyColorDisposer,
 kMF3DObjRendererInteractive,
 kMF3DObjRendererInteractiveText,
 MF3D_ObjRendererInteractiveReader,
 MF3D_ObjRendererInteractiveWriter,
 MF3D_ObjRendererInteractiveDisposer,
 kMF3DObjRendererGeneric,
 kMF3DObjRendererGenericText,
 MF3D_ObjRendererGenericReader,
 MF3D_ObjRendererGenericWriter,
 MF3D_ObjRendererGenericDisposer,
 kMF3DObjRendererWireFrame,
 kMF3DObjRendererWireFrameText,
 MF3D_ObjRendererWireFrameReader,
 MF3D_ObjRendererWireFrameWriter,
 MF3D_ObjRendererWireFrameDisposer,
 kMF3DObjRendererZBuffer,
 kMF3DObjRendererZBufferText,
 MF3D_ObjRendererZBufferReader,
 MF3D_ObjRendererZBufferWriter,
 MF3D_ObjRendererZBufferDisposer,
 kMF3DObjAttributeSet,
 kMF3DObjAttributeSetText,
 MF3D_ObjAttributeSetReader,
 MF3D_ObjAttributeSetWriter,
 MF3D_ObjAttributeSetDisposer,
 kMF3DObjOrthographicCamera,
 kMF3DObjOrthographicCameraText,
 MF3D_ObjOrthographicCameraReader,
 MF3D_ObjOrthographicCameraWriter,
 MF3D_ObjOrthographicCameraDisposer,
 kMF3DObjViewAngleAspectCamera,
 kMF3DObjViewAngleAspectCameraText,
 MF3D_ObjViewAngleAspectCameraReader,
 MF3D_ObjViewAngleAspectCameraWriter,
 MF3D_ObjViewAngleAspectCameraDisposer,
 kMF3DObjViewPlaneCamera,
 kMF3DObjViewPlaneCameraText,
 MF3D_ObjViewPlaneCameraReader,
 MF3D_ObjViewPlaneCameraWriter,
 MF3D_ObjViewPlaneCameraDisposer,
 kMF3DObjBox,
 kMF3DObjBoxText,
 MF3D_ObjBoxReader,
 MF3D_ObjBoxWriter,
 MF3D_ObjBoxDisposer,
 kMF3DObjCone,
 kMF3DObjConeText,
 MF3D_ObjConeReader,
 MF3D_ObjConeWriter,
 MF3D_ObjConeDisposer,
 kMF3DObjCylinder,
 kMF3DObjCylinderText,
 MF3D_ObjCylinderReader,
 MF3D_ObjCylinderWriter,
 MF3D_ObjCylinderDisposer,
 kMF3DObjDisk,
 kMF3DObjDiskText,
 MF3D_ObjDiskReader,
 MF3D_ObjDiskWriter,
 MF3D_ObjDiskDisposer,
 kMF3DObjEllipse,
 kMF3DObjEllipseText,
 MF3D_ObjEllipseReader,
 MF3D_ObjEllipseWriter,
 MF3D_ObjEllipseDisposer,
 kMF3DObjEllipsoid,
 kMF3DObjEllipsoidText,
 MF3D_ObjEllipsoidReader,
 MF3D_ObjEllipsoidWriter,
 MF3D_ObjEllipsoidDisposer,
 kMF3DObjGeneralPolygon,
 kMF3DObjGeneralPolygonText,
 MF3D_ObjGeneralPolygonReader,
 MF3D_ObjGeneralPolygonWriter,
 MF3D_ObjGeneralPolygonDisposer,
 kMF3DObjLine,
 kMF3DObjLineText,
 MF3D_ObjLineReader,
 MF3D_ObjLineWriter,
 MF3D_ObjLineDisposer,
 kMF3DObjMarker,
 kMF3DObjMarkerText,
 MF3D_ObjMarkerReader,
 MF3D_ObjMarkerWriter,
 MF3D_ObjMarkerDisposer,
 kMF3DObjMesh,
 kMF3DObjMeshText,
 MF3D_ObjMeshReader,
 MF3D_ObjMeshWriter,
 MF3D_ObjMeshDisposer,
 kMF3DObjNURBCurve,
 kMF3DObjNURBCurveText,
 MF3D_ObjNURBCurveReader,
 MF3D_ObjNURBCurveWriter,
 MF3D_ObjNURBCurveDisposer,
 kMF3DObjNURBPatch,
 kMF3DObjNURBPatchText,
 MF3D_ObjNURBPatchReader,
 MF3D_ObjNURBPatchWriter,
 MF3D_ObjNURBPatchDisposer,
 kMF3DObjPoint,
 kMF3DObjPointText,
 MF3D_ObjPointReader,
 MF3D_ObjPointWriter,
 MF3D_ObjPointDisposer,
 kMF3DObjPolygon,
 kMF3DObjPolygonText,
 MF3D_ObjPolygonReader,
 MF3D_ObjPolygonWriter,
 MF3D_ObjPolygonDisposer,
 kMF3DObjPolyLine,
 kMF3DObjPolyLineText,
 MF3D_ObjPolyLineReader,
 MF3D_ObjPolyLineWriter,
 MF3D_ObjPolyLineDisposer,
 kMF3DObjTorus,
 kMF3DObjTorusText,
 MF3D_ObjTorusReader,
 MF3D_ObjTorusWriter,
 MF3D_ObjTorusDisposer,
 kMF3DObjTriangle,
 kMF3DObjTriangleText,
 MF3D_ObjTriangleReader,
 MF3D_ObjTriangleWriter,
 MF3D_ObjTriangleDisposer,
 kMF3DObjTriGrid,
 kMF3DObjTriGridText,
 MF3D_ObjTriGridReader,
 MF3D_ObjTriGridWriter,
 MF3D_ObjTriGridDisposer,
 kMF3DObjGroup,
 kMF3DObjGroupText,
 MF3D_ObjGroupReader,
 MF3D_ObjGroupWriter,
 MF3D_ObjGroupDisposer,
 kMF3DObjDisplayGroup,
 kMF3DObjDisplayGroupText,
 MF3D_ObjDisplayGroupReader,
 MF3D_ObjDisplayGroupWriter,
 MF3D_ObjDisplayGroupDisposer,
 kMF3DObjIOProxyDisplayGroup,
 kMF3DObjIOProxyDisplayGroupText,
 MF3D_ObjIOProxyDisplayGroupReader,
 MF3D_ObjIOProxyDisplayGroupWriter,
 MF3D_ObjIOProxyDisplayGroupDisposer,
 kMF3DObjOrderedDisplayGroup,
 kMF3DObjOrderedDisplayGroupText,
 MF3D_ObjOrderedDisplayGroupReader,
 MF3D_ObjOrderedDisplayGroupWriter,
 MF3D_ObjOrderedDisplayGroupDisposer,
 kMF3DObjInfoGroup,
 kMF3DObjInfoGroupText,
 MF3D_ObjInfoGroupReader,
 MF3D_ObjInfoGroupWriter,
 MF3D_ObjInfoGroupDisposer,
 kMF3DObjLightGroup,
 kMF3DObjLightGroupText,
 MF3D_ObjLightGroupReader,
 MF3D_ObjLightGroupWriter,
 MF3D_ObjLightGroupDisposer,
 kMF3DObjAmbientLight,
 kMF3DObjAmbientLightText,
 MF3D_ObjAmbientLightReader,
 MF3D_ObjAmbientLightWriter,
 MF3D_ObjAmbientLightDisposer,
 kMF3DObjDirectionalLight,
 kMF3DObjDirectionalLightText,
 MF3D_ObjDirectionalLightReader,
 MF3D_ObjDirectionalLightWriter,
 MF3D_ObjDirectionalLightDisposer,
 kMF3DObjPointLight,
 kMF3DObjPointLightText,
 MF3D_ObjPointLightReader,
 MF3D_ObjPointLightWriter,
 MF3D_ObjPointLightDisposer,
 kMF3DObjSpotLight,
 kMF3DObjSpotLightText,
 MF3D_ObjSpotLightReader,
 MF3D_ObjSpotLightWriter,
 MF3D_ObjSpotLightDisposer,
 kMF3DObjLambertIllumination,
 kMF3DObjLambertIlluminationText,
 MF3D_ObjLambertIlluminationReader,
 MF3D_ObjLambertIlluminationWriter,
 MF3D_ObjLambertIlluminationDisposer,
 kMF3DObjPhongIllumination,
 kMF3DObjPhongIlluminationText,
 MF3D_ObjPhongIlluminationReader,
 MF3D_ObjPhongIlluminationWriter,
 MF3D_ObjPhongIlluminationDisposer,
 kMF3DObjTextureShader,
 kMF3DObjTextureShaderText,
 MF3D_ObjTextureShaderReader,
 MF3D_ObjTextureShaderWriter,
 MF3D_ObjTextureShaderDisposer,
 kMF3DObjBackfacingStyle,
 kMF3DObjBackfacingStyleText,
 MF3D_ObjBackfacingStyleReader,
 MF3D_ObjBackfacingStyleWriter,
 MF3D_ObjBackfacingStyleDisposer,
 kMF3DObjFillStyle,
 kMF3DObjFillStyleText,
 MF3D_ObjFillStyleReader,
 MF3D_ObjFillStyleWriter,
 MF3D_ObjFillStyleDisposer,
 kMF3DObjHighlightStyle,
 kMF3DObjHighlightStyleText,
 MF3D_ObjHighlightStyleReader,
 MF3D_ObjHighlightStyleWriter,
 MF3D_ObjHighlightStyleDisposer,
 kMF3DObjInterpolationStyle,
 kMF3DObjInterpolationStyleText,
 MF3D_ObjInterpolationStyleReader,
 MF3D_ObjInterpolationStyleWriter,
 MF3D_ObjInterpolationStyleDisposer,
 kMF3DObjOrientationStyle,
 kMF3DObjOrientationStyleText,
 MF3D_ObjOrientationStyleReader,
 MF3D_ObjOrientationStyleWriter,
 MF3D_ObjOrientationStyleDisposer,
 kMF3DObjPickIDStyle,
 kMF3DObjPickIDStyleText,
 MF3D_ObjPickIDStyleReader,
 MF3D_ObjPickIDStyleWriter,
 MF3D_ObjPickIDStyleDisposer,
 kMF3DObjPickPartsStyle,
 kMF3DObjPickPartsStyleText,
 MF3D_ObjPickPartsStyleReader,
 MF3D_ObjPickPartsStyleWriter,
 MF3D_ObjPickPartsStyleDisposer,
 kMF3DObjReceiveShadowsStyle,
 kMF3DObjReceiveShadowsStyleText,
 MF3D_ObjReceiveShadowsStyleReader,
 MF3D_ObjReceiveShadowsStyleWriter,
 MF3D_ObjReceiveShadowsStyleDisposer,
 kMF3DObjSubdivisionStyle,
 kMF3DObjSubdivisionStyleText,
 MF3D_ObjSubdivisionStyleReader,
 MF3D_ObjSubdivisionStyleWriter,
 MF3D_ObjSubdivisionStyleDisposer,
 kMF3DObjMatrixTransform,
 kMF3DObjMatrixTransformText,
 MF3D_ObjMatrixTransformReader,
 MF3D_ObjMatrixTransformWriter,
 MF3D_ObjMatrixTransformDisposer,
 kMF3DObjQuaternionTransform,
 kMF3DObjQuaternionTransformText,
 MF3D_ObjQuaternionTransformReader,
 MF3D_ObjQuaternionTransformWriter,
 MF3D_ObjQuaternionTransformDisposer,
 kMF3DObjRotateTransform,
 kMF3DObjRotateTransformText,
 MF3D_ObjRotateTransformReader,
 MF3D_ObjRotateTransformWriter,
 MF3D_ObjRotateTransformDisposer,
 kMF3DObjRotateAboutAxisTransform,
 kMF3DObjRotateAboutAxisTransformText,
 MF3D_ObjRotateAboutAxisTransformReader,
 MF3D_ObjRotateAboutAxisTransformWriter,
 MF3D_ObjRotateAboutAxisTransformDisposer,
 kMF3DObjRotateAboutPointTransform,
 kMF3DObjRotateAboutPointTransformText,
 MF3D_ObjRotateAboutPointTransformReader,
 MF3D_ObjRotateAboutPointTransformWriter,
 MF3D_ObjRotateAboutPointTransformDisposer,
 kMF3DObjScaleTransform,
 kMF3DObjScaleTransformText,
 MF3D_ObjScaleTransformReader,
 MF3D_ObjScaleTransformWriter,
 MF3D_ObjScaleTransformDisposer,
 kMF3DObjTranslateTransform,
 kMF3DObjTranslateTransformText,
 MF3D_ObjTranslateTransformReader,
 MF3D_ObjTranslateTransformWriter,
 MF3D_ObjTranslateTransformDisposer,
 kMF3DObjMacintoshPath,
 kMF3DObjMacintoshPathText,
 MF3D_ObjMacintoshPathReader,
 MF3D_ObjMacintoshPathWriter,
 MF3D_ObjMacintoshPathDisposer,
 kMF3DObjUnixPath,
 kMF3DObjUnixPathText,
 MF3D_ObjUnixPathReader,
 MF3D_ObjUnixPathWriter,
 MF3D_ObjUnixPathDisposer,
 kMF3DObjCString,
 kMF3DObjCStringText,
 MF3D_ObjCStringReader,
 MF3D_ObjCStringWriter,
 MF3D_ObjCStringDisposer,
 kMF3DObjUnicode,
 kMF3DObjUnicodeText,
 MF3D_ObjUnicodeReader,
 MF3D_ObjUnicodeWriter,
 MF3D_ObjUnicodeDisposer,
 kMF3DObjPixmapTexture,
 kMF3DObjPixmapTextureText,
 MF3D_ObjPixmapTextureReader,
 MF3D_ObjPixmapTextureWriter,
 MF3D_ObjPixmapTextureDisposer,
 kMF3DObjViewHints,
 kMF3DObjViewHintsText,
 MF3D_ObjViewHintsReader,
 MF3D_ObjViewHintsWriter,
 MF3D_ObjViewHintsDisposer,
 kMF3DObjUnknownBinary,		/* FB3_JRK */
 kMF3DObjUnknownBinaryText,
 MF3D_ObjUnknownBinaryReader,
 MF3D_ObjUnknownBinaryWriter,
 MF3D_ObjUnknownBinaryDisposer,
 kMF3DObjMetafileSwapped,	/* F3R_PAM allow swapped binary files */
 kMF3DObjMetafileText,
 MF3D_ObjMetafileReader,
 MF3D_ObjMetafileWriter,
 MF3D_ObjMetafileDisposer,
 kMF3DObjInvalid,
 NULL,
 NULL,
 NULL,
 NULL
};
#endif /* MF3D_Extern */

#ifndef MF3D_Extern
extern MF3D_ObjStuff gMF3D_UnknownObjStuffTable;
#else
MF3D_ObjStuff gMF3D_UnknownObjStuffTable =
{kMF3DObjUnknownType,
 NULL,
 MF3D_ObjUnknownReader,
 MF3D_ObjUnknownWriter,
 MF3D_ObjUnknownDisposer,
};
#endif /* MF3D_Extern */

MF3DErr MF3D_FindObjectFromName (
				  const char *inObjName,
				  MF3D_ObjStuffPtr * outObjStuffPtr,
				  MF3DObjType * outObjectType);

MF3DErr MF3D_FindObjectFromType (
				  MF3DObjType inObjType,
				  MF3D_ObjStuffPtr * outObjStuffPtr);

MF3DErr MF3D_ConvertUserDefinedObjType (
					 MF3D_FilePtr inMetafilePtr,
					 MF3DObjType inObjType,
					 MF3DCStringPtr * outObjName);

#endif
