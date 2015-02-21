# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# script.tcl - script objects Tcl code

set Script 1

proc init_Script { } {
global ay ayprefs Script_props ScriptAttr ScriptAttrData

set Script_props { Attributes Tags ScriptAttr }

array set ScriptAttr {
arr   ScriptAttrData
sproc setScriptp
gproc getScriptp
w     fScriptAttr

}
# array ScriptAttr

array set ScriptAttrData {
 Active 0
 Type 0
 Script ""
}
# array ScriptAttrData

proc resizeHandle:ButtonPress-1 {win resizeWin X Y} {
    upvar #0 _resizeHandle$win ar
    set ar(startX) $X
    set ar(startY) $Y
    set ar(oldWidth) [$resizeWin cget -width]
    set ar(oldHeight) [$resizeWin cget -height]
}

proc resizeHandle:B1-Motion {win resizeWin X Y} {
    upvar #0 _resizeHandle$win ar
    set xDiff [expr {$X - $ar(startX)}]
    set yDiff [expr {$Y - $ar(startY)}]

    set newWidth [expr {$ar(oldWidth) + $xDiff / $ar(cw)}]
    set newHeight [expr {$ar(oldHeight) + $yDiff / $ar(ch)}]

    if {$newWidth < 2 || $newHeight < 2} {
	return
    }
    if { $newWidth != $ar(oldWidth) || $newHeight != $ar(oldHeight) } {
	$resizeWin conf -width $newWidth -height $newHeight
	after idle "resizeHandle:PlaceHandle $win $resizeWin"
    }
}

proc resizeHandle:PlaceHandle {win resizeWin} {
    set x [expr [winfo x $resizeWin]+[winfo width $resizeWin]-3]
    set y [expr [winfo y $resizeWin]+[winfo height $resizeWin]-3]

    place $win -in [winfo parent $win] -anchor se -x $x -y $y
}

proc resizeHandle:Destroy {win} {
    upvar #0 _resizeHandle$win ar
    # catch, because ar may not be set
    catch {array unset ar}
}

proc resizeHandle:Create {win resizeWin args} {
    upvar #0 _resizeHandle$win ar
    eval label [concat $win $args -image ay_Resizehandle_img]
    resizeHandle:PlaceHandle $win $resizeWin
    bind $win <ButtonPress-1> \
	"resizeHandle:ButtonPress-1 $win $resizeWin %X %Y"
    bind $win <B1-Motion> \
	"resizeHandle:B1-Motion $win $resizeWin %X %Y"
    bind $win <ButtonRelease-1> \
	"resizeHandle:PlaceHandle $win $resizeWin"
    bind $win <Destroy> "resizeHandle:Destroy $win"

    set font [$win cget -font]
    set ar(cw) [expr [font measure $font "A"]]
    set ar(ch) [font metrics $font -linespace]

    return $win
}

# create ScriptAttr-UI:
set w [frame $ay(pca).$ScriptAttr(w)]
addVSpace $w s1 2
addCheck $w ScriptAttrData Active
addMenu $w ScriptAttrData Type [list Run Create Modify]
pack [text $w.tScript -undo 1 -width 60 -height 60]
set t $w.tScript
eval [subst "bindtags $t \{$t Text all\}"]
bind $t <Key-Escape> "resetFocus;break"

# create resize handle for text widget
if { $ay(ws) == "Win32" } {
    resizeHandle:Create $w.rsh $t -bg SystemWindow
} else {
    resizeHandle:Create $w.rsh $t
}

# create popup menu
set m [menu $t.popup -tearoff 0]
$m add command -label "Clear All" -command "$t delete 1.0 end"
$m add command -label "Paste (Replace)" -command {
    global ay ScriptAttr
    set t $ay(pca).${ScriptAttr(w)}.tScript
    set nt ""
    set nt [selection get -selection CLIPBOARD]
    if { $nt != "" } {
	$t delete 1.0 end
	$t insert end $nt
    }
}
$m add command -label "Load from file" -command {
    global ay ScriptAttr
    set t $ay(pca).${ScriptAttr(w)}.tScript
    set nt ""

    set types {{"Script Files" {".tcl" ".js" ".lua"}} {"All files" *}}
    set newfilename ""
    set newfilename [tk_getOpenFile -filetypes $types -parent .\
		-title "Select file to load:"]
    if { $newfilename != "" } {
	set scfile ""
	set scfile [open $newfilename r]
	if { $scfile != "" } {
	    set nt [read $scfile]
	    if { $nt != "" } {
		$t delete 1.0 end
		$t insert end $nt
	    }
	}
    }
}
# bind popup menu
set mb 3
if { $ay(ws) == "Aqua" && $ayprefs(SwapMB) } {
    set mb 2
}
bind $t <$mb> {
    global ay ScriptAttr
    set t $ay(pca).${ScriptAttr(w)}.tScript
    set xy [winfo pointerxy .]
    set x [lindex $xy 0]
    set y [lindex $xy 1]
    tk_popup $t.popup $x $y
}
# bind

return;
}
# init_Script


# Tcl -> C
proc setScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript

    set tags [$t tag names]
    if { [lsearch -exact $tags errtag] != -1 } {
	$t tag delete errtag
    }

    set ScriptAttrData(Script) [$t get 1.0 end]
    setProp

}
# setScriptp

# C -> Tcl
proc getScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript
    getProp
    set errrange ""
    set tags [$t tag names]
    if { [lsearch -exact $tags errtag] != -1 } {
	set errrange [$t tag range errtag]
    }
    $t delete 1.0 end
    $t insert 1.0 $ScriptAttrData(Script)
    if { $ScriptAttrData(Script) != "" } {
	$t delete "end - 1 chars"
    }

    if { $errrange != "" } {
	eval [subst "$t tag add errtag $errrange"]
    }

    after 100 "resizeHandle:PlaceHandle $ay(pca).$ScriptAttr(w).rsh \
      $ay(pca).$ScriptAttr(w).tScript"

 return;
}
# getScriptp

# script_disable:
#  ask user for permission to disable all script objects loaded
#  with a scene file
proc script_disable { } {
    global ay ayprefs

    if { $ayprefs(AskScriptDisable) && !$ay(askedscriptdisable) } {
	set t "Disable Scripts?"
	set m "Scene contains Script objects or tags, disable them?"

	if { $ayprefs(FixDialogTitles) == 1 } {
	    set m "$t\n\n$m"
	}

	set answer [tk_messageBox -title $t -type yesno -icon warning -message $m]
	if { $answer == "yes" } {
	    set ay(scriptdisable) 1
	} else {
	    set ay(scriptdisable) 0
	}
	set ay(askedscriptdisable) 1
    }

return;
}
# script_disable

# enable all script tags and objects
proc script_enable { } {
global ay ayprefs

goTop
selOb
cS
plb_update

# activate all script object scripts
forAll -type script {
    getScriptp
    set ::ScriptAttrData(Active) 1
    setScriptp
}

# activate script tags by renaming all DBNS/DANS tags to BNS/ANS tags
forAll {
    set tagnames ""; set tagvals ""
    getTags tagnames tagvals
    if { $tagnames != "" } {
	set tagindex 0
	foreach tagname $tagnames {
	    if { $tagname == "DBNS" } {
		setTags -index $tagindex "BNS" [lindex $tagvals $tagindex]
	    }
	    if { $tagname == "DANS" } {
		setTags -index $tagindex "ANS" [lindex $tagvals $tagindex]
	    }
	    incr tagindex
	}
    }
}

goTop
selOb
notifyOb

return;
}
# script_enable

return;