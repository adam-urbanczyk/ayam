# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# clone.tcl - clone objects Tcl code

proc init_Clone { } {
    global ay Clone_props CloneAttr CloneAttrData

    set Clone_props { Transformations Attributes Material Tags CloneAttr }

    array set CloneAttr {
	arr   CloneAttrData
	sproc ""
	gproc ""
	w     fCloneAttr
    }

    array set CloneAttrData {
	NumClones 1
	Quaternion "\[0, 0, 0, 1\]"
	QuaternionBall "n/a"
    }

    # create CloneAttr-UI
    set w [frame $ay(pca).$CloneAttr(w)]
    set a $CloneAttr(arr)
    addVSpace $w s1 2
    addParam $w $a NumClones
    addCheck $w $a Rotate
    addText $w e1 "Translation:"
    addParam $w $a Translate_X
    addParam $w $a Translate_Y
    addParam $w $a Translate_Z

    addText $w e2 "Rotation:"
    addParam $w $a Rotate_X
    addParam $w $a Rotate_Y
    addParam $w $a Rotate_Z
    addInfo $w $a Quaternion

    addText $w e3 "Scale:"
    addParam $w $a Scale_X
    addParam $w $a Scale_Y
    addParam $w $a Scale_Z

 return;
}
# init_Clone
