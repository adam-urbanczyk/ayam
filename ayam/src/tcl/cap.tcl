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
Mode 1
DisplayMode 0
}
# create CapAttr-UI
set w [frame $ay(pca).$CapAttr(w)]

addParam $w CapAttrData Tolerance
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w CapAttrData DisplayMode $l

#cap_crt:
#
#
proc cap_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Cap
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
    uS; sL; rV;

 return;
}
# cap_crt