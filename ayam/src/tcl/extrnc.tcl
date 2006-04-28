# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
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
}
# create ExtrNCAttr-UI
set w [frame $ay(pca).$ExtrNCAttr(w)]



addMenu $w ExtrNCAttrData Side [list U0 Un V0 Vn U V]
addParam $w ExtrNCAttrData Parameter
addParam $w ExtrNCAttrData PatchNum

addParam $w ExtrNCAttrData Tolerance
addMenu $w ExtrNCAttrData DisplayMode $ay(ncdisplaymodes)



#extrnc_crt:
#
#
proc extrnc_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "extrnc_crt" ""; return; }
    set ay_error 0
    crtOb ExtrNC
    if { $ay_error } {  return; }

    cutOb
    set ay(ul) $ay(CurrentLevel)
    uS
    sL
    getLevel a b
    goDown [expr [llength $a]-1]
    cmovOb
    goUp
    set ay(ul) $ay(CurrentLevel)
    uS; sL; forceNot; rV;

 return;
}
# extrnc_crt
