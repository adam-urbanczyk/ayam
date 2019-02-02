# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2019 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# link.tcl - manually control linking of view and main window

# set the keyboard shortcut to toggle linking:
set link_hotkey "<F7>"

catch {rename ::plb_update ::link_plb_update}
catch {rename ::rV ::link_rV}
set ay(couple) 0

proc plb_update { } {
    global ay
    if { $ay(couple) || ![link_focus] } {
	::link_plb_update
    }
 return;
}


proc rV { args } {
    global ay
    if { $ay(couple) || [link_focus] } {
	::link_rV $args
    }
 return;
}


proc link_focus { } {
    global ayprefs ay

    if { $ayprefs(SingleWindow) } {
	# single window
	if { [string first ".view" [focus]] == 0 } {
	    return 1;
	} else {
	    if { [string first ".fv" [focus]] == 0 } {
		return 1;
	    } else {
		if { [string first ".fu.fMain.fview3" [focus]] == 0 } {
		    return 1;
		}
	    }
	    return 0;
	}
    } else {
	# multi window
	if { [string first ".view" [focus]] == 0 } {
	    return 1;
	}
    }

 return 0;
}

proc link_toggle { } {
    global ay
    if { $ay(couple) } {
	uS 1 1; rV;
    }
    return;
}

proc link_togglekbd { } {
    global ay
    set ay(couple) [expr {!$ay(couple)}]
    link_toggle
    return;
}

set imgdata {\
R0lGODlhCwALAKECAAAAAE5OTv///////yH5BAEKAAIALAAAAAALAAsAAAIX
lI+pGeCwxEPBPHjgxHaKmjnRBUbmUgAAOw==}

image create photo ay_Link_img -format GIF -data $imgdata

catch {unset imgdata}

set cb .fu.fMain.fHier.cb
if { 0 } {
    image create photo ay_LinkO_img -format GIF -file chaino.gif
    image create photo ay_LinkC_img -format GIF -file chainc.gif
    checkbutton $cb -variable ay(couple) -command link_toggle\
	-image ay_LinkO_img -selectimage ay_LinkC_img\
	-ind false -selectcolor "" -rel flat\
	-overrel raised -offrel flat
} else {
    checkbutton $cb -variable ay(couple) -command link_toggle\
	-image ay_Link_img\
	-ind false -selectcolor "" -rel flat\
	-overrel raised -offrel flat -borderwidth 1
}

balloon_set $cb "Link Hierarchy with Views"

if { $ay(ws) == "Aqua" } {
    place $cb -in .fu.fMain.fHier -anchor ne -relx 1.0 -y -2 -height 22
} else {
    if { $ay(ws) == "Win32" } {
	place $cb -in .fu.fMain.fHier -anchor ne -relx 1.0 -y 0 -height 17
    } else {
	# X11
	place $cb -in .fu.fMain.fHier -anchor ne -relx 1.0 -y 0\
	    -height 17 -width 17
    }
}


addToProc shortcut_fkeys {
    bind $w <[repctrl $aymainshortcuts(Update)]>\
     {
	 ayError 4 $aymainshortcuts(Update) "Update!"
	 set ay(coupleorig) $ay(couple)
	 set ay(couple) 1
	 notifyOb -all; uS 1 1; rV "" 1;
	 set ay(couple) $ay(coupleorig)
     }
}

catch { bind . $link_hotkey link_togglekbd }
catch { shortcut_addviewbinding $link_hotkey link_togglekbd }

# EOF
