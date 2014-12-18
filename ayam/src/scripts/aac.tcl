#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2010 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aac.tcl - switch rotate and scale actions to their "about center" variants

# vars:

# set this to 0 if you do not want to enable auto-about-center
# upon loading of the script:
set aac_enableonload 1


# set this to 0 if you do not want to create Custom menu entries
# upon loading of the script:
set aac_enablemenu 1


# set the keyboard shortcut to enable/disable auto-about-center:
set aac_hotkey "<F11>"

# procs:

# aac_enable:
#  enable the aac functionality by replacing some action procs
proc aac_enable { } {
    global ay

    # rename original procs to safety
    rename actionRotOb o_actionRotOb
    rename actionSc1DXOb o_actionSc1DXOb
    rename actionSc1DYOb o_actionSc1DYOb
    rename actionSc1DZOb o_actionSc1DZOb
    rename actionSc2DOb o_actionSc2DOb
    rename actionSc3DOb o_actionSc3DOb
    rename actionStr2DOb o_actionStr2DOb

    # establish modified procs
    rename aac_actionRotOb actionRotOb
    rename aac_actionSc1DXOb actionSc1DXOb
    rename aac_actionSc1DYOb actionSc1DYOb
    rename aac_actionSc1DZOb actionSc1DZOb
    rename aac_actionSc2DOb actionSc2DOb
    rename aac_actionSc3DOb actionSc3DOb
    rename aac_actionStr2DOb actionStr2DOb

    set ay(aac_enabled) 1

    ayError 4 "aac" "Enabled."

 return;
}
# aac_enable


# aac_disable:
#  disable the aac functionality by replacing some action procs
proc aac_disable { } {
    global ay

    # rename modified procs to safety
    rename actionRotOb aac_actionRotOb
    rename actionSc1DXOb aac_actionSc1DXOb
    rename actionSc1DYOb aac_actionSc1DYOb
    rename actionSc1DZOb aac_actionSc1DZOb
    rename actionSc2DOb aac_actionSc2DOb
    rename actionSc3DOb aac_actionSc3DOb
    rename actionStr2DOb aac_actionStr2DOb

    # establish original procs
    rename o_actionRotOb actionRotOb
    rename o_actionSc1DXOb actionSc1DXOb
    rename o_actionSc1DYOb actionSc1DYOb
    rename o_actionSc1DZOb actionSc1DZOb
    rename o_actionSc2DOb actionSc2DOb
    rename o_actionSc3DOb actionSc3DOb
    rename o_actionStr2DOb actionStr2DOb

    set ay(aac_enabled) 0

    ayError 4 "aac" "Disabled."

 return;
}
# aac_disable


proc aac_actionRotOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionRotObA $w

 return;
}
# aac_actionRotOb


proc aac_actionSc1DXOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionSc1DXAOb $w

 return;
}
# aac_actionSc1DXOb


proc aac_actionSc1DYOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionSc1DYAOb $w

 return;
}
# aac_actionSc1DYOb


proc aac_actionSc1DZOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionSc1DZAOb $w

 return;
}
# aac_actionSc1DZOb


proc aac_actionSc2DOb { w } {
    global ay ayviewshortcuts

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionSc2DAOb $w

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # allow restriction: x only
    bind $t $ayviewshortcuts(RestrictX) "actionSc1DXOb $w"

    # allow restriction: y only
    bind $t $ayviewshortcuts(RestrictY) "actionSc1DYOb $w"

    # allow restriction: z only
    bind $t $ayviewshortcuts(RestrictZ) "actionSc1DZOb $w"

 return;
}
# aac_actionSc2DOb


proc aac_actionSc3DOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionSc3DAOb $w

 return;
}
# aac_actionSc3DOb


proc aac_actionStr2DOb { w } {
    global ay

    $w mc

    if { $ay(cVPnts) } {
	$w setconf -cmark 1
    } else {
	$w setconf -cmark 0
    }

    actionStr2DAOb $w

 return;
}
# aac_actionStr2DOb


proc aac_toggle { } {
    global ay
    if { $ay(aac_enabled) } {
	aac_disable
    } else {
	aac_enable
    }
}
# aac_toggle


if { $aac_enableonload } {
    aac_enable
} else {
    set ::ay(aac_enabled) 0
}

catch { unset aac_enableonload }

catch { bind . $aac_hotkey aac_toggle }

catch { shortcut_addviewbinding $aac_hotkey aac_toggle }

if { $aac_enablemenu } {
    set m $ay(cm)
    $m add command -label "Toggle AAC" -command aac_toggle\
	-accelerator [remkpkr $aac_hotkey]
}

# EOF
