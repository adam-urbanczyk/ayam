# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# render.tcl - invoke an external renderer

#XXXX does not work this way, see run.tcl
proc render { args } {
    global ay ayprefs

    set command "|"
    append command [lindex $args 0]
    append command " 2>@ stderr &"

    set f [open $command r]
    fileevent $f readable "gotline $f"
    proc gotline f {
	if {[gets $f line]<0} {
	    # it died !
	    close $f ;      # catch to get exit status
	    return ;        # (see errorCode in tclvars.n)
	}
	# use $line !
	puts $line
    }

}
# render

proc render_select { } {
    global ay ayprefs

    set w .selRenw
    catch {destroy $w}
    toplevel $w
    wm title $w "Select Renderer"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.flb]
 
    label $f.l -text "Select Renderer:"
    pack $f.l -in $f -side top

 
    # scrollbar
    scrollbar $f.sc -command "$f.li yview" -takefocus 0
    pack $f.sc -in $f -side right -fill y -expand no

    # listbox
    pack $f -in $w -side top -fill both -expand yes
    listbox $f.li -width 24 -height 10 -selectmode single\
	-yscrollcommand "$f.sc set" -exportselection 0
    pack $f.li -in $f -side left -fill both -expand yes

    pack $f -in $w -side top -fill x

    set names [list BMRT2.5 BMRT2.6 Aqsis Air 3Delight RDC PRMan]
    foreach name $names {
	$f.li insert end "$name"
    }

    if { [info exists ay(srr)] } {
	$f.li selection set $ay(srr)
    }

    # checkbutton
    set f [frame $w.fch]
    addCheck $f ay ScanShaders
    pack $f -in $w -side top -fill x
    
    # ok/cancel buttons
    set f [frame $w.fb1]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command {
	global ay ayprefs

	set sel [.selRenw.flb.li curselection]

	switch $sel {

	    0 {
		# BMRT2.5
		set ayprefs(QRender) "rgl -rd 4 %s"
		set ayprefs(QRenderPT) ""
		set ayprefs(Render) "rendrib -d 4 %s"
		set ayprefs(RenderPT) "Done computing %d"
		set ayprefs(SMRender) "rendrib %s"
		set ayprefs(SMRenderPT) "Done computing %d"
		set ayprefs(RenderMode) 0
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ""
		}
		set newr "BMRT2.5"
	    }

	    1 {
		# BMRT2.6
		set ayprefs(QRender) "rgl -rd 4 %s"
		set ayprefs(QRenderPT) ""
		set ayprefs(Render) "rendrib -d 4 -Progress %s"
		set ayprefs(RenderPT) "R90000 %d"
		set ayprefs(SMRender) "rendrib -Progress %s"
		set ayprefs(SMRenderPT) "R90000 %d"
		set ayprefs(RenderMode) 0
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ""
		}
		set newr "BMRT2.6"
	    }

	    2 {
		# Aqsis
		set ayprefs(QRender) "aqsis -progress -fb %s"
		set ayprefs(QRenderPT) "Done Computing %d"
		set ayprefs(Render) "aqsis -progress -fb %s"
		set ayprefs(RenderPT) "Done Computing %d"
		set ayprefs(SMRender) "aqsis -progress %s"
		set ayprefs(SMRenderPT) "Done Computing %d"
		set ayprefs(RenderMode) 0
		set splugin "ayslx"
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ".slx"
		}
		set newr "Aqsis"
	    }

	    3 { 
		# Air
		set ayprefs(QRender) "air -samples 1 1 -d 4 -Progress %s"
		set ayprefs(QRenderPT) "R90000 %d"
		set ayprefs(Render) "air -d 4 -Progress %s"
		set ayprefs(RenderPT) "R90000 %d"
		set ayprefs(SMRender) "air -Progress %s"
		set ayprefs(SMRenderPT) "R90000 %d"
		set ayprefs(RenderMode) 0
		set splugin "ayslb"
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ".slb"
		}
		set newr "Air"
	    }

	    4 {
		# 3Delight
		set ayprefs(QRender) "renderdl -d %s"
		set ayprefs(QRenderPT) ""
		set ayprefs(Render) "renderdl -d %s"
		set ayprefs(RenderPT) ""
		set ayprefs(SMRender) "renderdl %s"
		set ayprefs(SMRenderPT) ""
		set ayprefs(RenderMode) 0
		set splugin "ayslo3d"
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ".slo"
		}
		set newr "3Delight"
	    }

	    5 {
		# RDC
		set ayprefs(QRender) "renderdc %s"
		set ayprefs(QRenderPT) ""
		set ayprefs(Render) "renderdc %s"
		set ayprefs(RenderPT) ""
		set ayprefs(SMRender) "renderdc %s"
		set ayprefs(SMRenderPT) ""
		set ayprefs(RenderMode) 1
		set splugin "ayso"
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ".so"
		}
		set newr "RDC"
	    }

	    6 {
		# PRMan
		set ayprefs(QRender) "render -vector %s"
		set ayprefs(QRenderPT) ""
		set ayprefs(Render) "render -progress %s"
		set ayprefs(RenderPT) " %d"
		set ayprefs(SMRender) "render -progress %s"
		set ayprefs(SMRenderPT) " %d"
		set ayprefs(RenderMode) 1
		set splugin "ayslo"
		if { $ay(ScanShaders) == 1 } {
		    set ay(sext) ".slo"
		}
		set newr "PRMan"
	    }

	    default { puts Whoops? }
	}
	# switch

	if { $ay(ScanShaders) == 1 } {
	    if { $sel > 1 } {
		set ay(autoload) $splugin
		io_lcAuto
	    }
	    scanAllShaders
	}

	ayError 4 "Renderer_Select" "Now using \$newr to render."

	set ay(srr) $sel

	focus .
	grab release .selRenw
	destroy .selRenw
    }
    #-command


    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	focus . ; destroy $w "
    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x

    # center window
    winCenter $w

    grab $w
    focus $f.bok
    tkwait window $w

 return;
}
# render_select

