# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail2.tcl - interpolating curves objects Tcl code

set Birail2_props { Transformations Attributes Material Tags Birail2Attr }


array set Birail2Attr {
arr   Birail2AttrData
sproc ""
gproc ""
w     fBirail2Attr

}

array set Birail2AttrData {
DisplayMode 1
}
# create Birail2Attr-UI
set w [frame $ay(pca).$Birail2Attr(w)]

#addCheck $w Birail2AttrData Close
addParam $w Birail2AttrData Sections
addCheck $w Birail2AttrData StartCap
addCheck $w Birail2AttrData EndCap
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w Birail2AttrData DisplayMode $l
addParam $w Birail2AttrData Tolerance


#birail2_crt:
#
#
proc birail2_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "birail2_crt" ""; return; }
    set ay_error 0
    crtOb Birail2
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
# birail2_crt