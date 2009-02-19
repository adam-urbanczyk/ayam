#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

#
# useaqsisapp.tcl - arrange to run Aqsis from its app directory
#

# This script is for Mac OS X only!

# Usage: just add "useaqsisapp.tcl" to your Scripts preference setting

# Aqsis.app should be installed in /Applications, if you prefer a
# different location, just change the following path
# (the rest of the script will adapt):
set env(AQSIS_HOME) /Applications

if { ! [file exists "$env(AQSIS_HOME)/Aqsis.app" ] } {
    tk_messageBox -type ok -title Error -icon error -message "Aqsis.app not found in $env(AQSIS_HOME)!"
    return;
}

set env(PATH) "$env(PATH):$env(AQSIS_HOME)/Aqsis.app/Contents/MacOS"

set env(AQSIS_DISPLAY_PATH) "$env(AQSIS_HOME)/Aqsis.app/Contents/Resources/lib"

if { [string first "Aqsis.app" $ayprefs(Shaders)] == -1 } {
    set $ayprefs(Shaders) "$ayprefs(Shaders):$env(AQSIS_HOME)/Aqsis.app/Contents/Resources/shaders"
}

puts "Aqsis.app found and paths changed successfully."

if { [string first aqsis $ayprefs(Render)] == -1 } {
    puts "Be sure to also select Aqsis as renderer via main menu \"Special/Select Renderer\"!"
}

return;