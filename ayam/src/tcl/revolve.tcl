# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# revolve.tcl - interpolating curves objects Tcl code

set Revolve_props { Transformations Attributes Material Tags RevolveAttr }


array set RevolveAttr {
arr   RevolveAttrData
sproc ""
gproc ""
w     fRevolveAttr

}

array set RevolveAttrData {
DisplayMode 1
}
# create RevolveAttr-UI
set w [frame $ay(pca).$RevolveAttr(w)]

addParam $w RevolveAttrData ThetaMax
addParam $w RevolveAttrData Sections
addParam $w RevolveAttrData Order
addCheck $w RevolveAttrData UpperCap
addCheck $w RevolveAttrData LowerCap
addCheck $w RevolveAttrData StartCap
addCheck $w RevolveAttrData EndCap
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w RevolveAttrData DisplayMode $l
addParam $w RevolveAttrData Tolerance


#revolve_crt:
#
#
proc revolve_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Revolve
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
# revolve_crt