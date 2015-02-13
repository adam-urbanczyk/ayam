# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# level.tcl - level objects Tcl code

set Level_props { Transformations Attributes Material Tags LevelAttr }

array set LevelAttr {
arr   LevelAttrData
sproc ""
gproc ""
w     fLevelAttr
}

array set LevelAttrData {
Type 0
}

# create LevelAttr-UI
set w [frame $ay(pca).$LevelAttr(w)]
addVSpace $w s1 2
addMenu $w LevelAttrData Type { Level Union Difference Intersection Primitive }


# level_crt:
# create Level or tool object with child(ren) (e.g. Revolve);
# makes the currently selected objects children of the new
# object using the object clipboard
proc level_crt { objtype {crtargs "" } {keepsel 0} } {
    global ay ayprefs ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "${objtype}_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    # create an instance first (for ExtrNC/ExtrNP)?
    if { $keepsel < 0 } {
	crtOb Instance; uCR; sL;
	# arrange for the Transformations property to be hidden/disabled
	addTag RP Transformations
    }

    # now create the level object
    set ay_error 0
    if { $crtargs != "" } {
	eval [subst "crtOb $objtype $crtargs"]
    } else {
	crtOb $objtype
    }
    if { $ay_error } { return; }

    # move the instance/selected object to the new level object
    cutOb
    getLevel a b
    goDown -1
    pasmovOb

    # manage the selection
    if { $keepsel <= 0 } {
	goUp
	set ay(ul) $ay(CurrentLevel)
	uS; sL;	notifyOb; rV;
    } else {
	# arrange to keep the old selection
	if { $ay(lb) == 0 } {
	    # tree is active
	    set tree $ay(tree)

	    # update the level, where the new level/tool object was created
	    set ay(ul) $ay(CurrentLevel)
	    uS

	    # paint the old current level gray
	    set nlist [$tree nodes $ay(CurrentLevel)]
	    foreach n $nlist {
		$tree itemconfigure $n -fill darkgrey
	    }

	    # set the new current level
	    if { $ay(CurrentLevel) == "root" } {
		set ay(CurrentLevel) ${ay(CurrentLevel)}:[expr [llength $a]-1]
	    } else {
		set ay(CurrentLevel) ${ay(CurrentLevel)}:[expr [llength $a]-2]
	    }

	    # open the tree-node
	    tree_openSub $tree 1 $ay(CurrentLevel)

	    # paint all selected objects black (mark current level)
	    set nlist [$tree nodes $ay(CurrentLevel)]
	    foreach n $nlist {
		$tree itemconfigure $n -fill black
	    }

	    # select the nodes
	    set nodes [$tree nodes $ay(CurrentLevel)]
	    if { [llength $nodes] > 0 } {
		eval [subst "$tree selection set $nodes"]
		eval [subst "treeSelect $nodes"]
	    }
	    set ay(SelectedLevel) $ay(CurrentLevel)
	} else {
	    # listbox is active
	    uS
	    $ay(olbball) invoke
	}
	# if tree

	# update parent objects
	notifyOb

	# redraw all views
	rV
    }
    # if keepsel
 return;
}
# level_crt
