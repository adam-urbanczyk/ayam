# addmyst.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify"
# and adds a PV tag for optimized (chordal length)
# texture coordinates <mys, myt> for each control
# point to a NURBS patch

proc v3len { x1 y1 z1 x2 y2 z2 } {

    if { $x1 > $x2 }\
	{ set v1 [expr {$x1 - $x2}] } else { set v1 [expr {$x2 - $x1}] }
    if { $y1 > $y2 }\
	{ set v2 [expr ${y1 - $y2}] } else { set v2 [expr {$y2 - $y1}] }
    if { $z1 > $z2 }\
	{ set v3 [expr {$z1 - $z2}] } else { set v3 [expr {$z2 - $z1}] }

    if { [expr {abs($v1) < 1.0e-06}] &&
	 [expr {abs($v2) < 1.0e-06}] &&
	 [expr {abs($v3) < 1.0e-06}] } {
	return 0.0;
    } else {
	return [expr {sqrt($v1 * $v1 + $v2 * $v2 + $v3 * $v3)}];
    }
}
# v3len

proc addMyST { } {
global x y z w ox oy oz NPatchAttrData

withOb 0 do { getProp }
set width $NPatchAttrData(Width)
set height $NPatchAttrData(Height)
set l 0.0

set tcs mys,vertex,f,[expr $width*$height]
for {set j 0} {$j < $height} {incr j} {
    # calculate total length of row
    set rl 0.0; withOb 0 do {getPnt 0 $j ox oy oz w}
    for {set i 1} {$i < $width} {incr i} {
	withOb 0 do {getPnt $i $j x y z w}
	set d [v3len $x $y $z $ox $oy $oz]
	set rl [expr {$rl + $d}]
	set ox $x; set oy $y; set oz $z
    }

    # calculate texcoords for row
    set il 0.0; withOb 0 do {getPnt 0 $j ox oy oz w}
    append tcs ,0.0
    for {set i 1} {$i < $width} {incr i} {
	withOb 0 do {getPnt $i $j x y z w}
	set d [v3len $x $y $z $ox $oy $oz]
	set il [expr {$il + $d}]
	append tcs ,[expr {$il/$rl}]
	set ox $x; set oy $y; set oz $z
    }
}

set tct myt,vertex,f,[expr {$width*$height}]
set t ""
for {set i 0} {$i < $width} {incr i} {
    # calculate total length of column
    set rl 0.0; withOb 0 do {getPnt $i 0 ox oy oz w}
    for {set j 1} {$j < $height} {incr j} {
	withOb 0 do {getPnt $i $j x y z w}
	set d [v3len $x $y $z $ox $oy $oz]
	set rl [expr {$rl + $d}]
	set ox $x; set oy $y; set oz $z
    }
    # calculate texcoords for column
    set il 0.0; withOb 0 do {getPnt $i 0 ox oy oz w}
    lappend t 0.0
    for {set j 1} {$j < $height} {incr j} {
	withOb 0 do {getPnt $i $j x y z w}
	set d [v3len $x $y $z $ox $oy $oz]
	set il [expr {$il + $d}]
	lappend t [expr {$il/$rl}]
	set ox $x; set oy $y; set oz $z
    }
}

for {set j 0} {$j < $height} {incr j} {
    for {set i 0} {$i < $width} {incr i} {
	set ind [expr {($i * $height) + $j}]
	set elem [lindex $t $ind]
	append tct ,$elem
    }
}

# finally, add/replace the tags
withOb 0 do {getTags otn otv}
set founds 0; set foundt 0
for {set i 0} {$i < [llength $otn]} {incr i} {
    if { [lindex $otn $i] == "PV" } {
	if { [string compare -length 4 "mys," [lindex $otv $i]] == 0 } {
	    set founds 1
	    withOb 0 do {setTag -index $i "PV" $tcs}
	}
	if { [string compare -length 4 "myt," [lindex $otv $i]] == 0 } {
	    set foundt 1
	    withOb 0 do {setTag -index $i "PV" $tct}
	}
    }
}
if { $founds == 0 } { withOb 0 do { addTag "PV" $tcs } }
if { $foundt == 0 } { withOb 0 do { addTag "PV" $tct } }

}
# addMyST

addMyST

delTags NoExport

# addmyst.tcl