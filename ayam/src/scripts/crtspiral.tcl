# crtspiral.tcl: example script for Ayam Script object
# this script wants Script Object Type "Create"
# and creates a spiral NURBS curve
set length 30
set angled [expr {90.0*3.141592654/180.0}]
set r 3.0
set zd 0.25
crtOb NCurve -length $length;hSL
set angle 0.0
set z 0.0
for {set i 0} {$i < $length} {incr i} {
    set x [expr {$r*cos($angle)}]
    set y [expr {$r*sin($angle)}]

    setPnt $i $x $y $z 1.0

    set angle [expr {$angle + $angled}]
    set z [expr {$z + $zd}]
}
# for

# crtspiral.tcl