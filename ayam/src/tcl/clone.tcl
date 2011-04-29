# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# clone.tcl - clone objects Tcl code

set Clone_props { Transformations Attributes Material Tags CloneAttr }


# clone_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc clone_getAttr { } {
    global ay CloneAttr CloneAttrData

    set oldfocus [focus]

    # remove old, create new CloneAttr-UI
    catch {destroy $ay(pca).$CloneAttr(w)}
    set w [frame $ay(pca).$CloneAttr(w)]
    getProp

    set ay(bok) $ay(appb)
    if { $CloneAttrData(Mirror) == 0 } {
	addParam $w CloneAttrData NumClones
	addCheck $w CloneAttrData Rotate
    }

    addMenu $w CloneAttrData Mirror [list "No" "At YZ-Plane" "At XZ-Plane"\
					 "At XY-Plane" ]

    if { $CloneAttrData(Mirror) == 0 } {

	addText $w e1 "Translation:"
	addParam $w CloneAttrData Translate_X
	addParam $w CloneAttrData Translate_Y
	addParam $w CloneAttrData Translate_Z

	addText $w e2 "Rotation:"
	addParam $w CloneAttrData Rotate_X
	addParam $w CloneAttrData Rotate_Y
	addParam $w CloneAttrData Rotate_Z

	addText $w e3 "Scale:"
	addParam $w CloneAttrData Scale_X
	addParam $w CloneAttrData Scale_Y
	addParam $w CloneAttrData Scale_Z

	addText $w e4 "Quaternion:"
	addParam $w CloneAttrData Quat0
	addParam $w CloneAttrData Quat1
	addParam $w CloneAttrData Quat2
	addParam $w CloneAttrData Quat3
    }

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# clone_getAttr

array set CloneAttr {
arr   CloneAttrData
sproc ""
gproc clone_getAttr
w     fCloneAttr

}

array set CloneAttrData {
    Mirror 0
    NumClones 1
}
