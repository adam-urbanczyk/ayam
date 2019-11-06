# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# box.tcl - box objects Tcl code

proc init_Box { } {
    global ay Box_props BoxAttr BoxAttrData

    set Box_props { Transformations Attributes Material Tags BoxAttr }

    array set BoxAttr {
	arr   BoxAttrData
	sproc ""
	gproc ""
	w     fBoxAttr
    }

    array set BoxAttrData {
	Width 1.0
	Length 1.0
	Height 1.0
    }

    # create BoxAttr-UI
    set w [frame $ay(pca).$BoxAttr(w)]
    set a $BoxAttr(arr)
    addText $w e1 "Dimensions:"
    addParam $w $a Width
    addParam $w $a Length
    addParam $w $a Height

 return;
}
# init_Box
