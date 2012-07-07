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

set w [frame $ay(pca).$CloneAttr(w)]
addVSpace $w s1 2
addParam $w CloneAttrData NumClones
addCheck $w CloneAttrData Rotate
addText $w e1 "Translation:"
addParam $w CloneAttrData Translate_X
addParam $w CloneAttrData Translate_Y
addParam $w CloneAttrData Translate_Z

addText $w e2 "Rotation:"
addParam $w CloneAttrData Rotate_X
addParam $w CloneAttrData Rotate_Y
addParam $w CloneAttrData Rotate_Z
addInfo $w CloneAttrData Quaternion

addText $w e3 "Scale:"
addParam $w CloneAttrData Scale_X
addParam $w CloneAttrData Scale_Y
addParam $w CloneAttrData Scale_Z
