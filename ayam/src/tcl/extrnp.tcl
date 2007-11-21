# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnp.tcl - ExtrNP objects Tcl code

set ExtrNP_props { Transformations Attributes Material Tags ExtrNPAttr }


array set ExtrNPAttr {
arr   ExtrNPAttrData
sproc ""
gproc ""
w     fExtrNPAttr

}

array set ExtrNPAttrData {
DisplayMode 1
UMin 0.0
UMin 1.0
VMin 0.0
VMin 1.0
}
# create ExtrNPAttr-UI
set w [frame $ay(pca).$ExtrNPAttr(w)]

addParam $w ExtrNPAttrData UMin
addParam $w ExtrNPAttrData UMax
addParam $w ExtrNPAttrData VMin
addParam $w ExtrNPAttrData VMax

addParam $w ExtrNPAttrData PatchNum

addParam $w ExtrNPAttrData Tolerance
addMenu $w ExtrNPAttrData DisplayMode $ay(npdisplaymodes)

addText $w ExtrNPAttrData "Extracted NURBS Patch:"
addInfo $w ExtrNPAttrData NPInfo
