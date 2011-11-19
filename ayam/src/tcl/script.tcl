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

# create ScriptAttr-UI:
set w [frame $ay(pca).$ScriptAttr(w)]
addCheck $w ScriptAttrData Active
addMenu $w ScriptAttrData Type [list Run Create Modify]
pack [text $w.tScript -width 60 -height 60]
set t $w.tScript
eval [subst "bindtags $t \{$t Text all\}"]
bind $t <Key-Escape> "resetFocus;break"

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

    set types {{"Script Files" {".tcl" ".js"}} {"All files" *}}
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
    if { [lsearch $tags errtag] != -1 } {
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
    if { [lsearch $tags errtag] != -1 } {
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
forAllT script 1 {
    getScriptp
    set ::ScriptAttrData(Active) 1
    setScriptp
}

# activate script tags by renaming all DBNS/DANS tags to BNS/ANS tags
forAll 1 {
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