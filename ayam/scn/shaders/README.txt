[README.txt, 2. Sep 2003]

This directory contains shaders for the Ayam example scenes.
In order to use them, they need to be compiled for your renderer first.
Compiling is easy with the help of the supplied Makefile.
Note that the Makefile is in GNU make syntax, so that you may need to
use the "gmake" instead of the "make" command, depending on your system.

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

To compile all shaders for BMRT you, consequently, just have to type:

>gmake slc

or

>gmake bmrt

To get rid of all compiled shaders, just type:

>gmake clean

To add new shaders (that you possibly created on your own) you just
need to add the name of the shader source file to the SOURCES variable
in the Makefile.
