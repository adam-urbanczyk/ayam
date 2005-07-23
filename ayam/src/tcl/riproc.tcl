# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# riproc.tcl - riproc (RIB-Include) objects Tcl code

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
addMenu $w RiProcAttrData Type {DelayedReadArchive RunProgram DynamicLoad}
addFile $w RiProcAttrData File
addString $w RiProcAttrData Data
addText $w e0 "Bounding Box:"
addParam $w RiProcAttrData MinX
addParam $w RiProcAttrData MaxX

addParam $w RiProcAttrData MinY
addParam $w RiProcAttrData MaxY

addParam $w RiProcAttrData MinZ
addParam $w RiProcAttrData MaxZ

