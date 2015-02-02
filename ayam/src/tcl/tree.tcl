# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# tree.tcl - the object hierarchy tree widget

if { $AYWRAPPED != 1 } {
    package require -exact BWidget 1.2.1
}

set ay(tree) ""
#set ay(ObjectBar) ""
#set ay(NoteBook) ""

set ay(droplock) 0
set ay(CurrentLevel) ""
set ay(SelectedLevel) ""

# for drag and drop within the tree
set ay(DndDestination) ""

#tree_openTree:
# open all nodes pointing to <node>
proc tree_openTree { tree node } {

    regsub -all ":" $node " " nodes
    eval "set b [list $nodes]"

    set c [lrange $b 1 end]
    set node "root:"
    foreach elem $c {
	append node $elem

	if { [$tree exists $node] } {
	    $tree itemconfigure $node -open 1
	}
	append node ":"
    }

 return;
}
# tree_openTree


#tree_createSub:
# set up tree content
proc tree_createSub { tree node l } {

    set x 0
    set y -1
    foreach n $l {
	set ll [llength $n]
	set inode [lindex $n 0]
	if { $x > 0 } {
	    $tree finsert $node $node:$y -text $inode -drawcross auto\
		-image emptybm
	}
	if { $ll > 1 } {
	    tree_createSub $tree $node:$y $n
	}
        incr x
	incr y
    }
    # foreach

 return;
}
# tree_createSub


#tree_blockUI:
# block the tree user interface (active when a tree update takes longer)
proc tree_blockUI { } {
    grab .fl
    . configure -cursor watch
    update
 return;
}
# tree_blockUI


#tree_update:
# This procedure recreates the subtree pointed to by node.
# treeGetString is a C-command witch returns the subtree
# content as a list of lists of lists... string.
# This string is recursivly traversed by tree_createSub
# The nodes are named this way: root:<index in root level>[:<index2>[: ... ]]
proc tree_update { node } {
    global ay
    if { $ay(treelock) == 1 } {
	# inform the currently running update process, that we have some
	# more changes to the scene to consider
	set ay(ExtraTreeUpdate) 1
	return
    } else {
	set ay(treelock) 1
    }

    # update may take some time, take measures:
    # after 0.1s we block the UI and make the
    # mouse cursor a watch
    after 100 tree_blockUI

    treeGetString curtree $node

    # redraw AFTER recreation, not while (can see building process)
    $ay(tree) configure -redraw 0
    $ay(tree) delete [$ay(tree) nodes $node]

    # insert tree items
    if { $node == $ay(CurrentLevel) } {
	set color "black"
    } else {
	set color "darkgrey"
    }
    set count 0
    foreach n $curtree {
	set ll [llength $n]
	set inode [lindex $n 0]
	$ay(tree) finsert $node $node:$count -text $inode -drawcross auto\
	    -image emptybm -fill $color
	if { $ll > 1 } {
	    tree_createSub $ay(tree) $node:$count $n
	}
        incr count
    }

    $ay(tree) configure -redraw 1

    # unblock UI
    after cancel tree_blockUI
    if { [grab current] == ".fl" } {
	grab release .fl
    }
    . configure -cursor {}

    set ay(treelock) 0

    if { $ay(ExtraTreeUpdate) == 1 } {
	set ay(ExtraTreeUpdate) 0
	ayError 1 tree_update "Doing an extra update for node \"$node\"."
	update
	tree_update $node
    }

 return;
}
# tree_update


#tree_paintLevel:
# paint the level designated by <level> in black
# paint the current level in darkgrey
# should be called when the current level changes,
# but _before_ ay(CurrentLevel) is set
# does not change highlighted items
proc tree_paintLevel { level } {
    global ay

    if { $ay(CurrentLevel) != "" } {
	set nlist [$ay(tree) nodes $ay(CurrentLevel)]
	foreach n $nlist {
	    set col [$ay(tree) itemcget $n -fill]
	    if { $col == "black" } {
		$ay(tree) itemconfigure $n -fill darkgrey
	    }
	}
    }

    set nlist [$ay(tree) nodes $level]
    foreach n $nlist {
	set col [$ay(tree) itemcget $n -fill]
	if { $col == "darkgrey" } {
	    $ay(tree) itemconfigure $n -fill black
	}
    }

 return;
}
# tree_paintLevel


