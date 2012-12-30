# insover.tcl:
# this little Tcl script inserts the overview into the HTML document

set infile [ open html/ayam.html r ]
set outfile [ open html/ayam.html.tmp w ]
set found 0
while { ![eof $infile] } {
    gets $infile buf
    puts $outfile $buf
    if { $found == 0 } {
	if { [string first "</EM>" $buf] > -1 } {
	    set overfile [ open overview.html r ]
	    while { ![eof $overfile] } {
		gets $overfile buf
		puts $outfile $buf
	    }
	    close $overfile
	    set found 1
	    # eat <HR>
	    gets $infile buf
	}
    }
}
close $infile
close $outfile

file copy -force html/ayam.html.tmp html/ayam.html
file delete -force html/ayam.html.tmp

return;
