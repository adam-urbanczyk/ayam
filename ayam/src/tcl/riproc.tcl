# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# riproc.tcl - riproc (RiProcedural) objects Tcl code

proc init_RiProc { } {
    global ay RiProc_props RiProcAttr RiProcAttrData

    set RiProc_props { Transformations Attributes Material Tags RiProcAttr }

    array set RiProcAttr {
	arr   RiProcAttrData
	sproc ""
	gproc ""
	w     fRiProcAttr
    }

    array set RiProcAttrData {
	Type 0
	File ""
	Data ""
    }

    # create RiProcAttr-UI
    set w [frame $ay(pca).$RiProcAttr(w)]
    set a $RiProcAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Type {DelayedReadArchive RunProgram DynamicLoad}
    addFile $w $a File
    addString $w $a Data
    addText $w e0 "Bounding Box:"
    addParam $w $a MinX
    addParam $w $a MaxX

    addParam $w $a MinY
    addParam $w $a MaxY

    addParam $w $a MinZ
    addParam $w $a MaxZ

 return;
}
# init_RiProc