#tree_paintTree:
# recursively paint the current selected level black
# paint the rest in darkgrey
# also resets highlights
proc tree_paintTree { level } {
    global ay

    set nodes [$ay(tree) nodes $level]

    if { $level == $ay(CurrentLevel) } {
	foreach node $nodes {
	    set col [$ay(tree) itemcget $node -fill]
	    if { $col != "black" } {
		$ay(tree) itemconfigure $node -fill black
	    }
	}
    } else {
	foreach node $nodes {
	    set col [$ay(tree) itemcget $node -fill]
	    if { $col != "darkgrey" } {
		$ay(tree) itemconfigure $node -fill darkgrey
	    }
	}
    }

    foreach node $nodes {
	tree_paintTree $node
    }

 return;
}
# tree_paintTree


#tree_selectOrPopup
#
proc tree_selectOrPopup { extend redraw tree node } {
 global ay

    set nlist [$tree selection get]
    if { [llength $nlist] == 0 } {
	tree_selectItem $redraw $tree $node
    } else {
	if { $extend } {
	    tree_multipleSelection $tree $node
	} else {
	    if {[lsearch $nlist $node] == -1 } {
		tree_selectItem $redraw $tree $node
	    }
	}
    }
    winOpenPopup $ay(tree)
    after idle {focus -force $ay(tree).popup}

 return;
}
# tree_selectOrPopup


#tree_selectItem
# select one tree item; clicking an already selected object does nothing
proc tree_selectItem { redraw tree node } {
    global ay

    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }

    set ay(ts) 1;
    set nlist [$tree selection get]
    $ay(tree) selection clear
    $ay(tree) selection set $node
    if { [lsearch -exact $nlist $node] == "-1" } {
	$tree selection set $node
	set ay(SelectedLevel) [$tree parent $node]
	$tree selection set $node
    }
    tree_handleSelection
    $tree bindText <ButtonRelease-1> ""
    $tree bindText <ButtonPress-1> "tree_selectItem 1 $tree"
    update
    plb_update
    update idletasks
    if { $redraw == 1 } {
	if { $ay(need_redraw) == 1 } {
	    rV
	}
    }

    set ay(sellock) 0

 return;
}
# tree_selectItem


#tree_toggleSelection
# multiple selection via ctrl-key; only allowed within on level
proc tree_toggleSelection { tree node } {
    global ay

    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }

    set ay(ts) 1;
    set SelectedLevel $ay(SelectedLevel)

    set nlist [$tree selection get]
    if { [lsearch -exact $nlist $node] != -1 } {
	if { [llength $nlist] > "1" } {
	    $tree selection remove $node
	}
    } else {
	if { [llength $nlist] != "0" } {
	    set parent [$tree parent $node]
	    if { $parent == $SelectedLevel } {

		lappend nlist $node
		set newsel [lsort $nlist]
		eval [subst "$tree selection set $newsel"]
	    } else {
	ayError 1 "toggleSelection" "Can not select from different levels!"
	    }
	    # if parent
	} else {
	    $tree selection add $node
	}
	# if llength nlist
    }
    # if lsearch

    # allow dnd of multiple selected objects
    $tree bindText <ButtonPress-1> ""
    $tree bindText <ButtonRelease-1> "tree_selectItem 1 $tree"

    tree_handleSelection

    plb_update

    if { $ay(need_redraw) == 1 } {
	rV
    }

    set ay(sellock) 0

 return;
}
# tree_toggleSelection


#tree_multipleSelection
# multiple selection via shift-key; only allowed within one level
proc tree_multipleSelection { tree node } {
    global ay

    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }
    while { 1 } {
    set ay(ts) 1;
    set SelectedLevel $ay(SelectedLevel)

    set nlist [$tree selection get]

    if { [lsearch -exact $nlist $node] != -1 } {
	ayError 1 "multipleSelection" "Select a different object!"
	break;
    }

    set parent [$tree parent $node]
    if { $parent != $SelectedLevel } {
	ayError 1 "multipleSelection" "Can not select from different levels!"
	break;
    }

    set index1 [$tree index [lindex $nlist 0]]
    set index2 [$tree index $node]

    if { $index1 < $index2 } {
	set selnodes [lsort [$tree nodes $parent $index1 $index2]]
    } else {
	if { [llength $nlist] > 1 } {
	    set index1 [$tree index [lindex $nlist end]]
	}
	set selnodes [lsort [$tree nodes $parent $index2 $index1]]
    }

    eval [subst "$tree selection set $selnodes"]

    tree_handleSelection

    plb_update

    if { $ay(need_redraw) == 1 } {
	rV
    }

    # allow dnd of multiple selected objects
    $tree bindText <ButtonPress-1> ""
    $tree bindText <ButtonRelease-1> "tree_selectItem 1 $tree"

    break;
    }
    set ay(sellock) 0

 return;
}
# tree_multipleSelection


