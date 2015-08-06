# fixs3sgml.tcl:
# this little Tcl script adds a label to each <sect3> so that
# fixs3html.tcl can then transform the <H3> to <H4>

set infile [ open ayam.sgml r ]
set outfile [ open ayam.sgml.tmp w ]
while { ![eof $infile] } {
    gets $infile buf
    if { [string first "<sect3>" $buf] > -1 } {
	append buf "<label id=\"s3\">"
    }
    puts $outfile $buf
}
close $infile
close $outfile

file copy -force ayam.sgml.tmp ayam.sgml
file delete -force ayam.sgml.tmp

return;
