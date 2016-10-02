# Ayam, save array: TCircleAttrData
# tcircle.tcl: example script for an Ayam Script object;
# this script wants Script object type "Create" and creates
# a triangle based NURBS circle with; it also has a property GUI, just
# add a tag "NP TCircleAttr" to the Script object to see it
if { ![info exists ::TCircleAttrData] } {
    array set ::TCircleAttrData {
	Radius 1.0
	SP {Radius}
    }
}
if { ![info exists ::TCircleAttrGUI] } {
    set w [addPropertyGUI TCircleAttr "" ""]
    addParam $w TCircleAttrData Radius
}

set pi 3.141592653589793238462643383
set a [expr { cos(30.0*$pi/180.0) } ]
set b [expr { $a*2.0 } ]

set cv ""
lappend cv $a 0.5 0.0 1.0
lappend cv 0.0 2.0 0.0 0.5
lappend cv -$a 0.5 0.0 1.0
lappend cv -$b -1.0 0.0 0.5
lappend cv 0.0 -1.0 0.0 1.0
lappend cv $b -1.0 0.0 0.5
lappend cv $a 0.5 0.0 1.0

set a [expr { 1.0/3.0 } ]
set b [expr { 2.0/3.0 } ]

set kv [list 0.0 0.0 0.0 $a $a $b $b 1.0 1.0 1.0]

crtOb NCurve -length 7 -order 3 -cn cv -kn kv
hSL
scalOb $TCircleAttrData(Radius) $TCircleAttrData(Radius) 1.0

# tcircle.tcl
