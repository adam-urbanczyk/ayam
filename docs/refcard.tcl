# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# refcard.tcl - prettyprint current hot key configuration

# printHotKeys:
# creates a small (one page) LaTeX document containing some
# tables with the current hot keys
# USAGE:
# in the console, type:
# >source refcard.tcl; printHotKeys refcard.tex
# then, in a real shell:
# >latex refcard.tex
proc printHotKeys { filename } {
    global ay aymainshortcuts ayviewshortcuts

    set id [open $filename w]

    puts $id "\\documentclass\[11pt,twocolumn\]\{article\}"
    puts $id "\\title\{Ayam $ay(ay_version) Hot Keys\}"
    puts $id "\\date\{\}"
    puts $id "\\begin\{document\}"
    puts $id "\\topmargin -4cm"
    puts $id "\\maketitle\\vspace*\{-2cm\}"
    puts $id "\\thispagestyle \{empty\}"
    puts $id "\\enlargethispage \{10cm\}"
    puts $id "\\parindent 0cm"
################
    puts $id "\{\\bf File Operations\}\\\\ Scope: Main Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"
    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "New & \\verb|$aymainshortcuts(New)|\\\\"
    puts $id "\\hline"
    puts $id "Insert & \\verb|$aymainshortcuts(Insert)|\\\\"
    puts $id "\\hline"
    puts $id "Replace & \\verb|$aymainshortcuts(Replace)|\\\\"
    puts $id "\\hline"
    puts $id "Save & \\verb|$aymainshortcuts(Save)|\\\\"
    puts $id "\\hline"
    puts $id "Save as & \\verb|$aymainshortcuts(SaveAs)|\\\\"
    puts $id "\\hline"
    puts $id "Export RIB & \\verb|$aymainshortcuts(ExportRIB)|\\\\"
    puts $id "\\hline"
    puts $id "Quit & \\verb|$aymainshortcuts(Quit)|\\\\"
    puts $id "\\hline"
    puts $id "\\end\{tabular\}"

    puts $id "\\vspace \{0.5cm\}\\\\"
################
    puts $id "\{\\bf Edit Operations\}\\\\ Scope: Main Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"

    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Copy & \\verb|$aymainshortcuts(Copy)|\\\\"
    puts $id "\\hline"
    puts $id "Cut & \\verb|$aymainshortcuts(Cut)|\\\\"
    puts $id "\\hline"
    puts $id "Paste & \\verb|$aymainshortcuts(Paste)|\\\\"
    puts $id "\\hline"
    puts $id "Copy Property & \\verb|$aymainshortcuts(CopyP)|\\\\"
    puts $id "\\hline"
    puts $id "Copy Marked Prop& \\verb|$aymainshortcuts(ICopyP)|\\\\"
    puts $id "\\hline"
    puts $id "Paste Property& \\verb|$aymainshortcuts(PasteP)|\\\\"
    puts $id "\\hline"
    puts $id "Undo & \\verb|$aymainshortcuts(Undo)|\\\\"
    puts $id "\\hline"
    puts $id "Redo & \\verb|$aymainshortcuts(Redo)|\\\\"
    puts $id "\\hline"
    puts $id "Material & \\verb|$aymainshortcuts(Material)|\\\\"
    puts $id "\\hline"
    puts $id "Preferences & \\verb|$aymainshortcuts(Prefs)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Zap (Hide) Ayam& \\verb|$aymainshortcuts(Zap)|\\\\"
    puts $id "\\hline"
    puts $id "Apply & \\verb|$aymainshortcuts(Apply)|\\\\"
    puts $id "\\hline"
    puts $id "Switch Console & \\verb|$aymainshortcuts(SwCon)|\\\\"
    puts $id "\\hline"
    puts $id "Help & \\verb|$aymainshortcuts(Help)|\\\\"
    puts $id "\\hline"
    puts $id "\\end\{tabular\}\\\\"
#    puts $id "\newcolumn"
    puts $id "\\vspace \{0.5cm\}\\\\"
################
    puts $id "\{\\bf View Operations\}\\\\ Scope: View Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"
    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Quick Render& \\verb|$ayviewshortcuts(QRender)|\\\\"
    puts $id "\\hline"
    puts $id "Render & \\verb|$ayviewshortcuts(Render)|\\\\"
    puts $id "\\hline"
    puts $id "Redraw & \\verb|$ayviewshortcuts(Redraw)|\\\\"
    puts $id "\\hline"
    puts $id "Close & \\verb|$ayviewshortcuts(Close)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Make Front & \\verb|$ayviewshortcuts(Front)|\\\\"
    puts $id "\\hline"
    puts $id "Make Side & \\verb|$ayviewshortcuts(Side)|\\\\"
    puts $id "\\hline"
    puts $id "Make Top & \\verb|$ayviewshortcuts(Top)|\\\\"
    puts $id "\\hline"
    puts $id "Make Perspective & \\verb|$ayviewshortcuts(Persp)|\\\\"
    puts $id "\\hline"
    puts $id "Make Trim & \\verb|$ayviewshortcuts(Trim)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Halve Size & \\verb|$ayviewshortcuts(Halve)|\\\\"
    puts $id "\\hline"
    puts $id "Double Size & \\verb|$ayviewshortcuts(Double)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Toggle Redraw & \\verb|$ayviewshortcuts(Auto)|\\\\"
    puts $id "\\hline"
    puts $id "Toggle Shading & \\verb|$ayviewshortcuts(Shade)|\\\\"
    puts $id "\\hline"
    puts $id "Draw Grid & \\verb|$ayviewshortcuts(DGrid)|\\\\"
    puts $id "\\hline"
    puts $id "Use Grid & \\verb|$ayviewshortcuts(UGrid)|\\\\"
    puts $id "\\hline"
    puts $id "Set Gridsize & \\verb|$ayviewshortcuts(SGrid)|\\\\"
    puts $id "\\hline"
    puts $id "Zoom to Object & \\verb|$ayviewshortcuts(ZoomTO)|\\\\"
    puts $id "\\hline"
    puts $id "Align & \\verb|$ayviewshortcuts(Align)|\\\\"
    puts $id "\\hline"

    puts $id "\\end\{tabular\}"

    puts $id "\\newpage"
    puts $id "\\thispagestyle \{empty\}"
    puts $id "\\enlargethispage \{10cm\}"
################
    puts $id "\{\\bf Modelling Actions\}\\\\ Scope: View Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"
    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Move & \\verb|$ayviewshortcuts(MoveO)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate around Origin & \\verb|$ayviewshortcuts(RotO)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate around Point& \\verb|$ayviewshortcuts(RotA)|\\\\"
    puts $id "\\hline"
    puts $id "Scale 1D (X)& \\verb|$ayviewshortcuts(ScalX)|\\\\"
    puts $id "\\hline"
    puts $id "Scale 1D (Y)& \\verb|$ayviewshortcuts(ScalY)|\\\\"
    puts $id "\\hline"
    puts $id "Scale 1D (Z)& \\verb|$ayviewshortcuts(ScalZ)|\\\\"
    puts $id "\\hline"
    puts $id "Scale 2D & \\verb|$ayviewshortcuts(Scal2)|\\\\"
    puts $id "\\hline"
    puts $id "Scale 3D & \\verb|$ayviewshortcuts(Scal3)|\\\\"
    puts $id "\\hline"
    puts $id "Stretch 2D & \\verb|$ayviewshortcuts(Stretch)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Edit Point& \\verb|$ayviewshortcuts(Edit)|\\\\"
    puts $id "\\hline"
    puts $id "Directly Edit Point& \\verb|$ayviewshortcuts(EditD)|\\\\"
    puts $id "\\hline"
    puts $id "Edit Weight& \\verb|$ayviewshortcuts(WeightE)|\\\\"
    puts $id "\\hline"
    puts $id "Edit Weight& \\verb|$ayviewshortcuts(WeightE)|\\\\"
    puts $id "\\hline"
    puts $id "Select (tag) Points& \\verb|$ayviewshortcuts(Select)|\\\\"
    puts $id "\\hline"
    puts $id "Deselect all Points& \\verb|$ayviewshortcuts(DeSelect)|\\\\"
    puts $id "\\hline"
    puts $id "Insert Point (NC)& \\verb|$ayviewshortcuts(InsertP)|\\\\"
    puts $id "\\hline"
    puts $id "Delete Point (NC)& \\verb|$ayviewshortcuts(DeleteP)|\\\\"
    puts $id "\\hline"
    puts $id "Split Curve& \\verb|$ayviewshortcuts(SplitNC)|\\\\"
    puts $id "\\hline"
    puts $id "\\end\{tabular\}"

    puts $id "\\vspace \{1cm\}\\\\"
################
    puts $id "\{\\bf Camera Actions\}\\\\ Scope: View Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"
    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Rotate View & \\verb|$ayviewshortcuts(RotV)|\\\\"
    puts $id "\\hline"
    puts $id "Zoom View & \\verb|$ayviewshortcuts(ZoomV)|\\\\"
    puts $id "\\hline"
    puts $id "Move View& \\verb|$ayviewshortcuts(MoveV)|\\\\"
    puts $id "\\hline"
    puts $id "Move View (Z)& \\verb|$ayviewshortcuts(MoveZV)|\\\\"
    puts $id "\\hline"
    puts $id "\\hline"
    puts $id "Rotate View & \\verb|$ayviewshortcuts(RotMod)+Mouse-1|\\\\"
    puts $id "\\hline"
    puts $id "Zoom View & \\verb|Mouse-$ayviewshortcuts(ZoomVButton)|\\\\"
    puts $id "\\hline"
    puts $id "Move View & \\verb|Mouse-$ayviewshortcuts(MoveVButton)|\\\\"
    puts $id "\\hline"
    puts $id "\\end\{tabular\}"

    puts $id "\\vspace \{1cm\}\\\\"

    puts $id "\{\\bf Miscellaneous\}\\\\ Scope: View Window\\\\"
    puts $id "\\begin\{tabular\}\{\|l\|l\|\}"
    puts $id "\\hline"
    puts $id "Action & Key\\\\"
    puts $id "\\hline"
    puts $id "Break Action& \\verb|$ayviewshortcuts(Break)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate View (Y, 5Deg)& \\verb|$ayviewshortcuts(RotL)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate View (Y, -5Deg)& \\verb|$ayviewshortcuts(RotR)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate View (X, 5Deg)& \\verb|$ayviewshortcuts(RotU)|\\\\"
    puts $id "\\hline"
    puts $id "Rotate View (X, -5Deg)& \\verb|$ayviewshortcuts(RotD)|\\\\"
    puts $id "\\hline"
    puts $id "Zoom View In& \\verb|$ayviewshortcuts(ZoomI)|\\\\"
    puts $id "\\hline"
    puts $id "Zoom View Out& \\verb|$ayviewshortcuts(ZoomO)|\\\\"
    puts $id "\\hline"
    puts $id "\\end\{tabular\}"

    puts $id "\\end\{document\}"
    close $id

 return;
}
