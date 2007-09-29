# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# swing.tcl - Swing objects Tcl code

set Swing_props { Transformations Attributes Material Tags SwingAttr }


array set SwingAttr {
arr   SwingAttrData
sproc ""
gproc ""
w     fSwingAttr

}

array set SwingAttrData {
DisplayMode 1
NPInfoBall "n/a"
}
# create SwingAttr-UI
set w [frame $ay(pca).$SwingAttr(w)]

addCheck $w SwingAttrData UpperCap
addCheck $w SwingAttrData LowerCap
addCheck $w SwingAttrData StartCap
addCheck $w SwingAttrData EndCap

addParam $w SwingAttrData Tolerance
addMenu $w SwingAttrData DisplayMode $ay(npdisplaymodes)

addText $w SwingAttrData "Created NURBS Patch:"
addInfo $w SwingAttrData NPInfo

#swing_crt:
#
#
proc swing_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "swing_crt" ""; return; }
    set ay_error 0

    # the next command sorts the selected objects
    eval "selOb $selected"

    crtOb Swing
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
# swing_crt