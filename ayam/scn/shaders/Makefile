# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [30. Dec 2001]
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

#########

slc: $(SLCSHADERS)

bmrt: slc


slx: $(AQSHADERS)

aqsis: slx


sld: $(3DLSHADERS)

3delight: sld

3dlight: sld


all: slc slx sld

clean:
	rm -rf *.slc
	rm -rf *.slx
	rm -rf *.sdl


.SUFFIXES: .slc .slx .sld .sl

.sl.slc:
	$(SLC) $(SLCFLAGS) $*.sl

.sl.slx:
	$(AQSL) $(AQSLFLAGS) $*.sl

.sl.sld:
	$(SHADERDL) $(SDLFLAGS) $*.sl
