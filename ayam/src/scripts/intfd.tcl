# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# intfd.tcl - switch to internal file dialogs

proc tk_getOpenFile {args} { return [eval [subst "::tk::dialog::file:: open $args"]] }

proc tk_getSaveFile {args} { return [eval [subst "::tk::dialog::file:: save $args"]] }

proc tk_chooseColor {args} { return [eval [subst "::tk::dialog::color:: $args"]] }

ayError 4 "intfd" "Enabled."
