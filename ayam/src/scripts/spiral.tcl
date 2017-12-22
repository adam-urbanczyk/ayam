# Ayam, save array: SpiralAttrData
# spiral.tcl: example script for Ayam Script object
# this script wants Script Object Type "Create" and creates
# a spiral NURBS curve; it also has a property GUI, just
# add a tag "NP SpiralAttr" to the script object to see it
if { ![info exists ::SpiralAttrData] } {
    array set ::SpiralAttrData {
	Length 30
	Angle 45
	RMin 0.1
	RDiff 0.1
	SP {Length Angle RMin RDiff}
    }
}
if { ![info exists ::SpiralAttrGUI] } {
    set w [addPropertyGUI SpiralAttr "" ""]
    addParam $w SpiralAttrData Length
    addParam $w SpiralAttrData Angle
    addParam $w SpiralAttrData RMin
    addParam $w SpiralAttrData RDiff
}

set l $::SpiralAttrData(Length)
set ad [expr $::SpiralAttrData(Angle)*acos(-1)/180.0]
set r $::SpiralAttrData(RMin)
set rd $::SpiralAttrData(RDiff)

crtOb NCurve -kt 1 -length $l;sL
set angle 0.0
for {set i 0} {$i < $l} {incr i} {
    set x [expr {$r*cos($angle)}]
    set y [expr {$r*sin($angle)}]

    setPnt $i $x $y 0.0 1.0

    set angle [expr {$angle + $ad}]
    set r [expr {$r + $rd}]
}
# for

# spiral.tcl
