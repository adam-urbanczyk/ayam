# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2015 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# dtree.tcl - a dynamic/faster object hierarchy tree widget

# DTree changes the BWidget tree code, so that it no longer creates
# canvas items for _all_ nodes, but only for the nodes visible in
# the current scroll region, which is much faster, if there are many
# nodes (in the range of ten thousands); however, this means, while
# scrolling we must call Tree::_draw_tree (and thus create new items)
# all the time, also "see" can no longer just request the bbox of items
# that are not in the scroll region (as they are not there), instead, the
# potential position of the node must be inferred by some helper procs (that
# browse the nodes lists/hierarchy and accumulate a y-position, just like the
# _draw_node/_draw_subnodes duo does)...
# For additional speed-up, the current level painting mechanism has been
# integrated in the drawing mechanism (otherwise, changing the current level
# would lead to too many calls to Widget::setoption).


#tree_paintLevel:
# this proc normally paints levels by setting the respective node/item
# configurations, but in DTree, the current level is painted on the fly,
# so we just trigger a redraw
proc tree_paintLevel { level } {
    global ay
    Tree::_draw_tree $ay(tree)
    return;
}

#tree_paintTree:
# see above
proc tree_paintTree { level } {
    global ay
    Tree::_draw_tree $ay(tree)
    return;
}

#tree_handleSelection:
# do any stuff for the selected objects such as highligting the selected level
proc tree_handleSelection { } {
    global ay
    set snodes [$ay(tree) selection get]
    if { $ay(SelectedLevel) != $ay(CurrentLevel) } {
	set ay(CurrentLevel) $ay(SelectedLevel)
	Tree::_draw_tree $ay(tree)
	$ay(tree) selection set $snodes
    }
    eval [subst "treeSelect $snodes"]
 return;
}
# tree_handleSelection


#tree_openSub:
# open/close subtree in tree view
proc tree_openSub { tree newstate node } {
    global ay
    set ay(ts) 1;
    $tree itemconfigure $node -open $newstate
    if { [Widget::getoption $tree -redraw] } {
	after idle "Tree::_draw_tree $tree"
    }
 return;
}
# tree_openSub

#tree_expand:
# open all nodes
proc tree_expand { } {
    global ay
    $ay(tree) configure -redraw 0
    set nlist [$ay(tree) nodes root]
    foreach n $nlist {
	$ay(tree) opentree $n
    }
    $ay(tree) configure -redraw 1
 return;
}
# tree_expand

#tree_collapse:
# close all nodes
proc tree_collapse { } {
    global ay
    $ay(tree) configure -redraw 0
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
    $ay(tree) configure -redraw 1
 return;
}
# tree_collapse


# ------------------------------------------------------------------------------
#  Command Tree::_draw_node
# ------------------------------------------------------------------------------
proc Tree::_draw_node { path node x0 y0 deltax deltay padx showlines f } {
    global   env ay ObjectSearch
    variable $path
    upvar 0  $path data

    set x1 [expr {$x0+$deltax+5}]
    set y1 $y0
    set yl $y1
    if { $y0 < $data(bot) && $y0 > $data(top) } {
	if { $showlines } {
	    $path:cmd create line $x0 $y0 $x1 $y0 \
		-fill    [Widget::getoption $path -linesfill]   \
		-stipple [Widget::getoption $path -linestipple] \
		-tags    line
	}
	set wf [Widget::getoption $path.$node -fill]
	if { $wf != $ObjectSearch(HighlightColor) } {
	    set wf $f
	    Widget::setoption $path.$node -fill $f
	}
	$path:cmd create text [expr {$x1+$padx}] $y0 \
	    -text   [Widget::getoption $path.$node -text] \
	    -fill   $wf \
	    -font   [Widget::getoption $path.$node -font] \
	    -anchor w \
	    -tags   "node n:$node"
    }

    set len [expr {[llength $data($node)] > 1}]
    set dc  [Widget::getoption $path.$node -drawcross]
    set exp [Widget::getoption $path.$node -open]

    if { $len && $exp } {
	if { $node == $ay(CurrentLevel) } {
	    set sf "black"
	} else {
	    set sf "darkgrey"
	}
        set y1 [_draw_subnodes $path [lrange $data($node) 1 end] \
               [expr {$x0+$deltax}] $y0 $deltax $deltay $padx $showlines $sf]
    }

    if { $y0 < $data(bot) && $y0 > $data(top) } {

	# remember last node actually drawn in top level
	set data(lst) $yl

	if { [string compare $dc "never"] &&
	     ($len || ![string compare $dc "allways"]) } {
	    if { $exp } {
		#XXXX
		#set bmp [file join $env(BWIDGET_LIBRARY) "images" "minus.xbm"]
		set bmp "minus"
	    } else {
		#XXXX
		#set bmp [file join $env(BWIDGET_LIBRARY) "images" "plus.xbm"]
		set bmp "plus"
	    }
	    #XXXX was -bitmpa @$bmp...
	    $path:cmd create bitmap $x0 $y0 \
		-bitmap     $bmp \
		-background [$path:cmd cget -background] \
		-foreground [Widget::getoption $path -linesfill] \
		-tags       "cross c:$node" -anchor c
	}

	if { [set win [Widget::getoption $path.$node -window]] != "" } {
	    $path:cmd create window $x1 $y0 -window $win -anchor w\
		-tags "win i:$node"
	} elseif { [set img [Widget::getoption $path.$node -image]] != "" } {
	    $path:cmd create image $x1 $y0 -image $img -anchor w\
		-tags "img i:$node"
	}

    }
    return $y1
}


