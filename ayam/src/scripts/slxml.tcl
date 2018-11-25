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

proc shaderScanError { at ln fn } {
 ayError 2 shaderScanXML [concat "No" $at "attribute in:" $ln "(" $fn ")"]
}

proc shaderScan { file varname } {
    global ay ayprefs
    upvar 1 $varname shader

    append file ".sl"

    set spathstr [split [shader_unglobShaderPaths "$ayprefs(Shaders)"]\
		      $ay(separator)]
    set f ""
    foreach p $spathstr {
	set s [file join $p $file]
	if { [file exists $s] } {
	    catch {set f [open $s]}
	    if { $f != "" } {
		break;
	    }
	}
    }
    if { $f == "" } {
	ayError 10 shaderScanXML $file
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
		regexp -- {.*type="([^"]*)"} $rl a t
		if {$a != ""} {
		  regexp -- {.*name="([^"]*)"} $rl a n
		  if {$a == ""} {
		      shaderScanError name $rl $file
		      break;
		  }
		} else {
		    shaderScanError type $rl $file
		    break;
		}
		lappend shader $n
		lappend shader $t
	    }
	    # if is shader
	    if { [string first "<argument " $rl] > -1 } {
		regexp -- {.*name="([^"]*)"} $rl a n
                if {$a != ""} {
		  regexp -- {.*type="([^"]*)"} $rl a t
                  if {$a != ""} {
		    regexp -- {.*value="([^"]*)"} $rl a v
                    #the following comment just helps emacsens font lock mode
                    #"
                    if {$a == ""} {
			shaderScanError value $rl $file
			break;
                    }
		  } else {
		      shaderScanError type $rl $file
		      break;
		  }
                } else {
		    shaderScanError name $rl $file
		    break;
		}
	        set parameter ""
	        lappend parameter $n $t 0 $v
	        lappend parameters $parameter
	    }
	    # if is argument
        }
        # if not eof
    }
    # while not done

    lappend shader $parameters

    catch [close $f]

 return;
}
# shaderScan
