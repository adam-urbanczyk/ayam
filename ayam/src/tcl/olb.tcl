# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# olb.tcl - the object listbox


#
proc olb_open { w } {
global ay

set ay(lb) 1

frame $w.flb
set f $w.flb

# label
label $f.la -text "Objects:" -padx 0 -pady 0

# listbox
set ay(olb) $f.li
listbox $f.li -width 14 -height 10 -selectmode extended\
	-yscrollcommand {global ay; $ay(olbs) set} -exportselection 0

bind $f.li <Double-ButtonPress-1> {
    global ay
    set list [$ay(olb) curselection]
    eval [subst "goDown $list"]
    selOb
    olb_update
    rV
}

bind $f.li <Return> { olb_select }

bind $f.li <ButtonRelease-1> { after 150 olb_select }
    
bind $f.li <KeyRelease-space> { olb_select }

bind $f.li <ButtonPress-4> {
    global ay
    $ay(olb) yview scroll -1 pages
    break
}

bind $f.li <ButtonPress-5> {
    global ay
    $ay(olb) yview scroll 1 pages
    break
}

# olb context menu
set m [menu $ay(olb).popup -tearoff 0]
$m add command -label "Switch to Tree" -command\
 "  cS; uS;\
    olb_close $w;\
    tree_open $w;\
    tree_update root;\
    plb_update;\
    set ay(CurrentLevel) \"root\";\
    set ay(SelectedLevel) \"root\";\
    tree_paintLevel \"root\";\
    set ay(DropActive) 0;\
    rV"
$m add separator
$m add command -label "Copy Object" -command ".fu.fMenu.ed.m invoke 0"
$m add command -label "Cut Object" -command ".fu.fMenu.ed.m invoke 1"
$m add command -label "Paste Object" -command ".fu.fMenu.ed.m invoke 2"
#$m add command -label "Paste (Move)" -command "cmovOb;uS;rV"
$m add separator
$m add command -label "Delete Object" -command ".fu.fMenu.ed.m invoke 3"

bind $ay(olb) <ButtonPress-3> {
    global ay

    set xy [winfo pointerxy .]
    set x [lindex $xy 0]
    set y [lindex $xy 1]

    tk_popup $ay(olb).popup $x $y
    return;
}


# scrollbar
set ay(olbs) $f.s
scrollbar $f.s -command {global ay; $ay(olb) yview} -takefocus 0
pack $f.la -in $f -side top -fill x
pack $f.li -in $f -side left -fill both -expand yes
pack $f.s -in $f -side left -fill y -expand no

# switch from listbox to tree
bind $f.la <Double-1> "\
    cS; uS;\
    olb_close $w;\
    tree_open $w;\
    tree_update root;\
    plb_update;\
    set ay(CurrentLevel) \"root\";\
    set ay(SelectedLevel) \"root\";\
    tree_paintLevel \"root\";\
    set ay(DropActive) 0;\
    rV"

balloon_set $f.la "Double click here\nto switch to tree"

# buttons
frame $w.fb -highlightthickness 1

set f $w.fb
frame $f.f1
set f $f.f1
button $f.bnon -text "None"  -padx 0 -pady 0 -command {
    global ay
    $ay(olb) selection clear 0 end
    selOb
    plb_update
    rV
} -takefocus 0 -highlightthickness 0

button $f.bup -text "Up" -padx 0 -pady 0  -command {
    goUp
    selOb
    olb_update
    rV
} -takefocus 0 -highlightthickness 0
pack $f.bnon $f.bup -in $f -side top -fill x -expand yes

set f $w.fb
frame $f.f2
set f $f.f2

button $f.ball -text "All"   -padx 0 -pady 0 -command {
    global ay
    set lb $ay(olb)
    if { [$lb index end] > 1 } {
	$lb selection clear 0 end
	$lb selection set 1 end
	set selection [$lb curselection]
	eval [subst "selOb $selection"]
	olb_update
    }
    rV
} -takefocus 0 -highlightthickness 0

button $f.btop -text "Top" -padx 0 -pady 0  -command {
    goTop
    selOb
    olb_update
    rV
} -takefocus 0 -highlightthickness 0

pack $f.ball $f.btop -in $f -side top -fill x -expand yes

set f $w.fb
frame $f.f3
set f $f.f3

button $f.binv -text "Inv"  -padx 0 -pady 0 -command {
    global ay
    
    set selected [$ay(olb) curselection]
    $ay(olb) selection set 0 end
    foreach element $selected {
	$ay(olb) selection clear $element
    }
    olb_select
    rV
} -takefocus 0 -highlightthickness 0
    
button $f.bdwn -text "Down" -padx 0 -pady 0  -command {
    global ay
    set list [$ay(olb) curselection]
    eval [subst "goDown $list"]
    selOb
    olb_update
    rV
} -takefocus 0 -highlightthickness 0

pack $f.binv $f.bdwn -in $f -side top -fill x -expand yes

set f $w.fb
pack $f.f1 -in $f -side left -fill x -expand yes
pack $f.f2 -in $f -side left -fill x -expand yes
pack $f.f3 -in $f -side left -fill x -expand yes
pack $w.fb -in $w -side bottom -fill x
update
pack $w.flb -in $w -side left -fill both -expand yes

}
# olb_open


#
proc olb_close { w } {
global ay

set ay(lb) 0

destroy $w.flb
destroy $w.fb

}
# olb_close


#
proc olb_update { } {
    global ay curlevel curtypes

    set lb $ay(olb)

    set selobj [$lb curselection]

    update

    # delete all current entries
    $lb delete 0 end

    # get current objects
    getLevel curlevel curtypes

    eval [subst "$lb insert end $curlevel"]

    if { $selobj != "" } {
	foreach i $selobj {
	    $lb selection set $i
	}
	olb_select
    } else {
	plb_update
    }
}
# olb_update


# select from object listbox selection
proc olb_select { } {
    global ay

    undo savsel

    set lb $ay(olb)
    set selection [$lb curselection]
    
    eval [subst "selOb $selection"]

    plb_update

    rV
}
# olb_select