# ------------------------------------------------------------------------------
#  Command Tree::_draw_subnodes
# ------------------------------------------------------------------------------
proc Tree::_draw_subnodes { path nodes x0 y0 deltax deltay padx showlines f } {
    variable $path
    upvar 0  $path data

    set y1 $y0
    foreach node $nodes {
        set yp $y1
        set y1 [_draw_node $path $node $x0 [expr {$y1+$deltay}] $deltax $deltay $padx $showlines $f]
    }

    if { $y0 < $data(bot) || $y0 > $data(top) } {
	if { $showlines && [llength $nodes] } {
	    set id [$path:cmd create line $x0 $y0 $x0 [expr {$yp+$deltay}] \
			-fill    [Widget::getoption $path -linesfill]   \
			-stipple [Widget::getoption $path -linestipple] \
			-tags    line]

	    $path:cmd lower $id
	}
    }
    return $y1
}


# ------------------------------------------------------------------------------
#  Command Tree::Tree::_draw_tree
# ------------------------------------------------------------------------------
proc Tree::_draw_tree { path } {
    global ay
    variable $path
    upvar 0  $path data

    foreach { xmin ymin xmax ymax } [$path:cmd cget -scrollregion] break
    foreach { y1 y2 } [$path:cmd yview] break

    set data(top) [expr {($ymax - $ymin) * $y1 + $ymin - 8}]
    set data(bot) [expr {($ymax - $ymin) * $y2 + $ymin + 8}]

    $path:cmd delete all

    set b "darkgrey"
    if { $ay(CurrentLevel) == "root" } {
	set b "black"
    }

    set yl [_draw_subnodes $path [lrange $data(root) 1 end] 8 \
		[expr {-[Widget::getoption $path -deltay]/2}] \
		[Widget::getoption $path -deltax] \
		[Widget::getoption $path -deltay] \
		[Widget::getoption $path -padx]   \
		[Widget::getoption $path -showlines] $b]

    # these two items span the canvas space (which is almost empty)
    $path:cmd create text 0 0
    $path:cmd create text 0 $yl

    # we can not rely on _draw_subnodes to draw the leftmost vertical line
    set l [expr [llength $data(root)] - 1]
    if { [Widget::getoption $path -showlines] && $l > 1 } {
	set y0 [expr {-[Widget::getoption $path -deltay]/2}]
	if { $yl > $data(lst) } { set yl $data(lst) }
	set id [$path:cmd create line 8 $y0 8 $yl \
                    -fill    [Widget::getoption $path -linesfill]   \
                    -stipple [Widget::getoption $path -linestipple] \
                    -tags    line]
        $path:cmd lower $id
    }

    return;
}


proc Tree::yview { path args } {
    # scrolling avails to nothing, must trigger a redraw!
    _redraw_idle $path 3
    return [eval $path:cmd yview $args]
}


# ------------------------------------------------------------------------------
#  Command Tree::_scroll
# ------------------------------------------------------------------------------
proc Tree::_scroll { path cmd dir } {
    variable $path
    upvar 0  $path data

    if { ($dir == -1 && [lindex [$path:cmd $cmd] 0] > 0) ||
         ($dir == 1  && [lindex [$path:cmd $cmd] 1] < 1) } {
        $path:cmd $cmd scroll $dir units
	_redraw_idle $path 3
        set data(dnd,afterid) [after 100 Tree::_scroll $path $cmd $dir]
    } else {
        set data(dnd,afterid) ""
        DropSite::setcursor dot
    }
}


# Tree::_accum_node
# helper for _find_y, accumulate the y positions of a node and its children
proc Tree::_accum_node { path node y0 dy f } {
    variable $path
    upvar 0  $path data
    set y1 $y0
    set len [expr {[llength $data($node)] > 1}]
    set exp [Widget::getoption $path.$node -open]
    if { $len && $exp } {
        set y1 [Tree::_accum_subnodes $path [lrange $data($node) 1 end] \
                    $y0 $dy $f]
    }
    return $y1
}


# Tree::_accum_subnodes
# helper for _find_y, accumulate the y positions of a list of sub-nodes
proc Tree::_accum_subnodes { path nodes y0 dy f } {
    variable $path
    upvar 0  $path data
    set y1 $y0
    foreach node $nodes {
	if { $node == $f } { set data(lst) $y1; break; }
        set y1 [Tree::_accum_node $path $node [expr {$y1+$dy}] $dy $f]
    }
    return $y1;
}

