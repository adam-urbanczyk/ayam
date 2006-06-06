# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2006 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# shader.tcl - managing shaders

# a shader in the database:
# { sname stype { {argname argtype 1 defval} {arrargname arrargtype arrlen {defval_1 defval_2 }} }
# { plastic surface { {Ka float 1 0.8} {From point 1 { 0.0 1.0 0.0 }} } }
# a shader in an array:
# {
# Name(plastic)
# ArgNames { Ka From }
# ArgTypes { float point }
# Ka 0.8
# From_X 0.0
# From_Y 1.0
# From_Z 0.0
# }


# shader_scanAll:
#
proc shader_scanAll {} {
    global ay env ayprefs AYUSESLCARGS AYUSESLXARGS ay_error


    foreach i [list surface displacement imager light volume transformation] {
	eval "set ay(${i}shaders) \"\" "
    }

    set sext ".xml"

    if { $AYUSESLCARGS == 1 } {
	set sext ".slc"
    }

    if { $AYUSESLXARGS == 1 } {
	set sext ".slx"
    }

    if { $ay(sext) != "" } {
	set sext $ay(sext)
    }

    if { $sext == ".xml" } {
	ayError 2 scanAllShaders "No shader parsing library available."
	ayError 2 scanAllShaders "Falling back to XML parsing."
	return;
    }

    ayError 4 scanAllShaders "Scanning for $sext shaders..."

    set temp ""
    if { $ayprefs(Shaders) == "" } {
	ayError 2 scanAllShaders "Shaders are not configured."
	ayError 4 scanAllShaders "Trying to load initial configuration from env(SHADERS)."
	if {[string length [array names env SHADERS]] == 0} then {
	    ayError 2 scanAllShaders "Environment variable SHADERS not set."
	    ayError 2 scanAllShaders "Please configure Shaders in the Preferences."
	    return;
	} else {
	    set ayprefs(Shaders) $env(SHADERS)
	}
    } else { set env(SHADERS) $ayprefs(Shaders) }

    set spathstr [split "$ayprefs(Shaders)" $ay(separator)]

    ayError 4 scanAllShaders "In \\\"$ayprefs(Shaders)\\\"..."

    foreach p $spathstr {
	set files [glob -nocomplain "${p}/*${sext}"]
	append temp "$files "
    }

    set allshaders ""
    foreach s $temp {
	# silently omit unreadable shaders
	if {[file readable $s]} {
	    lappend allshaders $s
	}
    }
    # foreach

    foreach s $allshaders {

	# strip path from shader-file-name
	set dummy [file tail "$s"]
	# strip extension (.slc/.slx) from shader-file-name
	set dummy [file rootname "$dummy"]
	# strip an eventually present .linux (DSO-shaders)
	# from the shader-file-name
	set dummy [file rootname "$dummy"]

	set shaderarguments ""
	
	set ay_error 0
	if { $ay(sext) != "" } {
	    shaderScan "$dummy" shaderarguments
	    update
	} else {
	    if { $AYUSESLCARGS == 1 } {
		shaderScanSLC "$dummy" shaderarguments
	    }
	
	    if { $AYUSESLXARGS == 1 } {
		shaderScanSLX "$dummy" shaderarguments
	    }
	}
	# if

	if { $ay_error < 2 } {

	    set shadertype [lindex $shaderarguments 1]
	    if { $shadertype != "" } {
		set shadernamelistname ay(${shadertype}shaders)
		eval set shadernamelist \$$shadernamelistname

		if {[string first " $dummy " $shadernamelist ] == -1} {
		    lappend $shadernamelistname [lindex $shaderarguments 0]
		}
	    }
	    # if
	}
	# if
    }
    # foreach

    # sort all lists
    foreach i [list surface displacement imager light volume transformation] {
	set shadernamelistname ay(${i}shaders)
	eval set list \$$shadernamelistname
	set $shadernamelistname [lsort -dictionary $list]
    }
    # foreach

    # luniq (remove multiple entries)
    foreach i [list surface displacement imager light volume transformation] {
	set shadernamelistname ay(${i}shaders)
	eval set list \$$shadernamelistname
	set j [llength $list]
	set k 0
	set $shadernamelistname ""
	while { $j > 0 } {
	    if { [lindex $list $k] != [lindex $list [expr $k + 1]] } {
		lappend $shadernamelistname [lindex $list $k]
	    }
	    incr j -1
	    incr k
	}
	# while
    }
    # foreach

    set n 0
    foreach i [list surface displacement imager light volume transformation] {
	set shadernamelistname ay(${i}shaders)
	eval set list \$$shadernamelistname
	set n [expr ($n + [llength $list])]
    }
    # foreach

    if { $n > 0} {
	set out [format "%d unique shaders found." $n]
	ayError 4 scanAllShaders $out
    } else {
	ayError 1 scanAllShaders "No shaders found."
	ayError 4 scanAllShaders\
"Please check the setting of the Shaders preference setting."
    }
    # if

 return;
}
# shader_scanAll


