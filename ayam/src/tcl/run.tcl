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
    global $i ay
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
button $f.bok -text "Ok" -width 5 -command $okscript

button $f.bca -text "Cancel" -width 5 -command "destroy $w"

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
        close $channel
	catch {destroy .render${num}}
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
    global ayprefs
    if { [eof $channel] } {
        # program completed
        close $channel
	catch {destroy .render${num}}
    } else {
        set xx [gets $channel]

	set percent 0
	if { $ayprefs(RenderPT) != "" } {
	    scan $xx $template percent
	} else {
	    ayError 4 $cmd $xx
	}

	update
	catch {SetProgress .render${num}.f1.p $percent}
	update
    }
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

    incr ay(rnum)

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

    set found 0
    foreach win [winfo children .] {
	if { "[set top [winfo toplevel $win]]" == "." } continue
	if { [string first "render" $top] != -1  } { set found 1 }
    }


    toplevel $w
    wm title $w "Render_$ay(rnum)"
    wm iconname $w "Ayam"

    set f [frame $w.f1 -border 10]

    Progress $f.p
    pack $f.p -in $f -side top -fill x -expand yes
    pack $f -in $w -side top -fill x

    set f [frame $w.f2]
    button $f.bca -text "Cancel!" -width 16 -command "\
	    foreach i {$pids} { exec $kill \$i &}; destroy $w"

    pack $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x


    winCenter $w
    if { $found == 0 } {
	global ay
	set ay(runrenderwinraises) 0
	bind $w <Visibility> {
	    global ay
	    # are we obscured?
	    if { "%s" == "VisibilityPartiallyObscured" ||\
		 "%s" == "VisibilityFullyObscured" } {
		     # avoid deadlock fighting with a wm dock window
		     # that eventually wants to be "on top" too,
		     # the smarter one gives up...
		     if { $ay(runrenderwinraises) < 30 } {
			 incr ay(runrenderwinraises)
			 raise [winfo toplevel %W]
		     }
		 }
	}
    }

 return;   
}
# runRenderer