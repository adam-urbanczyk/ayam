# fixhtml.tcl:
# this little Tcl script fixes some problems with the sgml-tools (linuxdoc)
# packages HTML output, namely:


set procs { fixfigureb fixfiguree fixcaption }

# center all figures
proc fixfigureb { buf outfile } {
    set found 0
    set index [ string first "</FIGURE>" $buf ]
    if { $index > -1 } {
	puts $outfile "</FIGURE></CENTER>"
	set found 1
    }
    return $found;
}

proc fixfiguree { buf outfile } {
    set found 0
    set index [ string first "<FIGURE>" $buf ]
    if { $index > -1 } {
	puts $outfile "<CENTER><FIGURE>"
	set found 1
    }
    return $found;
}

proc fixcaption { buf outfile } {
    set found 0
    set index [ string first "<CAPTION>" $buf ]
    if { $index > -1 } {
	set out "<CAPTION><P>"
	append out [string range $buf 9 end]
	puts $outfile $out
	set found 1
    }
    return $found;
}

proc template { buf outfile } {
    set found 0

    return $found;
}

foreach infilename [glob html/*.html] {

    set infile [ open $infilename r ]
    set outfile [ open ${infilename}.tmp w ]

    while { ![eof $infile] } {
	gets $infile buf
	set found 0
	foreach p $procs {
	    set found [$p $buf $outfile]
	    if { $found } {
		break;
	    }
	}
	if { !$found } {
	    puts $outfile $buf
	}
    }
    close $infile
    close $outfile

    file copy -force ${infilename}.tmp ${infilename}
    file delete -force ${infilename}.tmp
}
# foreach

return;
