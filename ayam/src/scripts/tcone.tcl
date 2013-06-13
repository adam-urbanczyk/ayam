# Ayam, save array: TConeAttrData
# tcone.tcl: example script for an Ayam Script object;
# this script wants Script object type "Create" and creates
# a truncated cone; it also has a property GUI, just
# add a tag "NP TConeAttr" to the Script object to see it
if { ![info exists ::TConeAttrData] } {
    array set ::TConeAttrData {
	Closed 1
	ThetaMax 360.0
	ZMin 0
	ZMax 1
	RMin 1
	RMax 0.5
	SP {Closed ThetaMax ZMin ZMax RMin RMax}
    }
}
if { ![info exists ::TConeAttrGUI] } {
    set w [addPropertyGUI TConeAttr "" ""]
    addCheck $w TConeAttrData Closed
    addParam $w TConeAttrData ThetaMax
    addParam $w TConeAttrData RMin
    addParam $w TConeAttrData RMax
    addParam $w TConeAttrData ZMin
    addParam $w TConeAttrData ZMax
}
crtOb Hyperboloid; sL
getProp
set HyperbAttrData(Closed) $TConeAttrData(Closed)
set HyperbAttrData(ThetaMax) $TConeAttrData(ThetaMax)
set HyperbAttrData(P1_X) $TConeAttrData(RMin)
set HyperbAttrData(P1_Y) 0
set HyperbAttrData(P1_Z) $TConeAttrData(ZMin)
set HyperbAttrData(P2_X) $TConeAttrData(RMax)
set HyperbAttrData(P2_Y) 0
set HyperbAttrData(P2_Z) $TConeAttrData(ZMax)
setProp

# tcone.tcl
