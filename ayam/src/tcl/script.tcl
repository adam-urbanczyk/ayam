# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# script.tcl - script objects Tcl code

global ay Script_props ScriptAttr ScriptAttrData 

set Script_props { Attributes Tags ScriptAttr }

array set ScriptAttr {
arr   ScriptAttrData
sproc setScriptp
gproc getScriptp
w     fScriptAttr

}
# array ScriptAttr

array set ScriptAttrData {
 Type 0
 Script ""
}
# array ScriptAttrData

# create ScriptAttr-UI:
set w [frame $ay(pca).$ScriptAttr(w)]
addMenu $w ScriptAttrData Type [list Inactive Run Create Modify]
pack [text $w.tScript -width 60 -height 60]
set t $w.tScript
eval [subst "bindtags $t \{$t Text all\}"]
bind $t <Key-Escape> {resetFocus}

# create popup menu
set m [menu $t.popup -tearoff 0]
$m add command -label "Clear All" -command "$t delete 1.0 end"
$m add command -label "Paste (Replace)" -command {
    global ay ScriptAttr
    set t $ay(pca).${ScriptAttr(w)}.tScript
    set nt ""
    set nt [selection get]
    if { $nt != "" } {
	$t delete 1.0 end
	$t insert end $nt
    }
}
# bind popup menu
bind $t <3> {
    set xy [winfo pointerxy .];
    set x [lindex $xy 0]
    set y [lindex $xy 1]
    tk_popup $t.popup $x $y
}
# bind

# Tcl -> C
proc setScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript
    set ScriptAttrData(Script) [$t get 1.0 end]
    setProp

 return;
}
# setScriptp

# C -> Tcl
proc getScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript
    getProp
    $t delete 1.0 end
    $t insert 1.0 $ScriptAttrData(Script)

 return;
}
# getScriptp
global SCRIPTPLUGIN
if { [info exists SCRIPTPLUGIN] } {
# add menu entry to Create/Custom sub-menu
mmenu_addcustom Script "crtOb Script;uS;sL;"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) Script
}
return;