# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2017 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# slxml.tcl - switch to parsing shaders from XML tags embedded in SL comments

set AYUSESLCARGS 0
set AYUSESLXARGS 0
set ay(sext) ".sl"
#set ayprefs(StripShaderArch) 0

proc shaderScan { file varname } {
    global ay ayprefs
    upvar 1 $varname shader

    append file ".sl"

    set spathstr [split [shader_unglobShaderPaths "$ayprefs(Shaders)"] $ay(separator)]
    set f ""
    foreach p $spathstr {
	set f [open [file join $p $file]]
	if { $f != "" } {
	    break;
	}
    }
    if { $f == "" } {
	ayError 10 shader_scanXML $file
	return;
    }
    set shader ""
    set parameters ""
    set done 0
    while { ! $done } {
	set rl [gets $f]

	if { [eof $f] } {
	    set done 1
	} else {
	    if { [string first "<shader " $rl] > -1 } {
		regexp -- {^.*<shader type="([^"]*)" name="([^"]*)"} $rl a b c
		lappend shader $c
		lappend shader $b
	    }
	    # if is shader
	    if { [string first "<argument" $rl] > -1 } {
		regexp -- {^.*<argument name="([^"]*)".*type="([^"]*)".*value="([^"]*)"} $rl a b c d
#the following comment just helps emacsens font lock mode
#"
	        set parameter ""
	        lappend parameter $b $c 0 $d
	        lappend parameters $parameter
	    }
	    # if is argument
        }
        # if not eof
    }
    # while not done

    lappend shader $parameters

 return;
}
# shaderScan
