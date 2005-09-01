#!/usr/bin/env wish

# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ayamConf.tcl - Ayam Makefile configuration GUI by Hynek Schlawack

# Usage:
# to start the configuration, install Tcl/Tk V8.3 or higher and type
# > wish ayamConf.tcl

# Our RIB-Writer

set rib { \
	      {"sribw (Affine, supplied)"} \
	      {"libri2rib (Aqsis)"} \
	      {"libribout (BMRT)"} \
       }

# Our Shader-Parser
set shader { \
		 {"XML (internal, supplied)"} \
		 {"libslxargs (Aqsis)"} \
		 {"libslcargs (BMRT)"} \
	     }

# Our OS
set o { \
	    {IRIX} \
	    {Linux} \
	    {"Mac OS X"} \
	    {Solaris} \
	    {NetBSD} \
	 }

set def_ribval "RIB-Writer"
set def_shaderval "Shader-Parser"
set def_osval "OS"

proc create_makefile {} {
    global ribval shaderval osval
    global def_ribval def_shaderval def_osval

    global bmrtpath aqsispath

    global cflags prefix tclpath tkpath localtcl localtk useaqua aqsis0719 aqsis090 bmrt25 preview

    global tcl_version tk_version

    set err_str "Error!\n\n"
    set err 0


    # Check whether necessary values are set
    if {$ribval == $def_ribval} {
	set err_str "$err_str - No RIB-Writer selected.\n"
	set err 1
    } 

    if {$shaderval == $def_shaderval} {
	set err_str "$err_str - No Shader-Parser selected.\n"
	set err 1
    } 

    if {$osval == $def_osval} {
	set err_str "$err_str - No OS selected.\n"
	set err 1
    }

    # Retrieve Tcl/Tk-Versions
    if { !$localtcl } {
	if {![regexp {tcl([0-9]+)\.([0-9]+)\.[0-9]+/*$} $tclpath\
		dummy tclver tclsub]} {
	    set err_str "$err_str - Cannot determine Tcl-Version from path."
	    set err 1
	}
    }

    if { !$localtk } {
	if {![regexp {tk([0-9]+)\.([0-9]+)\.[0-9]+/*$} $tkpath\
		dummy tkver tksub]} {
	    set err_str "$err_str - Cannot determine Tk-Version from path."
	    set err 1
	}
    }

    # Validate paths
    if {![file exists $tclpath]} {
	set err_str "$err_str - Tcl-Path doesn't exist!\n"
	set err 1
    }

    if { ![file exists $tkpath] || !([file exist "$tkpath/generic/tkInt.h"] ||\
	    [file exist "$tkpath/include/tkInt.h"]) } {
	set err_str "$err_str - Tk-Path doesn't exist or invalid!\n"
	set err 1
    }

    if {([regexp "BMRT"  $ribval] || [regexp "BMRT"  $shaderval]) && ![file exists $bmrtpath]} {
	set err_str "$err_str - BMRT-Path doesn't exist!\n"
	set err 1
    }

    if {([regexp "Aqsis"  $ribval] || [regexp "Aqsis"  $shaderval]) && ![file exists $aqsispath]} {
	set err_str "$err_str - Aqsis-Path doesn't exist!\n"
	set err 1
    }


    if {$err} {
	tk_messageBox -icon error -message $err_str -parent . -type ok
	return
    }   

    #
    # Assemble CFLAGS, LD, PREFIX, LDSWDYNAMIC, EXLDFLAGS, TCLDIR, TCLINC,
    # TCLLIB, TKDIR, TKINC, TKLIB, DL
    # AQSISDIR, AQSISINCDIR, AQSISLIBDIR, AQSISOBJS, AQSISRI2RIB, BMRTDIR,
    # BMRTINCDIR, BMRTLIBDIR, RIBOUTLIB, SLCARGSLIB
    #

    # CFLAGS & LD
    if {[regexp "IRIX" $osval]} {
	set CFLAGS "-DAYIRIXBUG"
	set LD "CC"
    } elseif {[regexp "Linux" $osval]} {
	set CFLAGS "-DAYENABLEWAIT"
	set LD "g++"
    } elseif {[regexp "Solaris" $osval]} {
	set CFLAGS "-DSOLARIS_BUG"
	set LD "CC"
    } elseif {[regexp "MacOS" $osval]} {
	set CFLAGS "-DAYENABLEFEXIT"
	if {$useaqua} {
	    set CFLAGS "$CFLAGS -DAYWITHAQUA"
	}
	set LD "g++"
    } elseif {[regexp "NetBSD" $osval]} {
	set CFLAGS ""
    }

    if {[regexp "Affine" $ribval]} {
	set CFLAGS "$CFLAGS -DAYUSEAFFINE"
    } elseif {[regexp "libribout" $ribval]} {
	set CFLAGS "$CFLAGS -DAYUSEBMRTRIBOUT"
    } elseif {[regexp "libri2rib" $ribval]} {
	set CFLAGS "$CFLAGS -DAYUSEAQSISRIB"
    } else {
	tk_messageBox -icon error -message "Invalid RIB-Writer selected (THIS SHOULD NOT HAPPEN)." -parent . -type ok
	return
    }

    if {[regexp "libslcargs" $shaderval]} {
	set CFLAGS "$CFLAGS -DAYUSESLCARGS"
    } elseif {[regexp "libslxargs" $shaderval]} {
	set CFLAGS "$CFLAGS -DAYUSESLXARGS"
    }

    if {$preview} {
	set CFLAGS "$CFLAGS -DAYENABLEPPREV"
    }

    if {$aqsis0719} {
	set CFLAGS "$CFLAGS -DAYOLDSLX"
    }

    if { $localtcl } {
	if { $tcl_version > 8.3 } {
	    set CFLAGS "$CFLAGS -DUSE_NON_CONST"
	}
    } else {
	set ver ${tclver}.${tclsub}
	if { $ver > 8.3 } {
	    set CFLAGS "$CFLAGS -DUSE_NON_CONST"
	}
    }

    set CFLAGS "$CFLAGS $cflags"

    # SHLFLAGS
    if {[regexp "MacOS" $osval]} {
	set SHLFLAGS "-bundle -flat_namespace -undefined suppress"
    } else {
	set SHLFLAGS "-shared"
    }

    # PREFIX
    set PREFIX $prefix

    # LDSWDYNAMIC
    switch {$osval} {
	"IRIX" { set LDSWDYNAMIC "-dynamic" }
	"default" { set LDSWDYNAMIC "-Wl,-Bdynamic" }
    }

    # EXLDFLAGS
    switch {$osval} {
	"IRIX" { set EXLDFLAGS "" }
	"Solaris" { set EXLDFLAGS "" }
	"MacOS" { set EXLDFLAGS "" }
	"default" { set EXLDFLAGS "-rdynamic"}
    }

    # TCLDIR
    set TCLDIR $tclpath

    # TCLINC
    set TCLINC "-I$tclpath/generic"

    # TCLLIB
    if { $localtcl } {
	set TCLLIB "-ltcl$tcl_version"
    } else {
	set TCLLIB "-L$TCLDIR/unix -ltcl$tclver.$tclsub"
    }

    # TKDIR
    set TKDIR $tkpath

    # TKINC
    set TKINC "-I$tkpath/generic -I$tkpath/unix"

    # TKLIB
    if { $localtcl } {
	set TKLIB "-ltk$tk_version"
    } else {	
	set TKLIB "-L$TKDIR/unix -ltk$tkver.$tksub"
    }

    # Affine
    if {[regexp "Affine" $ribval]} {
	set AFFINEOBJS "./affine/src/write.o ./affine/src/constant.o"
    } else {
	set AFFINEOBJS ""
    }

    # Aqsis
    if {[string length $aqsispath]} {
	set AQSISDIR $aqsispath
	set AQSISINCDIR "$AQSISDIR/include"
	set AQSISLIBDIR "$AQSISDIR/lib"

	if {[regexp "libri2rib" $ribval]} {
	    set AQSISRI2RIB  "$AQSISLIBDIR/libri2rib.so -L /usr/local/lib"
	    if {!$aqsis090} {
		set AQSISRI2RIB "$AQSISRI2RIB -llog4cpp"
	    }
	} else {
	    set AQSISRI2RIB ""
	}

	if {[regexp "libslxargs" $shaderval]} {
	    set AQSISOBJS "$AQSISLIBDIR/rcdummy.o $AQSISLIBDIR/libshadervm.so $AQSISLIBDIR/libshaderexecenv.so $AQSISLIBDIR/libslxargs.so $AQSISLIBDIR/libaqsistypes.so -L /usr/local/lib"
	    if {!$aqsis090} {
		set AQSISOBJS "$AQSISOBJS -llog4cpp"
	    }
	} else {
	    set AQSISOBJS ""
	}
    } else {
	set AQSISDIR ""
	set AQSISINCDIR ""
	set AQSISLIBDIR ""
	set AQSISRI2RIB ""
	set AQSISOBJS ""
    }

    # BMRT
    if {[string length $bmrtpath]} {
	set BMRTDIR $bmrtpath
	set BMRTINCDIR "$BMRTDIR/include"
	set BMRTLIBDIR "$BMRTDIR/lib"
	if {[regexp "libribout" $ribval]} {
	    set RIBOUTLIB "-L$BMRTLIBDIR -lribout"
	} else {
	    set RIBOUTLIB ""
	}

	if {[regexp "libslcargs" $shaderval] || !$bmrt25} {
	    set SLCARGSLIB "-L$BMRTLIBDIR -lslcargs"
	} else {
	    set SLCARGSLIB ""
	}
    } else {
	set BMRTDIR ""
	set BMRTINCDIR ""
	set BMRTLIBDIR ""
	set RIBOUTLIB ""
	set SLCARGSLIB ""
    }

    # DL
    if {$osval != "NetBSD"} {
	if {$osval == "Linux"} {
	    set DL "-ldl -lpthread"
	} else {
	    set DL "-ldl"
	}
    } else {
	set DL ""
    }

    #
    # Write Makefile now
    #

    if {[file exists "Makefile.shared"] && [file readable "Makefile.shared"]} {
	set src [open "Makefile.shared" r]
	set dst [open "Makefile" w]

	puts $dst "#################################################################"
	puts $dst "# WARNING, this is a generated Makefile, created by ayamConf.tcl."
	puts $dst "# If you edit, note that the variables are overwritten again"
	puts $dst "# in section \"Following lines are generated by ayamConf\"!"
	puts $dst "#################################################################"

	while {![eof $src]} {
	    gets $src l
	    if {[regexp "nothing needs to be changed below this line" $l]} {
		puts $dst "# Following lines are generated by ayamConf"
		puts $dst "#############################################"

		#
		# Let's rock!
		#

	puts $dst "CFLAGS=$CFLAGS"
		puts $dst "LD=$LD"
		puts $dst "PREFIX=$PREFIX"
		puts $dst "BINDIR=$PREFIX/bin"
		puts $dst "LIBDIR=$PREFIX/lib"
		puts $dst "DOCDIR=$PREFIX/doc"
		puts $dst "LDSWDYNAMIC=$LDSWDYNAMIC"
		puts $dst "EXLDFLAGS=$EXLDFLAGS"
		puts $dst "SHLFLAGS=$SHLFLAGS"
		puts $dst "TCLDIR=$TCLDIR"
		puts $dst "TCLINC=$TCLINC"
		puts $dst "TCLLIB=$TCLLIB"
		puts $dst "TKDIR=$TKDIR"
		puts $dst "TKINC=$TKINC"
		puts $dst "TKLIB=$TKLIB"
		puts $dst "DL=$DL"
		puts $dst "AFFINEOBJS=$AFFINEOBJS"
		puts $dst "AQSISDIR=$AQSISDIR"
		puts $dst "AQSISINCDIR=$AQSISINCDIR"
		puts $dst "AQSISLIBDIR=$AQSISLIBDIR"
		puts $dst "AQSISOBJS=$AQSISOBJS"
		puts $dst "AQSISRI2RIB=$AQSISRI2RIB"
		puts $dst "BMRTDIR=$BMRTDIR"
		puts $dst "BMRTINCDIR=$BMRTINCDIR"
		puts $dst "BMRTLIBDIR=$BMRTLIBDIR"
		puts $dst "RIBOUTLIB=$RIBOUTLIB"
		puts $dst "SLCARGSLIB=$SLCARGSLIB"

		puts $dst "#############################################"
	    }
	    puts $dst $l
	}

	close $src
	close $dst	
    } else {
	tk_messageBox -icon error -message "Cannot read Makefile.shared!" -parent . -type ok
	return
    }

    tk_messageBox -icon info -message "Makefile successfully created." -parent . -type ok
}

#
# Build the GUI
#

wm title . "ayamConf"

frame .s
frame .s.str

lappend strs [list cflags -O2 CFLAGS: 0 normal]
lappend strs [list prefix /usr/local Prefix: 1 normal]
lappend strs [list tclpath ../../tcl$tcl_patchLevel Tcl-Path:  1 normal]
lappend strs [list tkpath ../../tk$tk_patchLevel Tk-Path:  1 normal]
lappend strs [list bmrtpath "" BMRT-Path:  1 disabled]
lappend strs [list aqsispath "" Aqsis-Path:  1 disabled]

proc dirDialog {w ent} {
    set dir [tk_chooseDirectory -parent $w -mustexist true -initialdir [$ent get]]

    if [string compare $dir ""] {
	$ent delete 0 end
	$ent insert 0 $dir
	$ent xview end
    }
}

set j 0
foreach i $strs {
    set [lindex $i 0] [lindex $i 1]

    label .s.str.[lindex $i 0] -text [lindex $i 2]   -state [lindex $i 4]
    grid .s.str.[lindex $i 0] -row $j -column 0 -sticky "w"

    entry .s.str.[lindex $i 0]Entry -textvariable [lindex $i 0]  -state [lindex $i 4]
    grid .s.str.[lindex $i 0]Entry -row $j -column 1 -sticky "w"
    
    if {[lindex $i 3] && $tcl_version >= 8.3} {
	button .s.str.[lindex $i 0]Req -text "Browse" -command "dirDialog . .s.str.[lindex $i 0]Entry" -state [lindex $i 4]
	grid .s.str.[lindex $i 0]Req -row $j -column 2 -sticky "w"
    }
   
    incr j
}

frame .s.opts
pack .s.opts .s.str -side left

proc update_states {} {
    global ribval shaderval osval localtcl localtk .

    if {[regexp "BMRT" $ribval] || [regexp "BMRT" $shaderval]} {
	.s.opts.bmrt25 config -state normal
	.s.str.bmrtpath config -state normal
	.s.str.bmrtpathEntry config -state normal
	.s.str.bmrtpathReq config -state normal
    } else {
	.s.opts.bmrt25 config -state disabled
	.s.str.bmrtpath config -state disabled
	.s.str.bmrtpathEntry config -state disabled
	.s.str.bmrtpathReq config -state disabled
    }

    if {[regexp "Aqsis" $ribval] || [regexp "Aqsis" $shaderval]} {
	.s.opts.aqsis0719 config -state normal
	.s.opts.aqsis090 config -state normal
	.s.str.aqsispath config -state normal
	.s.str.aqsispathEntry config -state normal
	.s.str.aqsispathReq config -state normal
    } else {
	.s.opts.aqsis0719 config -state disabled
	.s.opts.aqsis090 config -state disabled
	.s.str.aqsispath config -state disabled
	.s.str.aqsispathEntry config -state disabled
	.s.str.aqsispathReq config -state disabled
    }

    if {[regexp "MacOS" $osval]} {
	.s.opts.useaqua config -state normal
    } else {
	.s.opts.useaqua config -state disabled
    }

}

set ribval $def_ribval
tk_optionMenu .s.opts.rib ribval dummy
.s.opts.rib.menu del 0
pack .s.opts.rib -fill x -padx 2 -pady 2
foreach i $rib {
    .s.opts.rib.menu add radio -label [lindex $i 0] -value [lindex $i 0] -variable ribval -command "update_states"
}

set shaderval $def_shaderval
tk_optionMenu .s.opts.shade shaderval dummy
.s.opts.shade.menu del 0
pack .s.opts.shade -fill x -padx 2 -pady 2
foreach i $shader {
    .s.opts.shade.menu add radio -label [lindex $i 0] -value [lindex $i 0] -variable shaderval -command "update_states"
}

set osval $def_osval
tk_optionMenu .s.opts.os osval dummy
.s.opts.os.menu del 0
pack .s.opts.os -fill x -padx 2 -pady 2
foreach i $o {
    .s.opts.os.menu add radio -label [lindex $i 0] -value [lindex $i 0] -variable osval -command "update_states"
}

lappend check [list useaqua "Use Aqua" 1 disabled]
lappend check [list localtcl "Use installed Tcl-library" 1 normal]
lappend check [list localtk "Use installed Tk-library" 1 normal]
lappend check [list bmrt25 "BMRT < 2.5" 0 disabled]
lappend check [list aqsis0719 "Aqsis < 0.7.19" 0 disabled]
lappend check [list aqsis090 "Aqsis > 0.9.0" 0 disabled]
lappend check [list preview "Enable permanent preview" 0 normal]

proc excl {i} {
    global aqsis0719 aqsis090

    if {[regexp "0\.7\.19" $i]} {
	set aqsis090 0
    }

    if {[regexp "0\.9\.0" $i]} {
	set aqsis0719 0
    }
}

foreach i $check {
    set [lindex $i 0] [lindex $i 2]
    checkbutton .s.opts.[lindex $i 0] -text [lindex $i 1] -variable [lindex $i 0] -relief flat -state [lindex $i 3] -command "excl \"[lindex $i 1]\""
    pack .s.opts.[lindex $i 0] -anchor w
}

pack .s
frame .buttons
pack .buttons -side bottom -padx 2m -pady 2m -fill x
button .buttons.create -text "Create Makefile" -command "create_makefile"
button .buttons.exit -text "Exit" -command "exit"
pack .buttons.create .buttons.exit -side left -expand 1
