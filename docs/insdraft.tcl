# insdraft.tcl:
# this little Tcl script inserts the draft marker into the HTML document

foreach infilename [glob html/*.html] {

    set infile [ open $infilename r ]
    set outfile [ open ${infilename}.tmp w ]

    while { ![eof $infile] } {
	gets $infile buf

	set index [ string first "<HTML>" $buf ]
	if { $index > -1 } {
	    puts $outfile\
"<HTML><style type=\"text/css\">\
#watermark {\
  color: #f0f0f0;\
  font-size: 200pt;\
  -webkit-transform: rotate(-45deg);\
  -moz-transform: rotate(-45deg);\
  position: fixed;\
  width: 100%;\
  height: 100%;\
  margin: 0;\
  z-index: -1;\
  left:100px;\
  top:0px;\
}\
</style>"
	} else {
	    set index [ string first "<BODY>" $buf ]
	    if { $index > -1 } {
		puts $outfile\
"<div id=\"watermark\">\
<p>DRAFT</p>\
</div> "
	    } else {
		puts $outfile $buf
	    }
	}
    }
    # while
    close $infile
    close $outfile

    file copy -force ${infilename}.tmp ${infilename}
    file delete -force ${infilename}.tmp
}
# foreach

return;
