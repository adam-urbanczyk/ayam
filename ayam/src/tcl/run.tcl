# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# run.tcl - run tools
# runRenderer and auxiliary procs (runGetStdout, runGetStderr) are
# modified versions of procedures posted by
# MarK Stucky (mstucky5@home.com)
# in comp.lang.tcl (MID:"KgYT5.74231$P82.8003950@news1.rdc1.ct.home.com")


###################################
# runTool:
# execute a tool, asking for arguments
# argvars:    list of global varnames
# argstrings: labels for the argument entry fields
# command:    the command that calls the tool, where
#              "%0" will be substituted with first argument etc.
# example:
# runTool splitCurveu {"Split at u:"} "splitCurve %0"
proc runTool { argvars argstrings command } {
global ay
set w .rtw
catch {destroy $w}
toplevel $w
wm title $w "Ayam"
wm iconname $w "Ayam"
wm transient $w .

set f [frame $w.f1]
pack $f -in $w -side top -fill x


set okscript ""
append okscript "set command \"$command\";"

set index 0
foreach i $argvars {
    global $i
    # check variable
    if { ![info exists $i] } { set $i 0}

    # create GUI
    set f2 [frame $f.f$index]

    label $f2.l -text [lindex $argstrings $index ] -width 12
    entry $f2.e -width 12

    eval [subst "$f2.e insert @0 \$$i"]

    pack $f2.l $f2.e -in $f2 -padx 2 -pady 2 -side left -expand yes -fill x
    pack $f2 -in $w.f1 -side top -expand yes -fill x
    # and simultaneously create a script
    append okscript "global $i ay; set $i \[$f2.e get\];update;"
    append okscript "regsub -all \"%$index\" \$command \[$f2.e get\] command;"
    incr index
}

append okscript "eval \"\$command\";"
append okscript "grab release .rtw; focus .; destroy .rtw"

set f [frame $w.f2]
button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command $okscript

button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	focus . ; destroy $w"

pack $f.bok $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side top -anchor n -fill x -expand yes

winCenter $w
grab $w
focus $f.bok
tkwait window $w

return;
}
# runTool


###################################
# runGetStderr:
# 
proc runGetStderr { num cmd channel } {

    if { [eof $channel] } {
        # program completed
        catch { close $channel }
    } else {
        set xx [gets $channel]
	if { $xx != "" } { 
	    ayError 2 $cmd $xx
	}
    }
}
# runGetStderr


###################################
# runGetStdout:
# 
proc runGetStdout { num cmd template channel } {
    global ayprefs ay
    if { [eof $channel] } {
        # program completed => invoke cancel button to properly
	# clean up all processes and destroy the RenderGUI window
	catch { .render${num}.bca invoke }
    } else {
        set xx [gets $channel]

	set percent 0
	if { $ayprefs(RenderPT) != "" } {
	    scan $xx $template percent
	} else {
	    ayError 4 $cmd $xx
	}

	update
	if { $percent != 0 } {
	    catch { SetProgress .render${num}.f1.p $percent }

	    set cur [clock seconds]
	    set start $ay(rstarttime${num})
	    set fulltime [expr ($cur-$start)*100.0/$percent]
	    set togo [expr (100.0-$percent)*$fulltime/100.0]
	    set hours [expr int(floor($togo/3600))]
	    set mins [expr int(floor(($togo-($hours*3600))/60))]
	    set secs [expr int(round($togo-($hours*3600)-($mins*60)))]
	    set string [format "~ %d:%02d:%02d to go"  $hours $mins $secs]
	    catch { .render${num}.f2.la configure -text $string }
	    if { $percent >= 100 } {
		set fulltime [expr ($cur-$start)]
		set hours [expr int(floor($fulltime/3600))]
		set mins [expr int(floor(($fulltime-($hours*3600))/60))]
		set secs [expr int(round($fulltime-($hours*3600)-($mins*60)))]
		set string [format "%d:%02d:%02d elapsed"  $hours $mins $secs]
		catch { .render${num}.f2.la configure -text $string }
		
		if {$ay(renderbeep${num})} {bell}

	    }
	    # if

	}
	# if
	update
    }
    # if
}
# runGetStdout


