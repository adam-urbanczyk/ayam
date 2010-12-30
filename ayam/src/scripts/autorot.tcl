# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2010 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# autorot.tcl - procedures to automatically rotate a view (showcase mode)

# autorot:
#  rotate all views from ::arparam(views) a single step (dx/dy)
#  arrange for next rotation in dt ms
#  stop autorotating according to arparam(run)
proc autorot { dt dx dy } {
    global arparam

    foreach v $arparam(views) {

	set togl $v.f3D.togl

	$togl setconf -drotx $dx -droty $dy
    }
    if $arparam(run) {
	after $dt "autorot $dt $dx $dy"
    }
 return;
}
# autorot


# autorot_open:
#  open the autorot GUI
#
proc autorot_open { } {
    global ay arparam

    array set arparam {
	DX 2.5
	DY 0.0
	DT 100
	run 0
    }

    set w .arw
    catch {destroy $w}
    toplevel $w -class Ayam
    wm title $w "AutoRotate"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }
    wm withdraw $w

    set f [frame $w.f1]

    scrollbar $f.sc -command {.arw.f1.lb yview} -takefocus 0
    listbox $f.lb -width 40 -height 5 -selectmode multiple\
	    -exportselection 0 -yscrollcommand {.arw.f1.sc set}
    set arparam(lb) $f.lb

    foreach view $ay(views) {
	if { [winfo toplevel $view] == $view } {
	    scan $view ".%s" name
	} else {
	    set last [string last . $view]
	    set name [string range $view [expr $last + 2] end]
	}

	$f.lb insert end "$name"
	lappend arparam(allviews) $view
    }
    pack $f -in $w -side top -fill x

    pack $f.lb -in $f -side left -fill both
    pack $f.sc -in $f -side right -fill y
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f3.bd
    set f [frame $w.f2]
    addParam $f arparam DT [list 50 100 150 200]
    addParam $f arparam DX [list 0.5 1 2.5 5]
    addParam $f arparam DY [list 0.5 1 2.5 5]

    button $f.ss -text "Start/Stop" -command {
	set arparam(views) ""
	foreach view [$arparam(lb) curselection] {
	    lappend arparam(views) [lindex $arparam(allviews) $view]
	}
	if $arparam(run) {
	    set arparam(run) 0
	} else {
	    set arparam(run) 1
	    autorot $arparam(DT) $arparam(DX) $arparam(DY)
	}
    }
    # button

    pack $f.ss -in $f -side bottom -fill x
    pack $f -in $w -side top -fill x

    set f [frame $w.f3]
    button $f.bd -text "Dismiss" -command {
	focus .; destroy .arw;
    }
    # button

    pack $f.bd -in $f -fill x
    pack $f -in $w -side bottom -fill x

    bind $w <Escape> "$f.bd invoke"

    winCenter $w

 return;
}
# autorot_open


autorot_open

