# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# aycsg.tcl - aycsg CSG preview plugin Tcl code

# addToProc:
#  add code in <addition> to the code of procedure <procedure>
#  replacing the last "return;"-statement in that procedure
#  (which has to be present in order to make this work)
proc addToProc { procedure addition } {
    # get old proc code
    set oldprocbody [info body $procedure]
    # find last "return;"-statement
    set index [string last "return;\n" $oldprocbody]
    # remove last "return;"-statement
    set newprocbody [string range $oldprocbody 0 [expr $index - 1]]
    # add new code
    append newprocbody "\n"
    append newprocbody $addition
    # overwrite procedure
    if { [info args $procedure] != "" } {
	set newargs [list [info args $procedure]] } else { set newargs { } }
    proc $procedure $newargs $newprocbody
 return;
}
# addToProc


set a "bind \$w <Control-C> {global ay; \$ay(currentView) rendercsg;}"
addToProc shortcut_viewactions $a
global ay
foreach view $ay(views) {
    bind ${view} <Control-C>\
	    {global ay; $ay(currentView) rendercsg;}
}
# foreach

return;