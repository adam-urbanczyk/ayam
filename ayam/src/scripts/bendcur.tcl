# bendcur.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify"
# and deforms a curve based on the shape of a
# second curve
withOb 0 do {getProp}
set length $NCurveAttrData(Length)
withOb 1 do {getProp}
set umin [lindex $NCurveAttrData(Knots)\
	      [expr {$NCurveAttrData(Order) - 1}]]
set umax [lindex $NCurveAttrData(Knots)\
    $NCurveAttrData(Length)]
set ud [expr {($umax-$umin)/($length-1)}]
set u 0.0
for {set i 0} {$i < $length} {incr i} {
    withOb 1 do {getPnt -u $u sx sy sz sw}
    withOb 0 do {getPnt $i x y z w}
    set y [expr {$y + $sy}]
    withOb 0 do {setPnt $i $x $y $z $w}
    set u [expr {$u + $ud}]
}
# for

# bendcur.tcl