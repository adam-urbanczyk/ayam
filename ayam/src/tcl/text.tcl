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
sproc "textSetProp"
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

proc textSetProp { } {
    setProp
    update
    forceNot
    rV
}