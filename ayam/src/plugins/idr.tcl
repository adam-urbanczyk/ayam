# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# idr.tcl - Importance Driven Rendering

global idrprefs

array set idrprefs {

    IDRBase "/tmp/idr"
    Mode 0
    QLevels 2
    Weight_R 24000
    ShowResult 0
    UseCurrentBG 0
    CacheParts 1

    PropRadius 1.0

    View ""

    Importance0 0.0
    HalfRes0 0
    ExcludeOther0 0
    OptimizeBB0 0
    Renderer0 "rgl -dumprgba"

    Importance1 1.0
    HalfRes1 0
    ExcludeOther1 0
    OptimizeBB1 1
    Renderer1 "rendrib"


}

variable idrmode 0

# idr_open:
#
#
proc idr_open {} {
    global ay idrprefs

    set w .idrw
    catch {destroy $w}
    toplevel $w
    wm title $w "Ayam IDR"
    wm iconname $w "Ayam"
    
    set f [frame $w.fu -bd 2 -relief sunken]
    pack $f -in $w -side top -fill both -expand yes

    set f [frame $w.fu.fl -relief ridge -bd 2]
    pack $f -side left -fill y -expand yes
    set f [frame $w.fu.fr -relief ridge -bd 2]
    pack $f -side right -fill y -expand yes



    # select view
    set f $w.fu.fl
    set l [label $f.l -text "Select View:"]
    set f [frame $f.fview]
    scrollbar $f.sc -command {.idrw.fu.fl.fview.lb yview} -takefocus 0
    listbox $f.lb -width 15 -height 5 -selectmode single\
	    -exportselection 0 -yscrollcommand {.idrw.fu.fl.fview.sc set}

    set index 0
    set firstselect 0
    foreach i $ay(views) {
	eval [subst "$i.f3D.togl mc"]

	if { $idrprefs(View) != "" } {
	    if { $idrprefs(View) == $i } {
		set firstselect $index
	    }
	} else {
	    if { $ay(cVType) == 3 } { set firstselect $index }
	}

	scan $i ".%s" name
	if { $ay(cVType) != 4 } {
	    $f.lb insert end "$name ([lindex $ay(viewtypenames) $ay(cVType)])"
	    incr index
	}


    }

    #    catch {eval [subst "$currentView getViewConfig"]}

    $f.lb selection set $firstselect

    pack $f.lb -in $f -side left -fill both -expand yes
    pack $f.sc -in $f -side right -fill y
    pack $l -side top
    pack $f -side top -fill x -expand yes
#


    set f $w.fu.fr
    set bboptimize_entries {None Combine Optimize}
    # Low Quality Rendering Parameters
    set f1 [frame $f.f1]
    addText $f1 e1 "LQ Params:"
    addCheck $f1 idrprefs ExcludeOther0
    addCheck $f1 idrprefs HalfRes0
    addMenu $f1 idrprefs OptimizeBB0 $bboptimize_entries
    addString $f1 idrprefs Renderer0 [list "rgl -dumprgba" \
"rgl -sketch -dumprgba -rd 5" "rendrib"]
    pack $f1 -side top

    # High Quality Rendering Parameters
    set f1 [frame $f.f2]
    addText $f1 e1 "HQ Params:"
    addCheck $f1 idrprefs ExcludeOther1
    addCheck $f1 idrprefs HalfRes1
    addMenu $f1 idrprefs OptimizeBB1 $bboptimize_entries
    addString $f1 idrprefs Renderer1 [list "rendrib" "rgl -dumprgba" \
"rgl -sketch -dumprgba -rd 5"]
    pack $f1 -side top
    set f1 [frame $f.f3]
    addText $f1 e1 "Global Parameters:"
    addCheck $f1 idrprefs CacheParts
    pack $f1 -side top -fill x -expand yes

    set f $w.fu.fl
    # Global IDR Parameters?
    set f1 [frame $f.f1]
    addText $f1 e1 "Assign Importance:"
    addMenu $f1 idrprefs Mode {Manual Selection Changed 2DRegions 3DRegions}
    pack $f1 -side top -fill x -expand yes

    # Actions!
    set f1 [frame $f.f2]
    addText $f1 e1 "IDR Actions:"
    addCommand $f1 c0 "DefineRegion" {
	set selection ""
	set selection [.idrw.fu.fl.fview.lb curselection]
	if { $selection != "" } {
	    set sname [.idrw.fu.fl.fview.lb get $selection]
	    scan $sname "%s (%s)" name type
	    idr_actionDefReg .$name.f3D.togl

	}
    }
    addCommand $f1 c1 "DeleteRegions" {
	set selection ""
	set selection [.idrw.fu.fl.fview.lb curselection]
	if { $selection != "" } {
	    set sname [.idrw.fu.fl.fview.lb get $selection]
	    scan $sname "%s (%s)" name type
	    idr_actionDefReg .$name.f3D.togl del

	}
    }

    addParam $f1 idrprefs PropRadius

    addCommand $f1 c2 "Propagate Distance" {
	global idrprefs
	idr_propDist $idrprefs(PropRadius)
    }
    addCommand $f1 c3 "Propagate Parent" {
	global idrprefs
	idr_propDist $idrprefs(PropRadius) 1
    }

    pack $f1 -side top -fill x -expand yes


    set f [frame $w.fl]
    button $f.bgo -text "Render!" -width 6 -command { 
	set selection ""
	set selection [.idrw.fu.fl.fview.lb curselection]
	if { $selection != "" } {
	    set sname [.idrw.fu.fl.fview.lb get $selection]
	    scan $sname "%s (%s)" name type
	    idr_run $name

	}
    }
    button $f.bca -text "Dismiss" -width 6 -command { 
	destroy .idrw
    }
    pack $f.bgo $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w

    focus $f.bca

 return;
}
# idr_open