#tree_selectLast
# select last object of current level in tree
proc tree_selectLast { } {
    global ay

    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }

    # ay(ts) is triggered, whenever a true selection
    # occurs in the tree, if not (ay(ts) is 0) we
    # select the last element of the current level
    if { $ay(ts) == 0 } {
	sL
	if { $ay(need_redraw) == 1 } { rV }
    }
    set ay(ts) 0

    set ay(sellock) 0

 return;
}
# tree_selectLast


#tree_handleSelection:
# do any stuff for the selected objects such as highligting the selected level
proc tree_handleSelection { } {
    global ay

    if { $ay(SelectedLevel) != $ay(CurrentLevel) } {
	tree_paintLevel $ay(SelectedLevel)
	set ay(CurrentLevel) $ay(SelectedLevel)
    }
    set nlist [$ay(tree) selection get]
    eval [subst "treeSelect $nlist"]

 return;
}
# tree_handleSelection


#tree_drop:
# user dropped object in the tree view
proc tree_drop { tree from droppos currentoperation datatype data } {
    global ay

    # is another drop action already active?
    # this semaphor avoids being called multiple times
    # in case the user moves the mouse shortly after
    # release of the mouse button (Bug in BWidgets?)
    if { $ay(droplock) == 0 } {
	set ay(droplock) 1

	set pos [lindex $droppos 0]
	set parent "root"
	set position -1
	set selection ""
	set selection [$ay(tree) selection get]

	if { $selection == "" } {
	    set ay(droplock) 0
	    return;
	}
	if { $pos == "node" } {
	    set parent [lindex $droppos 1]
	    set position -1
	}
	if { $pos == "position" } {
	    set parent [lindex $droppos 1]
	    set position [lindex $droppos 2]
	}
	# reject dropping of objects onto root
	if { $parent == "root:0" } {
	    if { $selection != "root:0" } {
		ayError 2 tree_drop "Can not place objects here!"
	    }
	    set ay(droplock) 0
	    return;
	}
	# reject dropping of objects just before root (replacing
	# root as first object in the top level)
	if { $parent == "root" && $position == "0" } {
	    ayError 2 tree_drop "Can not place objects here!"
	    set ay(droplock) 0
	    return;
	}

	if { $from == $tree } {
	    # drag and drop within the tree
	    set err 0
	    # prevent moving of the root object
	    if { $data == "root:0" } {
		ayError 2 tree_drop "Can not move root!"
		set ay(droplock) 0
		return;
	    }
	    # prevent moving of objects into their own childs
	    while { $parent != "root" } {
		if { [lsearch -exact $selection $parent] != "-1" } {
		    ayError 2 tree_drop "Can not place objects here!"
		    set err 1
		}
		if { [$ay(tree) exists $parent] } {
		    set parent [$ay(tree) parent $parent]
		}
	    }
	    if { $err == "0" } {
		# everything seems to be ok, move the node
		set ay(DndDestination) $droppos
		tree_move
	    }
	    set ay(droplock) 0
	} else {
	    # XXXX unused
	    # create object via objectbar
	    set ay(droplock) 1
	    set newnode ""
	    set i [string first "img" $data ]
	    set otype [string range $data 0 [expr $i-1]]
	    puts $otype
	    crtOb $otype
	    sL
	    set ay(DndDestination) $position
#	    tree_move
#	    set
#	    CreateDndObject $data $parent $position $selection newnode
#	    puts $data
	    if { $newnode == "" } {
		ayError 1 tree_drop "Unable to create object!"
	    } else {
		tree_update $parent
		if { $parent != "root" } {
		    $ay(tree) itemconfigure $parent -open 1
		}
		#tree_selectItem 1 $ay(tree) $newnode
	    }
	    set ay(droplock) 0
	}
    } else {
	# XXXX eliminate this warning, it appears too often?
	#ayError 1 tree_drop "Drop action already active!"
    }
 return;
}
# tree_drop


