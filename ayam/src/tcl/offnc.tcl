# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# offnc.tcl - offset curves objects Tcl code

set OffsetNC_props { Transformations Attributes Tags OffsetNCAttr }

array set OffsetNCAttr {
arr   OffsetNCAttrData
sproc ""
gproc ""
w     fOffsetNCAttr
}

array set OffsetNCAttrData {
DisplayMode 0
NCInfoBall "N/A"
}
# create UI
set w [frame $ay(pca).$OffsetNCAttr(w)]

addCheck $w OffsetNCAttrData Revert
addParam $w OffsetNCAttrData Offset

addParam $w OffsetNCAttrData Tolerance
addMenu $w OffsetNCAttrData DisplayMode $ay(ncdisplaymodes)

addText $w OffsetNCAttrData "Created NURBS Curve:"
addInfo $w OffsetNCAttrData NCInfo


# offnc_crt:
#
#
proc offnc_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "offnc_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb OffsetNC
    if { $ay_error } { return; }

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
# offnc_crt
