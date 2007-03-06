# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# cap.tcl - cap objects Tcl code

set Cap_props { Transformations Attributes Material Tags CapAttr }


array set CapAttr {
arr   CapAttrData
sproc ""
gproc ""
w     fCapAttr

}

array set CapAttrData {
Type 1
DisplayMode 0
}
# create CapAttr-UI
set w [frame $ay(pca).$CapAttr(w)]

addMenu $w CapAttrData Type {Trim Gordon}
addParam $w CapAttrData Tolerance
addMenu $w CapAttrData DisplayMode $ay(npdisplaymodes)


#cap_crt:
#
#
proc cap_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "cap_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb Cap
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
# cap_crt