# Tree::_find_y:
# helper for _see, find the y position of a node
proc Tree::_find_y { path node } {
    variable $path
    upvar 0  $path data
    set data(lst) 0
    Tree::_accum_subnodes $path [lrange $data(root) 1 end] [expr {-[Widget::getoption $path -deltay]/2}] [Widget::getoption $path -deltay] $node
    return $data(lst);
}


# ------------------------------------------------------------------------------
#  Command Tree::_see
# ------------------------------------------------------------------------------
proc Tree::_see { path node side } {
    if { $side != "left" } {
	set scrl [$path:cmd cget -scrollregion]
	set ymax [lindex $scrl 3]
	set dy   [$path:cmd cget -yscrollincrement]
	set yv   [$path yview]
	set yv0  [expr {round([lindex $yv 0]*$ymax/$dy)}]
	set yv1  [expr {round([lindex $yv 1]*$ymax/$dy)}]
	if { [llength $node] > 1 } {
	    # first see the end, then the first; this way lists fitting
	    # into the view will be shown completely, otherwise, first wins
	    set ye [expr [Tree::_find_y $path [lindex $node end]] / $dy]
	    if { $ye < $yv0 } {
		$path:cmd yview scroll [expr {$ye-$yv0}] units
		Tree::_draw_tree $path
		update idletasks
	    } elseif { $ye >= [expr {$yv1 - 3}] } {
		$path:cmd yview scroll [expr {$ye-$yv1+3}] units
		Tree::_draw_tree $path
		update idletasks
	    }
	    Tree::_see $path [lindex $node 0] $side
	    return;
	}
	set y [expr [Tree::_find_y $path $node] / $dy]
	if { $y < $yv0 } {
	    $path:cmd yview scroll [expr {$y-$yv0}] units
	    Tree::_draw_tree $path
	    update idletasks
	} elseif { $y >= [expr {$yv1 - 3}] } {
	    $path:cmd yview scroll [expr {$y-$yv1+3}] units
	    Tree::_draw_tree $path
	    update idletasks
	}
    }

    set scrl [$path:cmd cget -scrollregion]
    set idn [$path:cmd find withtag n:$node]
    set bbox [$path:cmd bbox $idn]
    if { $bbox == "" } {
	# node does not exist?
	return;
    }
    set xmax [lindex $scrl 2]
    set dx   [$path:cmd cget -xscrollincrement]
    set xv   [$path xview]
    if { ![string compare $side "right"] } {
        set xv1 [expr {round([lindex $xv 1]*$xmax/$dx)}]
        set x1  [expr {int([lindex $bbox 2]/$dx)}]
        if { $x1 >= $xv1 } {
            $path:cmd xview scroll [expr {$x1-$xv1+1}] units
        }
    } else {
        set xv0 [expr {round([lindex $xv 0]*$xmax/$dx)}]
        set x0  [expr {int([lindex $bbox 0]/$dx)}]
        if { $x0 < $xv0 } {
            $path:cmd xview scroll [expr {$x0-$xv0}] units
        }
    }
}
# Tree::_see


# ------------------------------------------------------------------------------
#  Command Tree::see
# ------------------------------------------------------------------------------
proc Tree::see { path node } {
    variable $path
    upvar 0  $path data

    if { [Widget::getoption $path -redraw] && $data(upd,afterid) != "" } {
        after cancel $data(upd,afterid)
        _redraw_tree $path
    }
    Tree::_see $path $node right
    Tree::_see $path $node left
}


#
addToProc shortcut_main {

    bind $w <Left> {
	global ay
	if { $ay(lb) == 0 } {
	    set tree $ay(tree)
	    set cl $ay(CurrentLevel)
	    if { $cl == "root" } {
		break
	    }
	    set i [string last ":" $cl]
	    set newcl [string range $cl 0 [expr ${i}-1]]
	    goUp
	    set ay(CurrentLevel) $newcl
	    set ay(SelectedLevel) $newcl
	    $tree see $cl
	    $tree selection set $cl
	    treeSelect $cl
	    plb_update
	    rV
	} else {
	    $ay(olbbup) invoke
	    break
	}
	# if
    }
    # bind

    bind $w <Right> {
	global ay
	if { $ay(lb) == 0 } {
	    set tree $ay(tree)
	    set sel [$tree selection get]
	    if { $sel == "" } {
		break
	    }
	    set sel [lindex $sel 0]
	    $tree selection set $sel
	    treeSelect $sel
	    if { ! [hasChild] } {
		break
	    }
	    $tree itemconfigure $sel -open 1
	    goDown
	    set ay(CurrentLevel) $sel
	    $tree see ${sel}:0
	    set ay(SelectedLevel) $sel
	    $tree selection set ${sel}:0
	    treeSelect ${sel}:0
	    plb_update
	    rV
	} else {
	    $ay(olbbdwn) invoke
	    break
	}
	# if
    }
    # bind
}
