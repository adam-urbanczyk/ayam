# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# browser.tcl - invoke a HTML browser (code taken from "The Tcl'ers Wiki")

proc browser_findExecutable {progname varname} {
    upvar 1 $varname result
    set progs [auto_execok $progname]
    if {[llength $progs]} {
	set result [lindex $progs 0]
    }
 return [llength $progs]
}
# browser_findExecutable


proc browser_urlOpen {url} {
    global env tcl_platform

    ayError 4 "Help" "Launching browser, please wait..."

    switch $tcl_platform(platform) {
        "unix" {
	    if { $tcl_platform(os) == "Darwin" } {
		exec open ${url}
		return;
	    }
	    if { ! [info exists env(NETSCAPE)] } {
		set mozilla netscape
	    } else {
		set mozilla $env(NETSCAPE)
	    }

	    if { (! [info exists env(BROWSER)]) || \
		 ([auto_execok $env(BROWSER)] == "") } {
		set browser ""
		expr {
		    [browser_findExecutable $mozilla browser] ||
		    [browser_findExecutable galeon browser] ||
		    [browser_findExecutable konqueror browser] ||
		    [browser_findExecutable links browser] ||
		    [browser_findExecutable lynx browser] ||
		    [browser_findExecutable iexplorer browser]
		}
	    } else {
		set browser $env(BROWSER)
	    }
            # lynx can also output formatted text to a variable
            # with the -dump option, as a last resort:
            # set formatted_text [ exec lynx -dump $url ] - PSE
            if {[catch {exec $browser -remote "openURL(${url},new-window)"}]} {
                # perhaps browser doesn't understand -remote flag
                if {[catch {exec $browser $url &} emsg]} {
                    error "Error displaying $url in browser\n$emsg"
                    # Another possibility is to just pop a window up
                    # with the URL to visit in it. - DKF
                }
            }
        }
        "windows" {
	    set rc [catch {
		exec rundll32 url.dll,FileProtocolHandler $url & } emsg]
            if {$rc} {
                error "Error displaying $url in browser\n$emsg"
            }
	    return;
	    # XXXX this does, for some obscure reason, not work
	    # or just for local files?
            if {$tcl_platform(os) == "Windows NT"} {
                set rc [catch {exec $env(COMSPEC) /c start $url &} emsg]
            } else {
		# Windows 95/98
		set rc [catch {exec start $url} emsg]
            }
            if {$rc} {
                error "Error displaying $url in browser\n$emsg"
            }
        }
        "macintosh" {
            if {0 == [info exists env(BROWSER)]} {
                set env(BROWSER) "Browse the Internet"
            }
            if {[catch {
                AppleScript execute\
			"tell application \"$env(BROWSER)\"
		open url \"$url\"
		end tell
                "} emsg]
            } then {
                error "Error displaying $url in browser\n$emsg"
            }
	}
    }

 return;
}
# browser_urlOpen

proc help { {command ""} } {
    global ayprefs

    if { $command == "" } {
	puts "Usage: help <scripting interface command>"
	return;
    }

    set lcommand [string tolower $command]
    browser_urlOpen $ayprefs(Docs)ayam-6.html\#sc${lcommand}

 return;
}
# help