
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