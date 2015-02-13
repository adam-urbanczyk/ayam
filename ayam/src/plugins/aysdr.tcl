# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2015 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aysdr.tcl - aysdr shader plugin GUI code

# aysdr_rewritepath:
# helper for aysdr_scansdrtcmd
# changes '...;C:/bla...' to '...;//C/bla...'
proc aysdr_rewritepath { } {
    global ay ayprefs

    set t [split $ayprefs(Shaders) \;]
    set p ""

    foreach elem $t {
	set ind [string first ":" $elem]
	if { $ind > 0 } {
	    append p "//"
	    append p [string index $elem 0]
	    append p [string range $elem 2 end]
	} else {
	    append p $elem
	}
	append p ";"
    }

    set ay(PixieShaders) $p

 return;
}
# aysdr_rewritepath
