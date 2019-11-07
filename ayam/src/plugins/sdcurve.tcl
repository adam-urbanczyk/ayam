# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2017 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdcurve.tcl - SDcurve objects Tcl code

global ay SDCurve_props SDCurveAttr SDCurveAttrData

set SDCurve_props { Transformations Attributes Tags SDCurveAttr }

array set SDCurveAttr {
    arr   SDCurveAttrData
    sproc ""
    gproc ""
    w     fSDCurveAttr
}

array set SDCurveAttrData {
    Type 0
    SLength "n/a"
}

set ay(bok) $ay(appb)

# create SDCurveAttr-UI
set w [frame $ay(pca).$SDCurveAttr(w)]
set a $SDCurveAttr(arr)

addVSpace $w s1 2
addCheck $w $a Closed
addParam $w $a Length
addMenu $w $a Type [list Chaikin Cubic]
addParam $w $a Level
addText $w $a "Subdivision:"
addInfo $w $a SLength

# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SDCurve "crtOb SDCurve;uCR;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SDCurve


# sdcurve_conv:
#  convert to SDCurve
#
proc sdcurve_conv { } {
    global ay ay_error sdcurve_convopt aymainshortcuts

    winAutoFocusOff

    set sdcurve_convopt(oldfocus) [focus]

    set w .sdc
    set t "Convert To SDCurve"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .sdc.f2.bca
    set ay(bok) .sdc.f2.bok

    #addParam $f sdcurve_convopt ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]
    addCheck $f sdcurve_convopt ApplyTrafo

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global sdcurve_convopt
	sdcconvertC -a $sdcurve_convopt(ApplyTrafo)
	grab release .sdc
	restoreFocus $sdcurve_convopt(oldfocus)
	destroy .sdc
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .sdc;\
		restoreFocus $sdcurve_convopt(oldfocus);\
		destroy .sdc"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # context help
    #shortcut_addcshelp $w ayam-7.html impply

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# sdcurve_conv

global ay
$ay(cm) add cascade -menu $ay(cm).sdc -label "SDCurve"
menu $ay(cm).sdc -tearoff 0
set m $ay(cm).sdc
$m add command -label "From Curve" -command {sdcurve_conv; uCR; rV;}

# EOF