###################################
# runRenderer:
# 
proc runRenderer { cmd template } {
    global ay ayprefs

    if { $ayprefs(Kill) != "" } {
	set kill $ayprefs(Kill)
    } else {
	set kill "kill"
    }
    if { $ayprefs(Cat) != "" } {
	set cat $ayprefs(Cat)
    } else {
	set cat "cat"
    }

    set wait $ayprefs(Wait)

    incr ay(rnum)

    set ay(rstarttime$ay(rnum)) [clock seconds]

    set ioPipe [open "|$cat" r+]
    fconfigure $ioPipe -buffering none -blocking 0
    fileevent $ioPipe readable "\
	runGetStderr $ay(rnum) [lindex $cmd 0] $ioPipe"
    
    set ioFid [open "|$cmd 2>@$ioPipe" r+]

    fconfigure $ioFid -buffering none -blocking 0
    fileevent $ioFid readable "\
     runGetStdout $ay(rnum) [lindex $cmd 0] \"$template\" $ioFid"

    global pids
    set pids [list [pid $ioPipe]]
    lappend pids [pid $ioFid]

    set w .render$ay(rnum)
    catch {destroy $w}

    toplevel $w
    wm title $w "Render_$ay(rnum)"
    wm iconname $w "Ayam"
    if { $ayprefs(RGTrans) == 1 } {
	wm transient $w .
    }

    set f [frame $w.f1 -border 10]

    Progress $f.p
    pack $f.p -in $f -side top -fill x -expand yes
    pack $f -in $w -side top -fill x

    set f [frame $w.f2]
    label $f.la -text "~ 0:00:00 to go"
    pack $f.la -in $f -side left -fill x -expand yes

    set ay(renderbeep$ay(rnum)) 0
    global tcl_platform
    if { $tcl_platform(platform) == "windows" } {
	checkbutton $f.cb -image emptyimg -variable ay(renderbeep$ay(rnum))\
		-bd 1 -indicatoron 0 -selectcolor #b03060
	balloon_set $f.cb "Beep when finished."
	pack $f.cb -in $f -side left -pady 2 -expand yes -anchor center
    } else {
	checkbutton $f.cb -variable ay(renderbeep$ay(rnum))
	balloon_set $f.cb "Beep when finished."
	pack $f.cb -in $f -side left -fill x -expand yes
    }
    pack $f -in $w -side top -fill x

    button $w.bca -text "Cancel!" -width 16 -command "\
	    set dontwait 0;\
	    foreach i {$pids} {\
	     if { \"$kill\" == \"w32kill\" } {\
	      catch \{ w32kill \$i; \} result } else {\
	      catch \{ exec $kill \$i \} result;\
	     };\
	     if { \$result != \"\" } {\
	      puts stderr \"\$result\"; set dontwait 1\
             };\
            };\
	    if { !\$dontwait } {
	     foreach i {$pids} {\
	      if { \"$wait\" != \"\" } {\
	      catch \{ $wait \$i \};
              };\
             };\
	    };\
	    catch \{ fileevent $ioPipe readable \"\" \} ;\
	    catch \{ fileevent $ioFid readable \"\" \} ;\
	    focus .;\
	    destroy $w"
    pack $w.bca -in $w -side bottom -anchor s -fill x -expand yes

    winCenter $w

    # auto raise window, when obscured
    bind $w <Visibility> {
	global ay
	# are we obscured?
	if { "%s" == "VisibilityPartiallyObscured" ||\
		"%s" == "VisibilityFullyObscured" } {
	    # yes: try to raise the window, but just one time
	    raise [winfo toplevel %W]
	    bind %W <Visibility> ""
	}
    }

    # Esc-Key == Cancel button
    bind $w <Escape> "$w.bca invoke"

    # disable close button of window decoration
    # (if we would allow to close the dialog, before rendering
    # finishes or the cancel button is used, we would not be able
    # to properly clean up all started processes, because unfortunately
    # (or better, stupidly ;)) the necessary information is tied to the
    # cancel button)
    # XXXX reorganize the code, so that the pids are not bound
    # to the cancel button, then enable premature closing of the RenderGUI
    # window again
    wm protocol $w WM_DELETE_WINDOW "#donothing"

    focus $w

 return;   
}
# runRenderer