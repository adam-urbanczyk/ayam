#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# colfocus.tcl - switch the focus ring color
global ayprefs

if { $ayprefs(SingleWindow) == 1 } {

    # set up the color
    #set focuscol red
    set focuscol blue

    # compile list of widgets to adapt
    # internal views
    set widgets { .fv.fViews.fview1 .fv.fViews.fview2 .fu.fMain.fview3 }
    # object hierarchy
    lappend widgets .fu.fMain.fHier
    # property canvas
    lappend widgets .fu.fMain.fProp.fArg
    # console
    lappend widgets .fl.con

    # set the focus ring color
    foreach w $widgets {
	catch [$w conf -highlightcolor $focuscol]
    }


    ayError 4 "colfocus" "Enabled."
}
