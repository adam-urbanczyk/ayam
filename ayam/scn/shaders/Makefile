# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [27. May 2003]
# Makefile for Ayam example scene shaders
# Warning! This Makefile uses GNU make syntax!

SOURCES = RCClouds.sl\
	RSstripessrf.sl\
	DSnoisedsp.sl\
	MFicysrf.sl\
	MFicydsp.sl


# BMRT
SLCSHADERS := $(SOURCES:%.sl=%.slc)
SLC = slc
SLCFLAGS = 

# AQSIS
AQSHADERS := $(SOURCES:%.sl=%.slx)
AQSL = aqsl
AQSLFLAGS =

# 3Delight
3DLSHADERS := $(SOURCES:%.sl=%.sld)
SHADERDL = shaderdl
SDLFLAGS =

# RenderDotC
RDCSHADERS := $(SOURCES:%.sl=%.so)
SHADERDC = shaderdc
SDCFLAGS =

# PRMan
PRMANSHADERS := $(SOURCES:%.sl=%.slo)
SHADER = shader
SFLAGS =

# Air
AIRSHADERS := $(SOURCES:%.sl=%.slb)
SHADERD = shaded
SDFLAGS =

# XML (sl2xml from K3D)
XMLSHADERS := $(SOURCES:%.sl=%.xml)
SL2XML = sl2xml
SL2XMLFLAGS =
CPP = gpp
CPPFLAGS = -C

# Pixie
SDRSHADERS := $(SOURCES:%.sl=%.sdr)
SDRC = sdrc
SDRCFLAGS =

#########

slc: $(SLCSHADERS)

bmrt: slc


slx: $(AQSHADERS)

aqsis: slx


sld: $(3DLSHADERS)

3delight: sld

3dlight: sld


so: $(RDCSHADERS)

rdc: so


slo: $(PRMANSHADERS)

prman: slo


slb: $(AIRSHADERS)

air: slb


xml: $(XMLSHADERS)

k3d: xml


sdr: $(SDRSHADERS)

pixie: sdr


all: slc slx sld so slo slb sdr

clean:
	-rm -rf *.slc
	-rm -rf *.slx
	-rm -rf *.sdl
	-rm -rf *.so
	-rm -rf *.slo
	-rm -rf *.slb
	-rm -rf *.xml
	-rm -rf *.sdr
	-rm -rf *.slpp

.SUFFIXES: .slc .slx .sld .so .slo .slb .xml .sdr .sl

.sl.slc:
	$(SLC) $(SLCFLAGS) $*.sl

.sl.slx:
	$(AQSL) $(AQSLFLAGS) $*.sl

.sl.sld:
	$(SHADERDL) $(SDLFLAGS) $*.sl

.sl.so:
	$(SHADERDC) $(SDCFLAGS) $*.sl

.sl.slo:
	$(SHADER) $(SFLAGS) $*.sl

.sl.slb:
	$(SHADERD) $(SDFLAGS) $*.sl

.sl.xml:
	$(CPP) $(CPPFLAGS) -o $*.slpp $*.sl
	$(SL2XML) $(SL2XMLFLAGS) $*.slpp > $*.xml
	rm -f $*.slpp

.sl.sdr:
	$(SDRC) $(SDRCFLAGS) $*.sl