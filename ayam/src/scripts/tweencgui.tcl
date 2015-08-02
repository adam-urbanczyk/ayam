# Ayam, save array: TweenCAttrData
# tweencgui.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify";
# it creates a tweened/interpolated curve from two arbitrary curves;
# it also has a property GUI, just add a tag "NP TweenCAttrData"
# to the script object to see it
if { ![info exists ::TweenCAttrData] } {
    array set ::TweenCAttrData {
	Parameter 0.5
	SP { Parameter }
    }
}
if { ![info exists ::TweenCAttrGUI] } {
    set w [addPropertyGUI TweenCAttr "" ""]
    addParam $w TweenCAttrData Parameter
}
getType type
if { [llength $type] == 2 } {
    if { [lindex $type 0] != "NCurve" } { withOb 0 {convOb -inplace} }
    if { [lindex $type 1] != "NCurve" } { withOb 1 {convOb -inplace} }
    if { ! [isCompNC] } { makeCompNC }
    tweenNC $TweenCAttrData(Parameter)
    delOb
}
# tweenncgui.tcl