#tree_openSub:
# open/close subtree in tree view
proc tree_openSub { tree newstate node } {
    global ay
    set ay(ts) 1;
    $tree itemconfigure $node -open $newstate
    $tree configure -redraw 1
 return;
}
# tree_openSub


#tree_toggleSub:
# toggle open/close state of a tree node
proc tree_toggleSub { tree node } {

    if { [$tree itemcget $node -open] == 1 } {
	tree_openSub $tree 0 $node
    } else {
	tree_openSub $tree 1 $node
    }

 return;
}
# tree_toggleSub


#tree_move:
# move a tree node (via DnD)
proc tree_move { } {
    global ay ay_error

    set old_selection [$ay(tree) selection get]
    set pos [lindex $ay(DndDestination) 0]
    set parent "root"
    set position -1
    if { $pos == "node" } {
	set parent [lindex $ay(DndDestination) 1]
	set position -1
    }
    if { $pos == "position" } {
	set parent [lindex $ay(DndDestination) 1]
	set position [lindex $ay(DndDestination) 2]
    }

    set ay_error 0
    set nodrop 0
    set newclevel ""

    treeDnd $parent $position nodrop newclevel

    if { $ay_error == 0 } {

	set ay(SelectedLevel) "root"

	set ay(sc) 1

	if { ! $nodrop } {
	    if { $newclevel == "" } {
		set newclevel "root"
	    }
	    set oldclevel $ay(CurrentLevel)
	    set ay(CurrentLevel) $newclevel
	    set ay(SelectedLevel) $newclevel
	    if { [string eq $newclevel $oldclevel] } {
		# both levels are the same, just need one update
		tree_update $oldclevel
	    } else {
		# levels are different, but we might just need one update,
		# if e.g. oldlevel is root: and newlevel is root:1
		# or, oldlevel is root:1 and newlevel is root:
		if { [string first $newclevel $oldclevel] != 0 } {
		    # newlevel does not include oldlevel => update oldlevel
		    tree_update $oldclevel
		}
		if { [string first $oldclevel $newclevel] != 0 } {
		    # oldlevel does not include newlevel => update newlevel
		    after idle "update;tree_update $newclevel"
		}
	    }
	    update
	    tree_openTree $ay(tree) $newclevel
	    $ay(tree) selection clear
	    tree_handleSelection

	    set sel ""
	    set nodes [$ay(tree) nodes $newclevel]
	    if { $position == -1 } {
		if { [llength $old_selection] == 1 } {
		    set sel [lindex $nodes end]
		} else {
		    set off [expr [llength $old_selection] - 1]
		    set sel [lrange $nodes end-$off end]
		}
	    } else {
		if { [llength $old_selection] == 1 } {
		    set sel [lindex $nodes $position]
		} else {
		    set off [expr $position + [llength $old_selection] - 1]
		    set sel [lrange $nodes $position $off]
		}
	    }
	    if { $sel != "" } {
		eval "$ay(tree) selection set $sel"
		tree_handleSelection
		set node [lindex $sel 0]
		update
		$ay(tree) see $node
	    }
	    notifyOb -all
	    plb_update
	    rV
	} else {
	    # treeDnd returned "no-drop"; restore old selection
	    $ay(tree) selection set $old_selection
	    set level [$ay(tree) parent [lindex $old_selection 0]]
	    set ay(CurrentLevel) $level
	    set ay(SelectedLevel) $level
	    tree_handleSelection
	    notifyOb -all
	    plb_update
	    rV
	}
	# if

    } else {
	# an error occured in treeDnd; restore old selection
	$ay(tree) selection set $old_selection
	set level [$ay(tree) parent [lindex $old_selection 0]]
	set ay(CurrentLevel) $level
	set ay(SelectedLevel) $level
	tree_handleSelection
	plb_update
    }
    # if

 return;
}
# tree_move


