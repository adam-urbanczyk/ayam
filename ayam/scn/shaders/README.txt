[README.txt, 27. May 2003]

This directory contains shaders that are used by the Ayam example scenes.
In order to use them, they need to be compiled for the renderer you use.
Compiling is easy using the supplied Makefile.
Note that the Makefile uses GNU make syntax, so that you may need to
use the gmake instead of the make command, depending on your system.

The Makefile understands the following targets and compiles all shaders
for the respective renderer:
slc/bmrt: BMRT
slx/aqsis: Aqsis
sld/3delight/3dlight: 3Dlight
so/rdc: RenderDotC
slo/prman: PRMan
slb/air: Air
sdr/pixie: Pixie
xml/k3d: Use sl2xml to create an XML representation of the shader parameters
all: compile for all renderers