# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
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

addText $w e0 "Image:"
addParam $w RiOptData Width
addParam $w RiOptData Height
addCheck $w RiOptData StdDisplay

addText $w e1 "Antialias:"
addParam $w RiOptData Variance
addParam $w RiOptData Samples_X
addParam $w RiOptData Samples_Y
addMenu $w RiOptData FilterFunc [list Gaussian Triangle CatmullRom Box Sinc]
addParam $w RiOptData FilterWidth
addParam $w RiOptData FilterHeight

addText $w e2 "Quantization:"
addParam $w RiOptData ExpGain
addParam $w RiOptData ExpGamma
addParam $w RiOptData RGBA_ONE
addParam $w RiOptData RGBA_MIN
addParam $w RiOptData RGBA_MAX
addParam $w RiOptData RGBA_Dither

addText $w e3 "BMRT-Specific:"
addText $w e4 "Rendering:"
addParam $w RiOptData MinSamples
addParam $w RiOptData MaxSamples
addParam $w RiOptData MaxRayLevel
addParam $w RiOptData ShadowBias
addMenu $w RiOptData PRManSpec [list RiStandard PRMan]

addText $w e5 "Radiosity:"
addParam $w RiOptData RadSteps
addParam $w RiOptData PatchSamples

addText $w e6 "SearchPaths:"
addString $w RiOptData Textures [list "textures:&" "textures"]
addString $w RiOptData Shaders [list "shaders:&" "shaders"]
addString $w RiOptData Archives [list "archives:&" "archives"]
addString $w RiOptData Procedurals [list "procedurals:&" "procedurals"]

addText $w e7 "Limits:"
addParam $w RiOptData TextureMem
addParam $w RiOptData GeomMem

