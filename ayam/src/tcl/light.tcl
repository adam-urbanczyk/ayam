# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# light.tcl - light objects Tcl code

set Light_props { Transformations Attributes Tags LightShader LightAttr }


# light_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc light_getAttr { } {
    global ay ayprefs LightAttr LightAttrData

    set oldfocus [focus]

    catch {destroy $ay(pca).$LightAttr(w)}
    set w [frame $ay(pca).$LightAttr(w)]
    getProp

    set ay(bok) $ay(appb)

    # create new UI
    set a $LightAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Type {Custom Point Distant Spot}
    addCheck $w $a IsOn
    addCheck $w $a IsLocal
    addCheck $w $a Shadows
    if { $LightAttrData(Type) == 0 } {
	addParam $w $a Samples
    }
    if { $LightAttrData(Type) != 0 } {
	addParam $w $a Intensity
	addColor $w $a Color
    }

    addCheck $w $a UseSM
    addParam $w $a SMRes

    if { $LightAttrData(Type) == 3 } {
	addParam $w $a ConeAngle
	addParam $w $a ConeDAngle
	addParam $w $a BeamDistrib
    }

    if { $LightAttrData(Type) != 0 } {
	addParam $w $a From_X
	addParam $w $a From_Y
	addParam $w $a From_Z
    }
    if { $LightAttrData(Type) >= 2 } {
	addParam $w $a To_X
	addParam $w $a To_Y
	addParam $w $a To_Z
    }

    plb_setwin $w $oldfocus

 return;
}
# light_getAttr


array set LightAttr {
    arr   LightAttrData
    sproc ""
    gproc light_getAttr
    w     fLightAttr
}

array set LightAttrData {
    Type 0
}

proc light_getShader { } {
    global ay ay_shader LightShader

    catch {destroy $ay(pca).$LightShader(w)}
    set w [frame $ay(pca).$LightShader(w)]
    shaderGet light ay_shader
    shader_buildGUI $w light

 return;
}


proc light_setShader { } {

    shaderSet light ay_shader

 return;
}
# light_setShader

array set LightShader {
    arr   ay_shader
    sproc light_setShader
    gproc light_getShader
    w     fLightShader
}

