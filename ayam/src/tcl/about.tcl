# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# about.tcl - the about requester

#
#
#
proc aboutAyam {} {
global ay tcl_version tk_version tcl_patchLevel tk_patchLevel
set w .aboutw
# there can only be one
catch {destroy $w}
toplevel $w
wm title $w "About Ayam"
wm iconname $w "Ayam"
wm transient .
frame $w.ftext
frame $w.fbutton

getVersion

pack $w.ftext -in $w -side top -expand yes -fill both
pack $w.fbutton -in $w -side bottom

text $w.ftext.text -yscrollcommand "$w.ftext.sbar set" \
	-setgrid 1 -height 17 -width 65
scrollbar $w.ftext.sbar -takefocus 0 -command "$w.ftext.text yview"
pack $w.ftext.text -in $w.ftext -side left -fill both -expand yes
pack $w.ftext.sbar -in $w.ftext -side right -fill y

$w.ftext.text insert end \
{
Ayam, a free 3D Tcl/Tk/OpenGL/RenderMan modeler

Ayam is
Copyright (c) 1998-2001 by Randolf Schultz
(rschultz@informatik.uni-rostock.de)
All Rights Reserved

See the file License for details.

For additional information new versions etc. see:
http://www.ayam3d.org/

http://wwwicg.informatik.uni-rostock.de/~rschultz/ayam/

Version Information:
}

$w.ftext.text tag add "tag1" 1.0 9.end
$w.ftext.text tag add "tag2" 2.0 2.end
$w.ftext.text tag add "httag1" 12.0 12.end
$w.ftext.text tag add "httag2" 14.0 14.end
$w.ftext.text tag configure tag1 -justify center
$w.ftext.text tag configure tag2 -underline yes
$w.ftext.text tag configure httag1 -relief raised -borderwidth 1
$w.ftext.text tag configure httag2 -relief raised -borderwidth 1
$w.ftext.text tag configure httag3 -relief raised -borderwidth 1
$w.ftext.text tag bind httag1 <ButtonPress-1> {
    browser_urlOpen "http://www.ayam3d.org/"}
$w.ftext.text tag bind httag2 <ButtonPress-1> {
    browser_urlOpen "http://wwwicg.informatik.uni-rostock.de/~rschultz/ayam/"}
$w.ftext.text insert end \
"
Ayam: $ay(ay_version)

Tcl
Version: $tcl_version
PatchLevel: $tcl_patchLevel

Tk
Version: $tk_version
PatchLevel: $tk_patchLevel

OpenGL
Version:  $ay(gl_ver)
Renderer: $ay(gl_ren)
Vendor:   $ay(gl_ven)
"

foreach i $ay(gl_ext) {
$w.ftext.text insert end "Extension: $i
" 
}

$w.ftext.text insert end \
"
GLU
Version: $ay(glu_ver)
"

foreach i $ay(glu_ext) {
$w.ftext.text insert end "Extension: $i
" 
}

$w.ftext.text insert end \
"

Ayam is based on the following fine software:

o BMRT (Blue Moon Rendering Tools) by Larry Gritz
o The Togl widget by Benjamin Bederson, Brian Paul et. al.
o The tkMegaWidget set by Jeffrey Hobbs
o The 3DMF parser from Apple (Duet Development Corp.)
o Tcl/Tk, OpenGL, The RenderMan Interface (too many to list)

Ayam learned a lot from the code of:

o The Affine Toolkit by Thomas E. Burge
o GLUT by M. Kilgard
o The NURBS Book by L. Piegl and W. Tiller
o The NURBS Procedure Library by W.T. Hewitt and D. Yipp
o The NURBS++ library by P. Lavoie

OpenGL (R) is a registered trademark of Silicon Graphics, Inc.

The RenderMan (R) Interface Procedures and Protocol are:
Copyright 1988, 1989, 2000, Pixar
All Rights Reserved

RenderMan (R) is a registered trademark of Pixar

The Affine Libraries and Tools are
Copyright (c) 1995, 1996, 1997, 1998 Thomas E. Burge
All rights reserved.

Affine (R) is a registered trademark of Thomas E. Burge
"

$w.ftext.text tag add "tag3" end-14lines end

$w.ftext.text tag configure tag3 -justify center

$w.ftext.text configure -state disabled
bind $w <Next> "$w.ftext.text yview scroll 1 pages"
bind $w <Prior> "$w.ftext.text yview scroll -1 pages"

button $w.fbutton.b -text "Dismiss" -command "destroy $w"
pack $w.fbutton.b -in $w.fbutton

}
