# conscir.tcl: example script for Ayam Script object
# wants script type "Run"
# shows a simple one object constraint that locks
# the control points of a NCurve object in radial
# configuration in the xy-plane
set t "";catch [getType t];if {$t != "NCurve"} {return;}
getProp;
set length $NCurveAttrData(Length)
set angled [expr (2*3.141592654)/$length]
set angle 0.0
for {set i 0} {$i < $length} {incr i} {
    getPnt $i x y z w
    if {[expr abs($x)] < 1e-6 && [expr abs($y)] < 1e-6} { continue }
    set r [expr sqrt($x * $x + $y * $y)]
    set x [expr $r * cos($angle)]
    set y [expr $r * sin($angle)]
    setPnt $i $x $y $z 1.0
    set angle [expr $angle + $angled]
}
# for

# conscir.tcl