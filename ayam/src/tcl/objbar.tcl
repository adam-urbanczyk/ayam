# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# objbar.tcl - object bar
# XXXX not working yet at all, tree.tcl needs work in tree_drop!
# button images should not be loaded here, but rather created
# at another central place, finally (why) do we need BWidgets here?

# objbar_open:
#
proc objbar_open { w } {
    global ayprefs ay
    set bp $ayprefs(AYLIB)
    set choices $bp
    lappend choices [file dirname [info nameofexecutable]]/../lib/
    lappend choices [file dirname [info nameofexecutable]]/../buttons/
    lappend choices [file dirname [info nameofexecutable]]/lib/
    lappend choices [file dirname [info nameofexecutable]]/buttons/

    set bp ""
    foreach choice $choices {
	if { [file exists $choice/box.gif ] } {
	    set bp $choice
	    break
	}
    }

    if { $bp != "" } {
	set files { box sphere cylinder cone disk torus hyperb parab ncurve ncircle nrect npatch revolve extrude sweep skin icurve }

    set sw [ScrolledWindow $w.sw -borderwidth 1 -relief sunken]
    set sf [ScrollableFrame $sw.sf -height 30 -constrainedheight true]
    set ay(objb) $sf
    foreach i $files {
        image create photo ${i}img -format GIF -file $bp/$i.gif
        set lab [button [$sf getframe].b$i -image ${i}img]
        pack $lab -side left
    }
    update
    $sw setwidget $sf
    pack $sw -side top -fill x -expand no
}
}
# objbar_open