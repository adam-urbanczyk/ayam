# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail1.tcl - interpolating curves objects Tcl code

set Birail1_props { Transformations Attributes Material Tags Birail1Attr }


array set Birail1Attr {
arr   Birail1AttrData
sproc ""
gproc ""
w     fBirail1Attr

}

array set Birail1AttrData {
DisplayMode 1
}
# create Birail1Attr-UI
set w [frame $ay(pca).$Birail1Attr(w)]

addCheck $w Birail1AttrData Close
addParam $w Birail1AttrData Sections
addCheck $w Birail1AttrData StartCap
addCheck $w Birail1AttrData EndCap
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w Birail1AttrData DisplayMode $l
addParam $w Birail1AttrData Tolerance


#birail1_crt:
#
#
proc birail1_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Birail1
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
# birail1_crt