# shader_cycSel:
#  arrange for listbox entries in list <l> to be cycle-selected
#  when key <k> is pressed for listbox <w>
proc shader_cycSel { w k l } {
    global $l

    # get old list
    eval [subst "set ol {\$$l}"] 

    set i [lindex $ol 0]

    # make selection and scroll listbox, so that it is visible
    $w selection clear 0 end
    $w selection set $i
    $w see $i

    # rotate list, so that next invocation uses next list entry
    if { [llength $ol] > 1 } {
	set nl [lrange $ol 1 end]
	lappend nl [lindex $ol 0]
	set $l $nl
    }

 return;
}
# shader_cycSel


# shader_setNew:
#  set a new shader for selected object (invoked from "Set new Shader"-button
#  in shader property dialogues)
proc shader_setNew { win type stype } {
    upvar #0 ${type}ShaderData sArgArray

    global env prefs ay ay_error AYUSESLCARGS AYUSESLXARGS

    eval "set shaders \$ay(${stype}shaders)"
    if { ($ay(sext) == "") && !$AYUSESLCARGS && !$AYUSESLXARGS } {
	set types {{"Parsed Shader" ".xml"} {"All files" *}}
  
	set newfilename [tk_getOpenFile -filetypes $types -parent .\
		-title "Select parsed shader:"]
	if { $newfilename != "" } {
	    set ay_error 0
	    shader_scanXML $newfilename shaderarguments 

	    if { $ay_error > 1 } {
		ayError 2 shader_setNew "Oops, could not scan shader!"
		return;
	    }

	    if { [lindex $shaderarguments 1] != $stype } {
		ayError 2 shader_setNew\
			"Shader is of wrong type, need a $stype shader!"
		return;
	    }

	    shader_DbToArray $shaderarguments
	    shaderSet $type ay_shader
	    plb_update

	}
	return;
    }
    # request a new shader

    winAutoFocusOff
    set w .setShaderw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Set Shader"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    listbox $f.lb -width 20 -height 10 -selectmode single\
	    -exportselection 0 -yscrollcommand {.setShaderw.f1.fsc.sc set}
    # insert shaders into listbox
    eval [subst "$f.lb insert 0 $shaders"]

    # arrange for keyboard to cycle-select through shaders
    # first, clear some global lists
    foreach shader $shaders {
	set fc [string tolower [string index $shader 0]]
	global ${fc}list
	set ${fc}list {}
    }
    # foreach
    # now, fill the global lists with list indizes, and arrange for
    # the respective key on the keyboard to cycle-select using shader_cycsel
    set i 0
    foreach shader $shaders {
	set fc [string tolower [string index $shader 0]]
	global ${fc}list
	lappend ${fc}list $i
	if { [ llength ${fc}list ] == 1 } {
	    bind $w <KeyPress-${fc}> "shader_cycSel $f.lb $fc ${fc}list" 
	}
	incr i
    }
    # foreach

    pack $f.lb -in $f -side left -fill both -expand yes
    set f [frame $f.fsc]
    scrollbar $f.sc -command {.setShaderw.f1.lb yview} -takefocus 0
    pack $f.sc -in $f -side right -fill y -expand yes
    set f $w.f1
    pack $f.fsc -in $f -side right -fill y
    pack $f -in $w -side top -fill both -expand yes
    
    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global newshaderindex
	set newshaderindex [.setShaderw.f1.lb curselection]
	grab release .setShaderw
	focus .
	destroy .setShaderw
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    global newshaderindex;\
	    set newshaderindex \"\";\
	    destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    wm protocol $w WM_DELETE_WINDOW {
	.setShaderw.f2.bca invoke
    }

    winCenter $w
    grab $w
    focus $f.bok
    tkwait window $w
    winAutoFocusOn

    # now we have the new shader
    global newshaderindex
    if {$newshaderindex == ""} {return;}

    set shadername [lindex $shaders $newshaderindex]

    set shaderarguments ""
    set ay_error 0
    if { $ay(sext) != "" } {
	shaderScan $shadername shaderarguments
    } else {
	if { $AYUSESLCARGS == 1 } {
	    shaderScanSLC $shadername shaderarguments
	}
    
	if { $AYUSESLXARGS == 1 } {
	    shaderScanSLX $shadername shaderarguments
	}
    }

    if { $ay_error > 1 } {
	ayError 2 shader_setNew "Oops, could not scan shader!"
	break;
    }

    shader_DbToArray $shaderarguments
    undo save SetShader
    shaderSet $type ay_shader
    plb_update

 return;
}
#shader_setNew