#tree_expand:
# open all nodes
proc tree_expand { } {
    global ay
    set nlist [$ay(tree) nodes root]
    foreach n $nlist {
	$ay(tree) opentree $n
    }
 return;
}
# tree_expand


#tree_collapse:
# close all nodes
proc tree_collapse { } {
    global ay
    set tree $ay(tree)
    set nlist [$tree nodes root]
    foreach n $nlist {
	$tree closetree $n
    }
    # show selection or current level (again)
    set sel ""
    set sel [$tree selection get]

    if { ($sel == "") && ($ay(CurrentLevel) != "root") } {
	set sel ${ay(CurrentLevel)}:0
    }

    if { $sel != "" } {
	set fsel [lindex $sel 0]
	set index 5
	set done 0
	while { ! $done } {
	    set index [string first : $fsel $index]
	    if { $index == -1 ||
		 ([string first : $fsel [expr $index]] == -1) } {
		set done 1
	    } else {
		incr index -1
		set item [string range $fsel 0 $index]
		incr index 2
		tree_openSub $tree 1 $item
	    }
	    # if
	}
	# while
	$tree see $sel
    }
    # if

 return;
}
# tree_collapse


# tree_open:
# create object hierarchy tree widget
proc tree_open { w } {
global ay ayprefs AYWITHAQUA

set ay(lb) 0
# frame
set fr [frame $w.ftr]
set f $fr

# label
set la [label $f.la -text "Objects:" -padx 0 -pady 0]

# a scrolled window
set sw [ScrolledWindow $f.sw -bd 2 -relief sunken]

image create bitmap emptybm

# the tree widget
set width 12

set tree [Tree $sw.tree -width $width -height 15\
	-relief flat -borderwidth 0\
	-highlightthickness 0\
	-redraw 0\
	-padx 1\
	-opencmd   "tree_openSub $sw.tree 1" \
	-closecmd  "tree_openSub $sw.tree 0" \
	-dragenabled true \
	-dropenabled true \
	-droptypes {TREE_NODE {copy {}} IMAGE { copy {}} } \
	-dropovermode pn\
	-dropcmd "tree_drop" ]
# XXXX was:
#	-droptypes {TREE_NODE {copy {}} IMAGE { copy {}} }

# work around a "bug" in Tk 8.4.15 MacOSX/Aqua
if { $ay(ws) == "Aqua" } {
    $tree configure -selectforeground systemHighlightText
}

$sw setwidget $tree

# remember path to the tree widget
set ay(tree) $tree

# bindings

# scroll tree with wheel
bind $tree <ButtonPress-4> "$tree yview scroll -1 pages; break"
bind $tree <ButtonPress-5> "$tree yview scroll 1 pages; break"

global tcl_platform AYWITHAQUA
if { ($tcl_platform(platform) == "windows") } {
    bind $tree <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(tree) yview scroll 1 pages
	} else {
	    $ay(tree) yview scroll -1 pages
	}
	break
    }
    # bind
}
# if

if { $AYWITHAQUA } {
    bind $tree <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(tree) yview scroll 3 units
	} else {
	    $ay(tree) yview scroll -3 units
	}
	break
    }
    # bind
}
# if

catch {bind $tree <Key-Page_Up> "$tree yview scroll -1 pages; break"}
catch {bind $tree <Key-Prior> "$tree yview scroll -1 pages; break"}
catch {bind $tree <Key-Page_Down> "$tree yview scroll 1 pages; break"}
catch {bind $tree <Key-Next> "$tree yview scroll 1 pages; break"}

# select last object, when clicking somewhere in the widget
# and not on a node
bind $tree <ButtonRelease-1> "+\
 global ay;\
 if { \$ay(sellock) == 0 } {\
 after 10 { tree_selectLast }; };\
 focus \$ay(tree)"

# toggle selection without affecting selection of other objects
$tree bindText <Control-ButtonPress-1> "tree_toggleSelection $sw.tree"
$tree bindText <Control-Double-ButtonPress-1> "tree_toggleSelection $sw.tree"

# multiple selection (regions)
$tree bindText <Shift-ButtonPress-1> "tree_multipleSelection $sw.tree"

