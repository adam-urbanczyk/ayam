#!/usr/bin/env wish
#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# bgconvert.tcl - use Ayam as background converter

# Usage:
#  start Ayam; then, in a shell, type:
#   bgconvert.tcl infile.rib outfile.dxf
#  or (with options):
#   bgconvert.tcl "infile.rib -p 1" outfile.obj

if { $argc < 2 } {
    puts stderr "Error, need two file names!"
    exit 1;
}

#
proc fileNameToPluginName { filename } {
    # extend as needed
    set extensions { ".rib" ".3dm" ".obj" ".3dmf" ".mop" ".dxf" ".x3d" }
    set plugins { "rrib" "onio" "objio" "mfio" "mopsi" "dxfio" "x3dio" }

    set ext [file extension $filename]
    if { $ext != "" } {
	set i 0
	set supported 0
	foreach sext $extensions {
	    # look for matching supported file name extension
	    if { ! [string compare -nocase $ext $sext] } {
		# get the name of the plugin that supports $ext files
		set plugin [lindex $plugins $i]
		set supported 1
	    }
	    # if
	    if { $supported } {
		break
	    }
	    incr i
	}
	# foreach
	if { ! $supported } {
	    puts stderr "Error, unknown or unsupported extension: $ext."
	    return "";
	}
	return $plugin;
    } else {
	puts stderr "Error, need file name extension!"
	return "";
    }
}
# fileNameToPluginName

set infile [lindex [lindex $argv 0] 0]
set inplugin [fileNameToPluginName $infile]

if { $inplugin == "" } { exit }

set outfile [lindex [lindex $argv 1] 0]
set outplugin [fileNameToPluginName $outfile]

if { $outplugin == "" } { exit }

set cscript "\
if { ! \[info exists \"${inplugin}Read\"\] } { \
   set ::ay(autoload) ${inplugin}; \
   io_lcAuto; \
}; \
if { ! \[info exists \"${outplugin}Write\"\] } { \
   set ::ay(autoload) ${outplugin}; \
   io_lcAuto; \
}; \
"
append cscript "newScene; cd [pwd];"
append cscript "${inplugin}Read [lindex $argv 0];"

# this is the right place to append some scene processing,
# like conversion to polygons:
#append cscript "toPoly;"

append cscript "${outplugin}Write [lindex $argv 1];"

# do we run in wish?
if { [string first wish [file tail [info nameofexecutable]]] != -1 } {
    # yes, send the script to Ayam (via Tk send)
    puts "Sending conversion script to Ayam..."
    send ayam $cscript
    exit
}

# do we run in Wish Shell (on Mac OS X Aqua)?
if { [string first Wish [file tail [info nameofexecutable]]] != -1 } {
    # yes, send the script to Ayam (via AppleScript event)
    set script [subst "tell application \"Ayam\"\n  do script \"$cscript\"\nend tell"]
    puts "Sending conversion script to Ayam..."
    exec osascript -e $script
    exit
}
