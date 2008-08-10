# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnc.tcl - ExtrNC objects Tcl code

set ExtrNC_props { Transformations Attributes Material Tags ExtrNCAttr }


array set ExtrNCAttr {
arr   ExtrNCAttrData
sproc ""
gproc ""
w     fExtrNCAttr

}

array set ExtrNCAttrData {
Side 1
DisplayMode 1
NCInfoBall "N/A"
}
# create ExtrNCAttr-UI
set w [frame $ay(pca).$ExtrNCAttr(w)]



addMenu $w ExtrNCAttrData Side\
    [list U0 Un V0 Vn U V Boundary Middle_U Middle_V]
addParam $w ExtrNCAttrData Parameter
addCheck $w ExtrNCAttrData Relative
addParam $w ExtrNCAttrData PatchNum
addCheck $w ExtrNCAttrData Revert

addParam $w ExtrNCAttrData Tolerance
addMenu $w ExtrNCAttrData DisplayMode $ay(ncdisplaymodes)

addText $w ExtrNCAttrData "Extracted NURBS Curve:"
addInfo $w ExtrNCAttrData NCInfo