# the next bindings consume unwanted events that would
# make the tree_selectItem-Binding fire, because tree_toggleSelection
# and tree_multipleSelection _change_ the binding for tree_selectItem
# from ButtonPress to ButtonRelease, to allow DnD of multiple selected
# objects, a single tree_selectItem re-establishes the normal
# binding to ButtonPress-1 then
$tree bindText <Shift-ButtonRelease-1> "set dummy "
$tree bindText <Control-ButtonRelease-1> "set dummy "

# select a single object
$tree bindText <ButtonPress-1> "tree_selectItem 1 $sw.tree"
# open sub tree
$tree bindText <Double-ButtonPress-1> "tree_toggleSub $sw.tree"

global aymainshortcuts
$tree bindText <ButtonPress-$aymainshortcuts(CMButton)>\
    "tree_selectOrPopup 0 1 $sw.tree"
$tree bindText <Shift-ButtonPress-$aymainshortcuts(CMButton)>\
    "tree_selectOrPopup 1 1 $sw.tree"

# focus management binding
if { !$::ayprefs(SingleWindow) } {
    bind $tree <Key-Tab> "focus .fl.con.console;break"
} else {
    bind $tree <Key-Tab> "focus .fu.fMain.fview3;break"
}

# arrange for switching back to good old listbox
bind $la <Double-ButtonPress-1> tree_toggle
set ay(treel) $la
balloon_set $la [ms tree_label]

# pack widgets
pack $fr -side top -expand yes -fill both
pack $la -side top -fill x -expand no

if { $AYWITHAQUA } {
    pack $sw -side top -expand yes -fill both -pady 0
} else {
    pack $sw -side top -expand yes -fill both -pady 1
}

# realize widgets
update

# add context-menu
set m [menu $ay(tree).popup -tearoff 0]

set ay(treecm) $m

$m add cascade -label "Tree" -menu $ay(tree).popup.tree\
    -underline 1
set m [menu $ay(tree).popup.tree -tearoff 0]
global aymainshortcuts
$m add command -label "Rebuild <$aymainshortcuts(Update)>"\
    -command "tree_reset"
$m add command -label "Expand All <$aymainshortcuts(ExpandAll)>"\
    -command "tree_expand"
$m add command -label "Collapse All <$aymainshortcuts(CollapseAll)>"\
    -command "tree_collapse"
$m add command -label "Expand Selected <+>" -command "tree_toggleTree 1"
$m add command -label "Collapse Selected <->" -command "tree_toggleTree 2"
$m add command -label "Toggle Selected <space>" -command "tree_toggleTree 0"
set m $ay(tree).popup

$m add separator
$m add command -label "Switch to Listbox" -command tree_toggle\
    -underline 0
$m add separator
$m add command -label "Deselect Object" -command {
    global ay
    cS
    plb_update
    if { $ay(need_redraw) == 1 } {
	rV
    }
} -underline 1
$m add separator

$m add command -label "Copy Object" -command "\$ay(editmenu) invoke 0"\
    -underline 0
$m add command -label "Cut Object" -command "\$ay(editmenu) invoke 1"\
    -underline 2
$m add command -label "Paste Object" -command "\$ay(editmenu) invoke 2"\
    -underline 0
#$m add command -label "Paste (Move)" -command "cmovOb;uS;rV"
$m add separator

$m add command -label "Delete Object" -command "\$ay(editmenu) invoke 3"\
    -underline 0
$m add separator

$m add command -label "Help on Object" -command "\$ay(helpmenu) invoke 1"\
    -underline 0

global aymainshortcuts
bind $ay(tree) <ButtonPress-$aymainshortcuts(CMButton)>\
    "winOpenPopup $ay(tree)"

# initial focus management (plb_update/plb_focus change this again)
if { $ayprefs(SingleWindow) == 1 } {
    bind $ay(tree) <Key-Tab> "focus .fu.fMain.fview3;break"
    bind $ay(tree) <Shift-Tab> "focus .fv.fViews.fview2;break"
} else {
    bind $ay(tree) <Key-Tab> "focus .fl.con.console;break"
}

# bind escape
bind $ay(tree) <Escape> {
    shortcut_addescescbinding $ay(tree)
}

bind $ay(tree) <Key-$aymainshortcuts(ExpandAll)> tree_expand
bind $ay(tree) <Key-$aymainshortcuts(CollapseAll)> tree_collapse

