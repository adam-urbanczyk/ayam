# Ayam, save array: LineAttrData
# crtlinegui.tcl: example script for an Ayam Script object;
# this script wants Script Object Type "Create" and creates
# a linear NURBS curve; it also has a property GUI, just
# add a tag "NP LineAttr" to the script object to see it
if { ![info exists ::LineAttrData] } {
    array set ::LineAttrData {
	Length 3
	SP {Length}
    }
}
if { ![info exists ::LineAttrGUI] } {
    set w [addPropertyGUI LineAttr "" ""]
    addParam $w LineAttrData Length
}

set length $::LineAttrData(Length)

crtOb NCurve -length 2; sL
setPnt 0 [expr {-$length/2.0}] 0.0 0.0 1.0
setPnt 1 [expr {$length/2.0}] 0.0 0.0 1.0

# crtlinegui.tcl
