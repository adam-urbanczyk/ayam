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
    # add new trailing "return;"-statement
    append newprocbody "\nreturn;\n"
    # overwrite procedure
    if { [info args $procedure] != "" } {
	set newargs [list [info args $procedure]] } else { set newargs { } }
    proc $procedure $newargs $newprocbody
 return;
}
# addToProc

global ay ayprefs

# we always need an open view (OpenGL context) upon startup for the
# GLEW initialization, if there is none, we open it here
if { $ay(views) == "" } {
    viewOpen 400 300
    update
}
# if

# add key binding to view windows to initiate the CSG drawing
set a "bind \$w <Control-C> {global ay; \$ay(currentView) rendercsg;}"
addToProc shortcut_viewactions $a

foreach view $ay(views) {
    bind ${view} <Control-C>\
	    {global ay; $ay(currentView) rendercsg;}
}
# foreach

uplevel #0 {
    array set aycsg_options {
	Algorithm 0
	DCSampling 0
	CalcBBS 0
    }
    # aycsg_options

    array set aycsg_options_save [array get aycsg_options]

}
# uplevel

# aycsgPreferences:
#  AyCSG preferences dialog
proc aycsgPreferences { } {
    global aycsg_options aycsg_options_save

    array set aycsg_options_save [array get aycsg_options]

    set w .aycsgprefs
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "AyCSG Preferences"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x
    addMenu $f aycsg_options_save Algorithm [list Automatic Goldfeather SCS]
    addMenu $f aycsg_options_save DCSampling \
	[list NoDCSampling OcclusionQuery DCSampling]
    addCheck $f aycsg_options_save CalcBBS

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay ayprefs aycsg_options aycsg_options_save

	if { $aycsg_options_save(DCSampling) == 2 } {
	    # check, whether views will be created with stencil buffers,
	    # if not: ask the user for permission to add the needed parameters
	    if { ($ayprefs(AddViewParams) == "") ||
	         ([string first stencil $ayprefs(AddViewParams)] == -1) } {
		set t "Adapt Preferences?"
		set m "Ayam is not set up to use stencil buffers,\n\
			but this is needed by the depth complexity\n\
			sampling strategy you have choosen (DCSampling)!\n\
			Do you want me to adapt the preferences?"
		set answer [tk_messageBox -title $t -type yesno \
			-icon question -message $m]
		if { $answer == "no" } {
		    set aycsg_options_save(DCSampling) 0
		} else {
		    set ayprefs(AddViewParams) "-stencil true -stencilsize 8"
		    update
		    tk_messageBox -title "Information" -type ok\
		    -icon info -message "Stencil buffers are now enabled\n\
		    for every new view window!"
		}
		# if
	    }
	    # if
	}
	# if

	array set aycsg_options [array get aycsg_options_save]
	grab release .aycsgprefs
	focus .
	destroy .aycsgprefs
    }
    # button
    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .aycsgprefs;\
		focus .;\
		destroy .aycsgprefs"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    after idle viewMouseToCurrent

 return;
}
# aycsgPreferences


# add aycsg-preferences dialog to custom menu
set m $ay(cm)
$m add command -label "AyCSG Preferences" -command aycsgPreferences 

return;