# XXXX unfortunately, this does not work
# because this steals all events otherwise
# meant for nodes
#bind $ay(tree) <ButtonPress-1> "sL;rV"

set ay(CurrentLevel) "root"
set ay(SelectedLevel) "root"
tree_paintLevel "root"
set ay(droplock) 0

#Tree::finsert
#  A faster tree "insert", placed here to avoid trouble
#  on startup (only after tree widget creation BWidgets
#  code is fully loaded and the Tree namespace present).
#  In contrast to Tree::insert:
#  This proc has no error checks.
#  This proc just appends new nodes (it has no index argument).
#  This proc does not schedule any redraw updates (we
#  trigger them "manually" via the tree widgets -redraw option anyway).
#  This proc does not return the new node.
proc Tree::finsert { path parent node args } {
    variable $path
    upvar 0  $path data

    Widget::init Tree::Node $path.$node $args

    lappend data($parent) $node

    set data($node) [list $parent]

 return;
}
# Tree::finsert


 return;
}
# tree_open


#tree_close:
#
proc tree_close { w } {

    destroy $w.ftr

}
# tree_close


#tree_toggle:
# switch from listbox to tree or vice versa
proc tree_toggle { } {
    global ay ayprefs

    set w .fu.fMain.fHier

    if { [winfo exists $ay(tree)] } {
	# switch to listbox
	tree_close $w
	olb_open $w
	cS
	olb_update
	if { $ay(need_redraw) == 1 } {
	    rV
	}
	set ayprefs(showtr) 0
    } else {
	# switch to tree
	cS; uS
	olb_close $w
	tree_open $w
	tree_update root
	plb_update
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	tree_paintLevel "root"
	set ay(droplock) 0
	if { $ay(need_redraw) == 1 } {
	    rV
	}
	set ayprefs(showtr) 1
    }

    shortcut_main .

 return;
}
# tree_toggle


#tree_reset:
# reset/rebuild tree
proc tree_reset { } {
    global ay

    set sel ""
    set sel [$ay(tree) selection get]

    tree_update root
    update
    if { [$ay(tree) exists $ay(CurrentLevel)] } {
	tree_openTree $ay(tree) $ay(CurrentLevel)
	set ay(SelectedLevel) $ay(CurrentLevel)
	if { $sel != "" } {
	    tree_paintLevel $ay(CurrentLevel)
	    eval $ay(tree) selection set $sel
	    eval treeSelect $sel
	} else {
	    tree_paintLevel $ay(CurrentLevel)
	    $ay(tree) selection clear
	    treeSelect
	}
	tree_handleSelection
    } else {
	goTop
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	tree_paintLevel $ay(CurrentLevel)
	cS
	#tree_selectItem 1 $ay(tree) "root:0"
	$ay(tree) see "root:0"
    }

    plb_update
    rV

 return;
}
# tree_reset


#tree_gotop:
# go to top level
proc tree_gotop { } {
    global ay

    if { $ay(CurrentLevel) != "root" } {
	tree_paintLevel "root"
    }
    goTop
    set ay(CurrentLevel) "root"
    set ay(SelectedLevel) "root"

 return;
}
# tree_gotop


#tree_toggleTree:
# toggle/open/close all selected sub-trees recursively
proc tree_toggleTree { mode } {
    global ay

    if { $ay(lb) == 1 } { return }

    set sel ""
    set sel [$ay(tree) selection get]
    foreach node $sel {
	switch $mode {
	    0 {
		# toggle
		if { [$ay(tree) itemcget $node -open] } {
		    foreach n [$ay(tree) nodes $node] {
			$ay(tree) closetree $n
		    }
		    $ay(tree) closetree $node
		} else {
		    foreach n [$ay(tree) nodes $node] {
			$ay(tree) opentree $n
		    }
		    $ay(tree) opentree $node
		}
	    }
	    1 {
		# open
		foreach n [$ay(tree) nodes $node] {
		    $ay(tree) opentree $n
		}
		$ay(tree) opentree $node
	    }
	    2 {
		# close
		foreach n [$ay(tree) nodes $node] {
		    $ay(tree) closetree $n
		}
		$ay(tree) closetree $node
	    }
	}
	# switch mode
    }
    # foreach sel
 return;
}
# tree_toggleTree
