# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# text.tcl - text objects Tcl code

set Text_props { Transformations Attributes Material Tags TextAttr }


array set TextAttr {
arr   TextAttrData
sproc ""
gproc ""
w     fTextAttr

}

array set TextAttrData {
DisplayMode 1
BevelType 0
}
# create TextAttr-UI
set w [frame $ay(pca).$TextAttr(w)]

addFile $w TextAttrData FontName
addString $w TextAttrData String
addParam $w TextAttrData Height
addCheck $w TextAttrData Revert
addCheck $w TextAttrData UpperCap
addCheck $w TextAttrData LowerCap
addCheck $w TextAttrData UpperBevels
addCheck $w TextAttrData LowerBevels
addMenu $w TextAttrData BevelType [list Round Linear Ridge]
addParam $w TextAttrData BevelRadius
addCheck $w TextAttrData RevertBevels

global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w TextAttrData DisplayMode $l
addParam $w TextAttrData Tolerance
