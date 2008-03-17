# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# concatnc.tcl - concatnc objects Tcl code

set ConcatNC_props { Transformations Attributes Tags ConcatNCAttr }


array set ConcatNCAttr {
arr   ConcatNCAttrData
sproc ""
gproc ""
w     fConcatNCAttr

}

array set ConcatNCAttrData {
Closed 0
FillGaps 0
Revert 0
Length 0
Order 0
Knot-Type 1
TanLength 3.0
}

# create ConcatNCAttr-UI
set w [frame $ay(pca).$ConcatNCAttr(w)]

addCheck $w ConcatNCAttrData Closed
addCheck $w ConcatNCAttrData FillGaps
addCheck $w ConcatNCAttrData Revert
addParam $w ConcatNCAttrData FTLength
addMenu $w ConcatNCAttrData Knot-Type [list NURB Custom]
addText $w  e0 "Resulting Curve:"
addInfo $w ConcatNCAttrData Length
addInfo $w ConcatNCAttrData Order
