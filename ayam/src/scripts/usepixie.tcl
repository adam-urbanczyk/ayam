#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

#
# usepixie.tcl - arrange to run Pixie from the /Library directory
#

# This script is for Mac OS X only!

# Usage: just add "usepixie.tcl" to your Scripts preference setting

# Pixie should be installed in /Library, if you prefer a
# different location, just change the following path
# (the rest of the script will adapt):
set env(PIXIEHOME) /Library/pixie

if { ! [file exists "$env(PIXIEHOME)" ] } {
    tk_messageBox -type ok -title Error -icon error -message "Pixie not found
in $env(PIXIEHOME)!"
    return;
}

set env(PATH) "$env(PATH):$env(PIXIEHOME)/bin"

if { $env(DYLD_LIBRARY_PATH) != "" } {
    set env(DYLD_LIBRARY_PATH) "$env(DYLD_LIBRARY_PATH):$env(PIXIEHOME)/lib"
} else {
    set env(DYLD_LIBRARY_PATH) "$env(PIXIEHOME)/lib"
}

if { [string first "$env(PIXIEHOME)/shaders" $ayprefs(Shaders)] == -1 } {
    set $ayprefs(Shaders) "$ayprefs(Shaders):$env(PIXIEHOME)/shaders"
}

puts "Pixie found and paths changed successfully."

if { [string first rndr $ayprefs(Render)] == -1 } {
    puts "Be sure to also select Pixie as renderer via main menu \"Special/Select Renderer\"!"
}

return;