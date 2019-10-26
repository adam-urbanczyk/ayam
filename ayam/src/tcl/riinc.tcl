# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# riinc.tcl - riinc (RIB-Include) objects Tcl code

proc init_RiInc { } {
    global ay RiInc_props RiIncAttr RiIncAttrData

    set RiInc_props { Transformations Attributes Material Tags RiIncAttr }

    array set RiIncAttr {
	arr   RiIncAttrData
	sproc ""
	gproc ""
	w     fRiIncAttr
    }

    array set RiIncAttrData {
	File ""
	Width 1.0
	Length 1.0
	Height 1.0
    }

    # create RiIncAttr-UI
    set w [frame $ay(pca).$RiIncAttr(w)]
    set a $RiIncAttr(arr)
    addText $w e0 "Include:"
    addFile $w $a File
    addText $w e1 "Dimensions:"
    addParam $w $a Width
    addParam $w $a Length
    addParam $w $a Height

 return;
}
# init_RiInc
