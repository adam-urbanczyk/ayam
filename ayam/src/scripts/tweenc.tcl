# tweenc.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify";
# it creates a tweened/interpolated curve from two arbitrary curves
getType type
if { [llength $type] == 2 } {
    if { [lindex $type 0] != "NCurve" } { withOb 0 {convOb -inplace} }
    if { [lindex $type 1] != "NCurve" } { withOb 1 {convOb -inplace} }
    if { ! [isCompNC] } { makeCompNC }
    tweenNC 0.5
    delOb
}
# tweenc.tcl
