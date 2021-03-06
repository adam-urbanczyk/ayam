# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# root.tcl - root objects Tcl code

set Root_props { RiOptions Tags Atmosphere Imager }

array set RiOptions {
arr   RiOptData
sproc ""
gproc ""
w     fRiOpt

}

array set RiOptData {
FilterFunc Gaussian
PRManSpec RiStandard
}

# create RiOpt-UI
set w [frame $ay(pca).$RiOptions(w)]

addText $w ei "Image:"
addParam $w RiOptData Width
addParam $w RiOptData Height
addCheck $w RiOptData StdDisplay

addText $w ea "Antialias:"
addParam $w RiOptData Variance
addParam $w RiOptData Samples_X
addParam $w RiOptData Samples_Y
addMenu $w RiOptData FilterFunc [list Gaussian Triangle CatmullRom Box Sinc]
addParam $w RiOptData FilterWidth
addParam $w RiOptData FilterHeight

addText $w ee "Exposure:"
addParam $w RiOptData ExpGain
addParam $w RiOptData ExpGamma

addText $w eq "Quantization:"
addParam $w RiOptData RGBA_ONE
addParam $w RiOptData RGBA_MIN
addParam $w RiOptData RGBA_MAX
addParam $w RiOptData RGBA_Dither

addText $w eb "BMRT-Specific:"
addText $w er "Rendering:"
addParam $w RiOptData MinSamples
addParam $w RiOptData MaxSamples
addParam $w RiOptData MaxRayLevel
addParam $w RiOptData ShadowBias
addMenu $w RiOptData PRManSpec [list RiStandard PRMan]

addText $w ed "Radiosity:"
addParam $w RiOptData RadSteps
addParam $w RiOptData PatchSamples

addText $w es "SearchPaths:"
addString $w RiOptData Textures [list "textures:&" "textures"]
addString $w RiOptData Shaders [list "shaders:&" "shaders"]
addString $w RiOptData Archives [list "archives:&" "archives"]
addString $w RiOptData Procedurals [list "procedurals:&" "procedurals"]

addText $w el "Limits:"
addParam $w RiOptData TextureMem
addParam $w RiOptData GeomMem