# shader_scanXML:
#  scan a shader that has been parsed with sdpslparse/sl2xml
proc shader_scanXML { file varname } {
    global ay ayprefs
    upvar 1 $varname shader

    if {! [file readable $file] } {
	ayError 10 shader_scanXML $fi
	return;
    }

    set f [open $file]
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

		if { [string first "<argument" $rl] > -1 } {
		    regexp -- {^.*<argument name="([^"]*)".*type="([^"]*)".*value="([^"]*)"} $rl a b c d
#the following comment just helps emacsens font lock mode
#"

		    set parameter ""
		    lappend parameter $b $c 0 $d
		    lappend parameters $parameter

		}
		
	    }
	    
	}

	lappend shader $parameters

 return;
}
# shader_scanXML


# shader_setDefaultsXML:
#  reset all parameters to shader default values
proc shader_setDefaultsXML { type } {
    global ay ayprefs ay_shader

    if { $ay_shader(Name) == "" } { return; }

    set shaderarguments ""
    set ay_error 0

    # search for shader

    set spathstr [split "$ayprefs(Shaders)" $ay(separator)]

    foreach p $spathstr {
	set files [glob -nocomplain "${p}/*.xml"]
	append temp "$files "
    }

    set allshaders ""
    set filename ""
    foreach s $temp {
	# silently omit unreadable shaders
	if {[file readable $s]} {
	    # strip path from shader-file-name
	    set dummy [file tail $s]
	    if { $ay_shader(Name) == [ file rootname $dummy ] } {
		set filename $s
		break;
	    }
	}
    }
    #foreach

    if { $filename != "" } {
	shader_scanXML $filename shaderarguments
    } else {
	ayError 2 shader_setDefXML "Could not find $ay_shader(Name).xml!"
	ayError 2 shader_setDefXML\
		"Maybe a path is missing from Prefs/Main/Shaders?"
	return;
    }

    if { $ay_error > 1 } {
	ayError 2 shader_setDefXML "Oops, could not scan shader!"

	break;
    }

    shader_DbToArray $shaderarguments
    undo save ShadDflts
    shaderSet $type ay_shader
    plb_update

 return;
}
# shader_setDefaultsXML