proc idr_run { view } {
    global idrprefs

    if { $view == "" } {
	ayError 2 "idr_run" "No view selected!"
	return;
    }

    if { [winfo exists .$view] == 0 } {
	ayError 2 "idr_run" "Window does not exist, rebuilding GUI..."
	idr_open
	return;
    }
    
    set t [time {

	.$view.f3D.togl mc
	.$view.f3D.togl idr_wrib
	set i 0
	while { $i < $idrprefs(QLevels) } {

	    foreach file $idrprefs(Files${i}) {
		
		    set needrender 1

		    if { ( $idrprefs(CacheParts) == 1 ) &&\
		         ( [file exists ${file}.bak] == 1 ) } {

		       set res [ catch { exec diff $file ${file}.bak } ]
		       if { $res == 0 } {
			    set needrender 0
		       }
		    }

		    if { $needrender == 1 } {

			puts [subst "$idrprefs(Renderer${i}) $file"]

			catch {
			    eval [subst "exec $idrprefs(Renderer${i}) $file"]
			}
			if { $idrprefs(CacheParts) == 1 } {
			    file rename -force $file ${file}.bak
			}
		    } else {
			puts "Using cached image for $file"
		    }
		    
		}
		incr i
	}
    }
    ]
    set v [lindex $t 0]
    puts stdout "Rendering time: [expr $v/1000000] Seconds"

    set width 0
    set height 0
    set pdata ""
    idrCombineResults width height pdata
    if { $pdata != "" } {
	catch {image delete idrresult}
	image create photo idrresult -width $width -height $height
	idrresult put $pdata

	catch {destroy .idr_result}
	set w .idr_result
	toplevel $w -visual truecolor
	wm iconname $w "IDR"
	frame $w.m -bd 2 -relief raised
	frame $w.picture
	pack $w.m -side top -fill x
	pack $w.picture -in $w -fill both -expand yes
	menubutton $w.m.f -text "File" -menu $w.m.f.m
	pack $w.m.f -in $w.m -side left
	menu $w.m.f.m
	$w.m.f.m add command -label "Save as ..." -command {
	    
	    set types {{"Image" ".tif"} {"All files" *}}
	    set newfilename [tk_getSaveFile -filetypes $types\
		    -parent . -title "Choose destination file:"]
	    if { $newfilename != "" } {
		puts stdout "Saving IDR result to $newfilename"
		idrsaveResult $newfilename
	    }
	}
	$w.m.f.m add command -label "Close" -command {
	    image delete idrresult
	    destroy .idr_result
	}
	label $w.picture.l -text Ergebnis: -image idrresult
	pack $w.picture.l -in $w.picture
    }

}
# idr_run

proc setRenderstarttime { } {
 global Weight_R

    set w .idropt
    catch {destroy $w}
    toplevel $w
    wm title $w "Ayam - Importance driven rendering"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f]
    pack $f -in $w -side top -fill x
    text $f.t -wrap word -width 25 -height 8 -borderwidth 0
    $f.t insert end "This value shold express the time factor between rendering one pixel and starting the high quality renderer (5000 means: starting the renderer is equivalent to render 5000 pixels)"
    $f.t configure -state disabled
    pack $f.t -in $f -side top -fill both -expand yes
    entry $f.e
    $f.e delete 0 end
    $f.e insert end $Weight_R
    $f.e selection range 0 end
    pack $f.e -in $f -side top -fill x -expand yes
    button $f.bok -text "Ok" -width 5 -command {set Weight_R [.idropt.f.e get]
       						destroy .idropt}
    pack $f.bok -in $f -side top -fill x -expand yes

    centerWindowp $w
    grab $w
    focus $f.e
    tkwait window $w

 return;
}
# setRenderStartTime


# idr_actionDefReg
#
proc idr_actionDefReg { w {del ""} } {

    if { $del != "" } {

	$w idr_defreg del
	return;
    }

    viewTitle $w "" "Define_Region"

    bind $w <ButtonPress-1> {
#	undo save
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y)} {
	    %W idr_defreg %x %y $oldx $oldy 1.0
	}
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 0
	rV
	update
    }

    bind $w <B1-Motion> {
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 1
    }

}
# idr_actionDefReg

# for making changed objects important
rename treeSelect oldtreeSelect
rename idr_treeSelect treeSelect

global ay
$ay(cm) add command -label "Open IDR" -command "idr_open"
