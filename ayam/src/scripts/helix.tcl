# Ayam, save array: HelixAttrData
# helix.tcl: example script for Ayam Script object
# this script wants Script Object Type "Create" and creates
# a helix NURBS curve; it also has a property GUI, just
# add a tag "NP HelixAttr" to the script object to see it
if { ![info exists ::HelixAttrData] } {
    array set ::HelixAttrData {
	Length 30
	Angle 45.0
	Radius 2.0
	DZ 0.25
	SP {Length Radius Angle DZ}
    }
}
if { ![info exists ::HelixAttrGUI] } {
    set w [addPropertyGUI HelixAttr "" ""]
    set a HelixAttrData
    addParam $w $a Length
    addParam $w $a Radius
    addParam $w $a Angle
    addParam $w $a DZ
}

set l $::HelixAttrData(Length)
set r $::HelixAttrData(Radius)
set ad [expr $::HelixAttrData(Angle)*acos(-1)/180.0]
set zd $::HelixAttrData(DZ)

crtOb NCurve -kt 1 -length $l;sL
set a 0.0
set z 0.0
for {set i 0} {$i < $l} {incr i} {
    set x [expr {$r*cos($a)}]
    set y [expr {$r*sin($a)}]
    setPnt $i $x $y $z 1.0
    set a [expr {$a + $ad}]
    set z [expr {$z + $zd}]
}
# for

# helix.tcl