# shader_setDefaults:
#  reset all parameters to shader default values
proc shader_setDefaults { type } {
    global ay_shader AYUSESLCARGS AYUSESLXARGS

    if { $ay_shader(Name) == "" } { return; }

    set shaderarguments ""
    set ay_error 0

    if { $AYUSESLCARGS == 1 } {
	shaderScanSLC $ay_shader(Name) shaderarguments
    }

    if { $AYUSESLXARGS == 1 } {
	shaderScanSLX $ay_shader(Name) shaderarguments
    }

    if { $ay_error > 1 } {
	ayError 2 shader_setDefaults "Oops, could not scan shader!"
	break;
    }

    shader_DbToArray $shaderarguments
    undo save ShadDflts
    shaderSet $type ay_shader
    plb_update

 return;
}
# shader_setDefaults


# shader_buildGUI:
#  create new shader GUI in w
#  lets user select new shaders of type type
proc shader_buildGUI { w type } {
global ay ay_shader AYUSESLCARGS AYUSESLXARGS

set stype $type
if { $type == "atmosphere" } { set stype "volume" }
if { $type == "interior" } { set stype "volume" }
if { $type == "exterior" } { set stype "volume" }

addCommand $w c1 "Set new shader!" "shader_setNew $w $type $stype"

addCommand $w c2 "Delete shader!" "undo save DelShader; shaderSet $type;\
	plb_update"

if { ($ay(sext) == "") && !$AYUSESLCARGS && !$AYUSESLXARGS } {
    addCommand $w c3 "Default Values!" "shader_setDefaultsXML $type;"
} else {
    addCommand $w c3 "Default Values!" "shader_setDefaults $type;"
}

if { $ay_shader(Name) == "" } { return; }

addText $w e1 $ay_shader(Name)

set numargs [llength $ay_shader(ArgNames)]

for {set i 0} {$i < $numargs} {incr i} {

    set argtype [lindex $ay_shader(ArgTypes) $i]
    
    set argname [lindex $ay_shader(ArgNames) $i]

    switch $argtype {

	color {
	    addColor $w ay_shader $argname
	}
	point -
	normal -
	vector {
	    addParam $w ay_shader ${argname}_X
	    addParam $w ay_shader ${argname}_Y
	    addParam $w ay_shader ${argname}_Z
	}
	float {
	    addParam $w ay_shader ${argname}
	}
	string {
	    addFile $w ay_shader ${argname}
	}
	matrix {
	    addMatrix $w ay_shader ${argname}
	}
	default {
	  ayError 2 shader_buildGUI "Ignoring unknown argument type: $type!"
	}
    }
    # switch

}
# for

 return;
}
# shader_buildGUI


# shader_DbToArrayParam:
#  helper for shader_DbToArray, get argument defaultvalues
#  from DB and create appropriate variables in ay_shader
proc shader_DbToArrayParam { name type val } {
    global ay_shader

    switch $type {

	color {
	    set ay_shader(${name}_R) [expr int([lindex $val 0]*255)]
	    set ay_shader(${name}_G) [expr int([lindex $val 1]*255)]
	    set ay_shader(${name}_B) [expr int([lindex $val 2]*255)]
	}
	point -
	normal -
	vector {
	    set ay_shader(${name}_X) [lindex $val 0]
	    set ay_shader(${name}_Y) [lindex $val 1]
	    set ay_shader(${name}_Z) [lindex $val 2]
	}
	float {
	    set ay_shader(${name}) $val
	}
	string {
	    set ay_shader(${name}) $val
	}
	matrix {
	    for { set i 0 } { $i < 16 } { incr i } {
		set ay_shader(${name}_$i) [lindex $val $i]
	    }
	}
	default {
       ayError 2 shader_DbToArrayParam "Ignoring unknown argument type: $type!"
	}
    }
    # switch

 return;
}
# shader_DbToArrayParam

# shader_DbToArray:
#  copy shader information from shader database (Db) to
#  the global array ay_shader, ready for editing
proc shader_DbToArray { shader } {
    global ay_shader

    unset ay_shader
    
    set ay_shader(Name) [lindex $shader 0]
    set ay_shader(ArgNames) ""
    set ay_shader(ArgTypes) ""
    set params [lindex $shader 2]

    if { [llength $params] > 0 } {
	foreach param $params {

	    set pname [lindex $param 0]

	    set ptype [lindex $param 1]

	    set num  [lindex $param 2]

	    set vals [lindex $param 3]

	    lappend ay_shader(ArgNames) $pname
	    lappend ay_shader(ArgTypes) $ptype

	    if { $num > 1 } {
		for { set i 0 } { $i < $num } { incr i } {
		    set val [lindex $vals $i]
		    shader_DbToArrayParam ${pname}_${i} $ptype $val
		}
	    } else {
		shader_DbToArrayParam $pname $ptype $vals
	    }
	}
    }

 return;
}
# shader_DbToArray

