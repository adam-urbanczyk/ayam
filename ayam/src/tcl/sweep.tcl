# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sweep.tcl - interpolating curves objects Tcl code

set Sweep_props { Transformations Attributes Material Tags SweepAttr }


array set SweepAttr {
arr   SweepAttrData
sproc ""
gproc ""
w     fSweepAttr

}

array set SweepAttrData {
DisplayMode 1
}
# create SweepAttr-UI
set w [frame $ay(pca).$SweepAttr(w)]


addCheck $w SweepAttrData Rotate
addCheck $w SweepAttrData Interpolate
addCheck $w SweepAttrData Close
addParam $w SweepAttrData Sections
addCheck $w SweepAttrData StartCap
addCheck $w SweepAttrData EndCap
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w SweepAttrData DisplayMode $l
addParam $w SweepAttrData Tolerance


#sweep_crt:
#
#
proc sweep_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Sweep
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
# sweep_crt