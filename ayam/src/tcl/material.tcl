# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# material.tcl - material objects Tcl code

set Material_props { RiAttributes Tags Surface Displacement Interior Exterior }

proc setRiAttrp { } {
    setProp
    uCL cl "0 1"
 return;
}

array set RiAttributes {
arr   RiAttrData
sproc setRiAttrp
gproc ""
w     fRiAttr

}

array set RiAttrData {
   Interpolation 0
   BoundCoord 0
   CastShadows 0
}

# create RiAttributes-UI
set w [frame $ay(pca).$RiAttributes(w)]

addString $w RiAttrData Materialname
addInfo $w RiAttrData RefCount
addInfo $w RiAttrData Registered
addColor $w RiAttrData Color
addColor $w RiAttrData Opacity

addText $w e3 "Shading:"
addParam $w RiAttrData ShadingRate
addMenu $w RiAttrData Interpolation [list Constant Smooth]

addText $w e4 "Displacement:"

addMenu $w RiAttrData BoundCoord [list Object Shader Camera]
addParam $w RiAttrData BoundVal

addText $w e5 "BMRT-Specific:"
addCheck $w RiAttrData TrueDisp
addMenu $w RiAttrData CastShadows [list Os None Opaque Shade]

addText $w e6 "Visibility:"
addCheck $w RiAttrData Camera
addCheck $w RiAttrData Reflection
addCheck $w RiAttrData Shadow


proc material_createp { } {
    global ay

    set w .createMw
    catch {destroy $w}
    toplevel $w
    wm title $w "Create Material"
    wm iconname $w "Ayam"
    wm transient $w .

    
    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    label $f.l -text "Name:"
    entry $f.e -width 20
    pack $f.l $f.e -side left -fill x

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command { 
	global ay ayprefs ay_error
	set ay_error 0
	crtOb Material [.createMw.f1.e get]

	if { $ay_error == 0 } {
	    focus .
	    grab release .createMw
	    destroy .createMw
	    uCR; sL; rV;
	}
    }

    button $f.bca -text "Cancel" -width 5 -command "destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f1.e
    tkwait window $w

 return;
}
