# fixs3html.tcl:
# transform all <H3> with "s3" label to <H4>
foreach infilename [glob html/*.html] {

    set infile [ open $infilename r ]
    set outfile [ open ${infilename}.tmp w ]

    while { ![eof $infile] } {
	gets $infile buf
	if { [string first "\"s3\"" $buf] > -1 } {
	    set buf [string map {H3 H4} $buf]
	}
	if { [string first "<H3>" $buf] > -1 } {
	    puts $outfile "<div style=\"height: 0.5em\">&nbsp;</div>"
	}
	puts $outfile $buf
    }
    close $infile
    close $outfile

    file copy -force ${infilename}.tmp ${infilename}
    file delete -force ${infilename}.tmp
}
# foreach

return;
