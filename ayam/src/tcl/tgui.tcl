# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.



uplevel #0 { array set tgui_tessparam {
    SMethod 0
    SParam 20
}
}

# tgui_update:
#
#
proc tgui_update args {
    global ay tgui_tessparam

     tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParam)
    .tguiw.f1.fSParam.e delete 0 end
    .tguiw.f1.fSParam.e insert 0 $tgui_tessparam(SParam)

 return;
}
# tgui_update


# tgui_open:
#
#
proc tgui_open { } {
    global ay tgui_tessparam

    tguiCmd in

    set w .tguiw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Tesselation Parameters"
    wm iconname $w "Ayam"
    #wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x
    addMenu $f tgui_tessparam SMethod $ay(smethods)
    trace variable tgui_tessparam(SMethod) w tgui_update
    set f [frame $f.fSParam]
    label $f.l -text "SParam"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-variable tgui_tessparam(SParam)\
	-command tgui_update
    entry $f.e -width 5
    trace variable tgui_tessparam(SParam) w tgui_update
    pack $f.l $f.s $f.e -in $f -side left -fill x -expand yes
    pack $f -in $w.f1 -side top -fill x -expand yes
    tgui_update

    event add <<CommitTG>> <KeyPress-Return> <FocusOut>
    bind $f.e <<CommitTG>> "$f.s set \[$f.e get\]"

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	tguiCmd op; grab release .tguiw; focus .; destroy .tguiw;
	uCL cl; plb_update
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command {
	tguiCmd ca; grab release .tguiw; focus .; destroy .tguiw
    }
    # button

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    after idle viewMouseToCurrent

 return;
}
# tgui_open