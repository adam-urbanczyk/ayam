# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# concatnc.tcl - concatnc objects Tcl code

set ConcatNC_props { Transformations Attributes Tags ConcatNCAttr }


array set ConcatNCAttr {
arr   ConcatNCAttrData
sproc ""
gproc ""
w     fConcatNCAttr

}

# create ConcatNCAttr-UI
set w [frame $ay(pca).$ConcatNCAttr(w)]

addCheck $w ConcatNCAttrData Closed
addCheck $w ConcatNCAttrData Mode

array set ConcatNCAttrData {

}

#concatnc_crt:
#
#
proc concatnc_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb ConcatNC
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
# concatnc_crt