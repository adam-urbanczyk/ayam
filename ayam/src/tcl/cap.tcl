# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cap.tcl - cap objects Tcl code

set Cap 1

proc init_Cap { } {
global ay Cap_props CapAttr CapAttrData

set Cap_props { Transformations Attributes Material Tags CapAttr }

array set CapAttr {
arr   CapAttrData
sproc ""
gproc ""
w     fCapAttr

}

array set CapAttrData {
Type 1
DisplayMode 0
NPInfoBall "n/a"
}
# create CapAttr-UI
set w [frame $ay(pca).$CapAttr(w)]

addMenu $w CapAttrData Type {Trim Simple Gordon}
addParam $w CapAttrData Tolerance
addMenu $w CapAttrData DisplayMode $ay(npdisplaymodes)

addText $w CapAttrData "Created NURBS Patch:"
addInfo $w CapAttrData NPInfo

return;
}
# init_Cap

# as the cap type data is to be saved in the object type specific
# property arrays, using setProp is sufficient
array set Caps {
arr   CapsData
sproc ""
gproc cap_getCaps
w     fCaps
}

# cap_getCaps:
# get caps data from object and create a new property GUI,
# the Caps property GUI is dynamic, as boundary names may
# differ between object types
proc cap_getCaps { } {
    global ay Caps CapsData

    set oldfocus [focus]

    # remove old, create new Caps-UI
    catch {destroy $ay(pca).$Caps(w)}
    set w [frame $ay(pca).$Caps(w)]

    getProp

    set type ""
    getType type
    if { $type != "" } {
	global ${type}AttrData

	if { [info exists ${type}AttrData(BoundaryNames)] } {

	    eval set capnames \$${type}AttrData(BoundaryNames)
	    foreach capname $capnames {
		addMenu $w ${type}AttrData ${capname}Cap \
	     [list "Off" "Trim" "Simple" "SimpleInt" "Gordon"]
	    }
	} else {
	    addText $w e1 "Not supported here!"
	}
	# if
    }
    # if

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# cap_getCaps
