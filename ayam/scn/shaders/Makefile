# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [16. Sep 2002]
# Makefile for Ayam example scene shaders

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


all: slc slx sld so slo slb

clean:
	-rm -rf *.slc
	-rm -rf *.slx
	-rm -rf *.sdl
	-rm -rf *.so
	-rm -rf *.slo
	-rm -rf *.slb



.SUFFIXES: .slc .slx .sld .so .slo .slb .sl

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
