# Ayam, save array: ExtrudeNAttrData
# extruden.tcl: example script for an Ayam Script object;
# this script wants Script object type "Modify" and extrudes a planar
# curve along its normal; it also has a property GUI, just
# add a tag "NP ExtrudeNAttr" to the Script object to see it
if { ![info exists ::ExtrudeNAttrData] } {
    array set ::ExtrudeNAttrData {
	Height 1.0
	SP {Height}
    }
}
if { ![info exists ::ExtrudeNAttrGUI] } {
    set w [addPropertyGUI ExtrudeNAttr "" ""]
    addVSpace $w v1 5
    addParam $w ExtrudeNAttrData Height
}

convOb -i NCurve
applyTrafo -all
set n [getNormal]
set h $ExtrudeNAttrData(Height)
if { $h != 1 } {
    set n [list [expr [lindex $n 0] * $h] [expr [lindex $n 1] * $h]\
	       [expr [lindex $n 2] * $h] ]
}
copOb
pasOb
selOb 1
movOb [lindex $n 0] [lindex $n 1] [lindex $n 2]
selOb 0 1
cutOb
crtOb Skin
goDown -1
pasOb
goUp
selOb 0
notifyOb
