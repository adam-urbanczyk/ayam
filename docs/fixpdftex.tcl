# fixpdftex.tcl:
# this little Tcl script fixes some problems with the Debian3.0 (Woody)
# sgml-tools (linuxdoc) packages LaTeX-output for PDF, namely:
# change .gif to .png, and append the height parameter from the
# leading epsfile command line to the .png-command line.
# Why Debian-sgml-tools had to change in such an incompatible way (as opposed
# to the original sgml-tools package) is completely beyond my understanding.
#
# Additionally, this script arranges for the section Index to be
# formatted in two columns and set in a smaller font.
set infile [ open [lindex $argv 0] r ]
set outfile [ open [lindex $argv 1] w ]
set height 0
while { ![eof $infile] } {
    gets $infile buf
    set index [ string first "height=" $buf ]
    if { $index > -1 } {
	set buf2 [ string range $buf $index end ]
	scan $buf2 "height=%fcm" height
    }
    set index [ string last ".gif" $buf ]
    if { $index > -1 } {
	set buf2 [ string range $buf 0 $index ]
	set buf3 "${buf2}png,height=${height}cm\}\}\\fi"
	puts $outfile $buf3
    } else {
	set index [ string first "section\{Index\}" $buf ]
	if { $index > -1 } {
	    puts $outfile "\\section\{Index\} \{ \\twocolumn \\footnotesize"
	} else {
	    set index [ string first "end\{document" $buf ]
	    if { $index > -1 } {
		puts $outfile "\}\\end\{document\}"
	    } else {
		puts $outfile $buf
	    }
	}
    }
}

