# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# bpatch.tcl - bpatch objects Tcl code

proc init_BPatch { } {
    global ay BPatch_props BPatchAttr BPatchAttrData

    set BPatch_props { Transformations Attributes Material Tags BPatchAttr }

    array set BPatchAttr {
	arr   BPatchAttrData
	sproc ""
	gproc ""
	w     fBPatchAttr
    }

    # create BPatchAttr-UI
    set w [frame $ay(pca).$BPatchAttr(w)]
    set a $BPatchAttr(arr)
    addText $w e1 "Point 1:"
    addParam $w $a P1_X
    addParam $w $a P1_Y
    addParam $w $a P1_Z
    addText $w e2 "Point 2:"
    addParam $w $a P2_X
    addParam $w $a P2_Y
    addParam $w $a P2_Z
    addText $w e3 "Point 3:"
    addParam $w $a P3_X
    addParam $w $a P3_Y
    addParam $w $a P3_Z
    addText $w e4 "Point 4:"
    addParam $w $a P4_X
    addParam $w $a P4_Y
    addParam $w $a P4_Z

 return;
}
# init_BPatch
