# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# winheader.tcl - create import/export versions of the Ayam headers
# for compilation on Win32

# Usage:
# > cd ayam/src
# > wish scripts/winheader.tcl
# Note: sub-directories "imp" / "exp" have to exist!

set modules [list aycore.h contrib.h nurbs.h objects.h]

# export headers (for Ayam)

# mangle ayam.h
set infile [ open ayam.h r ]
set outfile [ open [file join "exp" ayam.h] w ]
while { ![eof $infile] } {
    gets $infile in
    if { ![string compare -length 6 $in "extern"] } {
	set in [string range $in 7 end]
	set out "__declspec(dllexport) $in"
	puts $outfile $out
    } else {
	puts $outfile $in
    }
}
close $infile
close $outfile

# mangle module headers
foreach module $modules {
    set infile [ open $module r ]
    set outfile [ open [file join "exp" $module] w ]
    while { ![eof $infile] } {
	gets $infile in
	set first [string index $in 0]
	if { ($first == "i") || ($first == "v") || ($first == "d") ||
	     ($first == "c") } {
	    set out "__declspec(dllexport) $in"
	    puts $outfile $out
	} else {
	    puts $outfile $in
	}
    }
    close $infile
    close $outfile
}

# import headers (for plugins)

# mangle ayam.h
set infile [ open ayam.h r ]
set outfile [ open [file join "imp" ayam.h] w ]
while { ![eof $infile] } {
    gets $infile in
    if { ![string compare -length 6 $in "extern"] } {
	set in [string range $in 7 end]
	set out "__declspec(dllimport) $in"
	puts $outfile $out
    } else {
	puts $outfile $in
    }
}
close $infile
close $outfile

# mangle module headers
foreach module $modules {
    set infile [ open $module r ]
    set outfile [ open [file join "imp" $module] w ]
    while { ![eof $infile] } {
	gets $infile in
	set first [string index $in 0]
	if { ($first == "i") || ($first == "v") || ($first == "d") ||
	     ($first == "c") } {
	    set out "__declspec(dllimport) $in"
	    puts $outfile $out
	} else {
	    puts $outfile $in
	}
    }
    close $infile
    close $outfile
}

# remove original headers
set t "Remove Headers?"
set m "Headers have been mangled successfully.\nSelect Ok to remove the\
original headers!"
set answer [tk_messageBox -title $t -type okcancel -icon warning -message $m]
if { $answer == "ok" } {
    file delete -force ayam.h
    foreach module $modules {
	file delete -force $module
    }
}

# all done
exit