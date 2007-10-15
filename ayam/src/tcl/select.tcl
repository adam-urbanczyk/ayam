# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# select.tcl - select objects Tcl code

set Select_props { Transformations Attributes Material Tags SelectAttr }


array set SelectAttr {
arr   SelectAttrData
sproc ""
gproc ""
w     fSelectAttr

}

array set SelectAttrData {
Index 0
}

# create SelectAttr-UI
set w [frame $ay(pca).$SelectAttr(w)]
addParam$w SelectAttrData Index

# select_crt:
#
#
proc select_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "select_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb Select
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
# select_crt
