# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# light.tcl - light objects Tcl code

set Light_props { Transformations Attributes Tags LightShader LightAttr }


proc light_getAttr { } {
global ay LightAttr LightAttrData

catch {destroy $ay(pca).$LightAttr(w)}
set w [frame $ay(pca).$LightAttr(w)]
getProp

# create new UI
addMenu $w LightAttrData Type {Custom Point Distant Spot}
addCheck $w LightAttrData IsOn
addCheck $w LightAttrData Shadows
if { $LightAttrData(Type) == 0 } {
    addParam $w LightAttrData Samples
}
if { $LightAttrData(Type) != 0 } {

    addParam $w LightAttrData Intensity
    addColor $w LightAttrData Color
}

addCheck $w LightAttrData UseSM
addParam $w LightAttrData SMRes

if { $LightAttrData(Type) == 3 } {
    addParam $w LightAttrData ConeAngle
    addParam $w LightAttrData ConeDAngle
    addParam $w LightAttrData BeamDistrib
}

if { $LightAttrData(Type) != 0 } {
    addParam $w LightAttrData From_X
    addParam $w LightAttrData From_Y
    addParam $w LightAttrData From_Z
}
if { $LightAttrData(Type) >= 2 } {
    addParam $w LightAttrData To_X
    addParam $w LightAttrData To_Y
    addParam $w LightAttrData To_Z
}
$ay(pca) itemconfigure 1 -window $w

plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

}


array set LightAttr {
arr   LightAttrData
sproc ""
gproc light_getAttr
w     fLightAttr

}

array set LightAttrData {
Type 0

}

# create LightAttr-UI
set w [frame $ay(pca).$LightAttr(w)]
addMenu $w LightAttrData Type {Custom Point Spot Distant}
addCheck $w LightAttrData IsOn
addCheck $w LightAttrData Shadows
addParam $w LightAttrData Samples


proc light_getShader { } {
global ay ay_shader LightShader

catch {destroy $ay(pca).$LightShader(w)}
set w [frame $ay(pca).$LightShader(w)]
shaderGet light ay_shader
shader_buildGUI $w light
$ay(pca) itemconfigure 1 -window $w
plb_resize
}


proc light_setShader { } {

shaderSet light ay_shader

}

array set LightShader {
arr   ay_shader
sproc light_setShader
gproc light_getShader
w     fLightShader

}

# create UI-Stub
set w [frame $ay(pca).$LightShader(w)]
