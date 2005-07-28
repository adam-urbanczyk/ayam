# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# gordon.tcl - interpolating curves objects Tcl code

set Gordon_props { Transformations Attributes Material Tags GordonAttr }


array set GordonAttr {
arr   GordonAttrData
sproc ""
gproc ""
w     fGordonAttr

}

array set GordonAttrData {
DisplayMode 1
}
# create GordonAttr-UI
set w [frame $ay(pca).$GordonAttr(w)]

addCheck $w GordonAttrData WatchCurves
addParam $w GordonAttrData Order_U
addParam $w GordonAttrData Order_V

global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w GordonAttrData DisplayMode $l
addParam $w GordonAttrData Tolerance


#gordon_crt:
#
#
proc gordon_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "gordon_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb Gordon
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
    uS
    sL
    rV

 return;
}
# gordon_crt