#
proc shader_findShader { sname } {
 global ay ayprefs

    set tmp ""
    regsub -all $ay(separator) $ayprefs(Shaders) " " tmp;

    foreach p $tmp {
	set fname $p/${sname}$ay(sext)
	if { [file readable $fname] } {
	    return $fname;
	}
    }

 return "";
}
# shader_findShader

proc shader_getAtm { } {
global ay ay_shader Atmosphere
destroy $ay(pca).$Atmosphere(w)
set w [frame $ay(pca).$Atmosphere(w)]
shaderGet atmosphere ay_shader
shader_buildGUI $w atmosphere
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setAtm { } {

shaderSet atmosphere ay_shader

}

array set Atmosphere {
arr    ay_shader
sproc  shader_setAtm
gproc  shader_getAtm
w      fAtmShader
}
# create UI-Stub
set w [frame $ay(pca).$Atmosphere(w)]
####
####
proc shader_getImg { } {
global ay ay_shader Imager

catch {destroy $ay(pca).$Imager(w)}
set w [frame $ay(pca).$Imager(w)]
shaderGet imager ay_shader
shader_buildGUI $w imager
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setImg { } {

shaderSet imager ay_shader

}

array set Imager {
arr    ay_shader
sproc  shader_setImg
gproc  shader_getImg
w      fImgShader
}
# create UI-Stub
set w [frame $ay(pca).$Imager(w)]
####
####
proc shader_getSurf { } {
global ay ay_shader Surface

catch {destroy $ay(pca).$Surface(w)}
set w [frame $ay(pca).$Surface(w)]
shaderGet surface ay_shader
shader_buildGUI $w surface
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setSurf { } {

shaderSet surface ay_shader

}

array set Surface {
arr    ay_shader
sproc  shader_setSurf
gproc  shader_getSurf
w      fSurfShader
}
# create UI-Stub
set w [frame $ay(pca).$Surface(w)]
####
####
proc shader_getDisp { } {
global ay ay_shader Displacement

catch {destroy $ay(pca).$Displacement(w)}
set w [frame $ay(pca).$Displacement(w)]
shaderGet displacement ay_shader
shader_buildGUI $w displacement
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setDisp { } {

shaderSet displacement ay_shader

}

array set Displacement {
arr    ay_shader
sproc  shader_setDisp
gproc  shader_getDisp
w      fDispShader
}
# create UI-Stub
set w [frame $ay(pca).$Displacement(w)]
####
####
proc shader_getInt { } {
global ay ay_shader Interior

catch {destroy $ay(pca).$Interior(w)}
set w [frame $ay(pca).$Interior(w)]
shaderGet interior ay_shader
shader_buildGUI $w interior
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setInt { } {

shaderSet interior ay_shader

}

array set Interior {
arr    ay_shader
sproc  shader_setInt
gproc  shader_getInt
w      fIntShader
}
# create UI-Stub
set w [frame $ay(pca).$Interior(w)]
####
####
proc shader_getExt { } {
global ay ay_shader Exterior

catch {destroy $ay(pca).$Exterior(w)}
set w [frame $ay(pca).$Exterior(w)]
shaderGet exterior ay_shader
shader_buildGUI $w exterior
$ay(pca) itemconfigure 1 -window $w
plb_resize
}

proc shader_setExt { } {

shaderSet exterior ay_shader

}

array set Exterior {
arr    ay_shader
sproc  shader_setExt
gproc  shader_getExt
w      fExtShader
}
# create UI-Stub
set w [frame $ay(pca).$Exterior(w)]
####
