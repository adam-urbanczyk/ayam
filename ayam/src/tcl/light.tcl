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
    addVSpace $w s1 2
    addMenu $w LightAttrData Type {Custom Point Distant Spot}
    addCheck $w LightAttrData IsOn
    addCheck $w LightAttrData IsLocal
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

