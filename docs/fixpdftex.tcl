# fixpdftex.tcl:
# this little Tcl script fixes some problems with the Debian3.0 (Woody)
# sgml-tools (linuxdoc) packages LaTeX-output for PDF, namely:
# change .gif to .png, and append the height parameter from the
# leading epsfile command line to the .png-command line.
# Why Debian sgml-tools had to change in such an incompatible way (as opposed
# to the original sgml-tools package) is completely beyond my understanding.
#
# Also, format control beyond the capabilites of Linuxdoc-SGML is
# established here:
# o Index section is formatted in two columns, set in a smaller font and
#   with smaller line spacing
# o verbatim environments should not cross pages (needspace)
# o all sections start on a new page (newpage)
# o labels in sections (using hyperref phantomsection)
# o inline graphics (icons in tables)
# o improve itemize/enumerate environments line spread and paragraph distance

set procs { fixheight fixsection fixenddoc fixdocclass fixitemize fixenum fixlist insnewpage insneedspace insphantomsection insinlinegfx fixtoc fixhyperref fixpara }

proc fixheight { buf outfile } {
    global height
    set found 0
    set index [ string first "height=" $buf ]
    if { $index > -1 } {
	set buf2 [ string range $buf $index end ]
	scan $buf2 "height=%fcm" height
	puts $outfile $buf
	set found 1
    }
    set index [ string last ".gif" $buf ]
    if { $index > -1 } {
	set buf2 [ string range $buf 0 $index ]
	set buf3 "${buf2}png,height=${height}cm\}\}\\fi"
	puts $outfile $buf3
	set found 1
    }

    return $found;
}


proc fixsection { buf outfile } {
    global rewriteLists
    set found 0
    set index [ string first "\\section\{" $buf ]
    if { $index > -1 } {
	set index [ string first "section\{Index\}" $buf ]
	if { $index > -1 } {
	    puts $outfile "\\twocolumn \\section\{Index\} \{ \\footnotesize "
	    set rewriteLists 1
	} else {
	    puts $outfile "\\newpage \\section\{"
	    set index [ string first "\{" $buf ]
	    incr index
	    set out2 [ string range $buf $index end ]
	    puts $outfile $out2
	    puts $outfile "\\setstretch\{1.1\}"

	}
	set found 1
    }

    return $found;
}

# add extra curly brace needed by footnotesize index section
proc fixenddoc { buf outfile } {
    set found 0
    set index [ string first "end\{document" $buf ]
    if { $index > -1 } {
	puts $outfile "\}\\end\{document\}"
	set found 1
    }
    return $found;
}


proc fixdocclass { buf outfile } {
    set found 0
    set index [ string first "documentclass" $buf ]
    if { $index > -1 } {
	puts $outfile "\\documentclass\[a4paper,11pt\]\{article\}"
	puts $outfile "\\usepackage\{needspace\}"
	puts $outfile "\\usepackage\{array\}"
	puts $outfile "\\setlength\{\\extrarowheight\}\{2pt\}"
	puts $outfile "\\usepackage\{setspace\}"
	puts $outfile "\\usepackage\[tight\]\{shorttoc\}"
	puts $outfile "\\usepackage\[perpage,para\]\{footmisc\}"
	puts $outfile "\\usepackage{times}"
	puts $outfile "\\widowpenalty=1000"
	puts $outfile "\\clubpenalty=1000"
	set found 1

	# one shot...
	set posn [lsearch -exact $::procs fixdocclass]
	set ::procs [lreplace $::procs $posn $posn]
    }

    return $found;
}


proc fixlist { buf outfile } {
    global rewriteLists
    set found 0
    set index [ string first "\\begin\{list" $buf ]
    if { ($index > -1) && $rewriteLists } {
	puts $outfile\
	    "\\begin\{list\}\{\}\{\\itemsep -0.5ex \\itemindent -2ex \\parsep 1ex\}"
	set found 1
    }
    return $found;
}

proc fixitemize { buf outfile } {
    global rewriteLists
    set found 0
    set index [ string first "\\begin\{itemize" $buf ]
    if { ($index > -1) } {
	if {  $rewriteLists } {
	puts $outfile\
     "\\begin\{itemize\}\\setstretch\{0.95\}\\setlength\{\\itemsep\}\{0.5ex\}"
	    puts $outfile "\\vspace\{-1.5ex\}"
	} else {
	puts $outfile\
	    "\\vspace\{-1.5ex\}\\begin\{itemize\}\\setstretch\{1\}\\setlength\{\\itemsep\}\{0.33ex\}\\setlength\{\\parsep\}\{1ex\}"
	}
	set found 1
    }
    return $found;
}

proc fixenum { buf outfile } {
    set found 0
    set index [ string first "\\begin\{enumerate" $buf ]
    if { ($index > -1) } {
	puts $outfile\
     "\\begin\{enumerate\}\\setstretch\{1\}\\setlength\{\\itemsep\}\{0.33ex\}\\setlength\{\\parsep\}\{1ex\}"
	set found 1
    }
    return $found;
}


proc fixpara { buf outfile } {
    set found 0
    set index [ string first "~\\" $buf ]
    if { ($index > -1) } {
	set out "~\\\\\\vskip -2.2ex"
	append out [string range $buf 3 end]
	puts $outfile $out    
	set found 1
    }
    return $found;
}

proc insnewpage { buf outfile } {
    set found 0
    if { [string first newpage $buf] > -1 } {
	puts $outfile "\\newpage"
	set found 1
    }
    return $found;
}


proc insneedspace { buf outfile } {
    set found 0
    if { [string first "needspace" $buf] > -1 } {
	scan $buf "needspace %d" num
	puts $outfile "\\needspace\{$num \\baselineskip\}"
	set found 1
    }
    return $found;
}


proc insphantomsection { buf outfile } {
    set found 0
    if { [string first "phantomsection" $buf] > -1 } {
	puts $outfile "\\phantomsection"
	set found 1
    }
    return $found;
}


proc insinlinegfx { buf outfile } {
    global height
    set found 0
    set index [string first "inlinegfx" $buf]
    if { $index > -1 } {
	set buf [regsub -all "inlinegfx (.*?) (.*?) " $buf\
      {\rule[-0.3cm]{0cm}{0.9cm}\raisebox{-2mm}{\epsfig{file=\1,height=\2}}}]
	puts $outfile $buf
	set found 1
    }
    return $found;
}


proc fixtoc { buf outfile } {

    set found 0
    set index [ string first "tableofcontents" $buf ]
    if { $index > -1 } {
	puts $outfile "\\shorttoc{Overview}{1}\n\\newpage"
	puts $outfile $buf

	set found 1

	# one shot...
	set posn [lsearch -exact $::procs fixtoc]
	set ::procs [lreplace $::procs $posn $posn]
    }

    return $found;
}


proc fixhyperref { buf outfile } {
    set found 0
    set index [ string first "hyperref" $buf ]
    if { $index > -1 } {
	puts $outfile "\\usepackage\[colorlinks=true,urlcolor=blue,breaklinks=true,hyperfootnotes=false\]\{hyperref\}"
	puts $outfile "\\hypersetup\{linkcolor=\[rgb\]\{0,0,0.3\}\}"

	set found 1

	# one shot...
	set posn [lsearch -exact $::procs fixhyperref]
	set ::procs [lreplace $::procs $posn $posn]
    }

    return $found;
}



proc template { buf outfile } {
    set found 0

    return $found;
}

set infile [ open [lindex $argv 0] r ]
set outfile [ open [lindex $argv 1] w ]
set rewriteLists 0
set height 0
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

return;
