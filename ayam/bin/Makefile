# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [24. Mar 2004]
# Makefile for the installation of Ayam (precompiled wrapped version)

# Choose an installation prefix.
PREFIX = /usr/local/ayam
#PREFIX = /opt/ayam
# if you use one of the following, be sure to change LIBDIR
# and DOCDIR to a more suitable (Ayam specific) path
#PREFIX = /usr/freeware
#PREFIX = /usr

# if PREFIX points to a generic directory (e.g. /usr) and not
# to a directory specific to Ayam (e.g. /usr/local/ayam) you
# probably want to switch LN to echo (this way the installation
# will not create a softlink named "plugins" in PREFIX/bin)
LN = ln -s
#LN = echo

# Where shall the executable be installed?
BINDIR = $(PREFIX)/bin

# Where shall the plugins and custom objects be installed?
LIBDIR = $(PREFIX)/lib
#LIBDIR = $(PREFIX)/lib/ayam

# Where shall the documentation be installed?
DOCDIR = $(PREFIX)/doc
#DOCDIR = $(PREFIX)/doc/ayam

INSTALL = ./ninstall
#INSTALL = install
MKDIR = mkdir -p

# Nothing needs to be changed below this line.

DOCFILES = ../Setup.txt ../Changes.txt ../License.txt ../doc/refcard.tcl\
../doc/refcard.ps ../doc/ayam.pdf

install:
	$(MKDIR) $(BINDIR)
	$(INSTALL) ayam $(BINDIR)
	$(MKDIR) $(LIBDIR)
	-$(LN) $(LIBDIR) $(BINDIR)/plugins
	-$(INSTALL) plugins/metaobj.so $(LIBDIR)
	-$(INSTALL) plugins/metaobj.tcl $(LIBDIR)
	-$(INSTALL) plugins/metacomp.tcl $(LIBDIR)
	-$(INSTALL) plugins/mfio.so $(LIBDIR)
	-$(INSTALL) plugins/mfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadmfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/idr.so $(LIBDIR)
	-$(INSTALL) plugins/idr.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadidr.tcl $(LIBDIR)
	-$(INSTALL) plugins/rrib.so $(LIBDIR)
	-$(INSTALL) plugins/rrib.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadrrib.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayslb.so $(LIBDIR)
	-$(INSTALL) plugins/loadayslb.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayslo.so $(LIBDIR)
	-$(INSTALL) plugins/loadayslo.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayslo3d.so $(LIBDIR)
	-$(INSTALL) plugins/loadayslo3d.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayslx.so $(LIBDIR)
	-$(INSTALL) plugins/loadayslx.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayso.so $(LIBDIR)
	-$(INSTALL) plugins/loadayso.tcl $(LIBDIR)
	-$(INSTALL) plugins/aysdr.so $(LIBDIR)
	-$(INSTALL) plugins/loadaysdr.tcl $(LIBDIR)
	-$(INSTALL) plugins/aycsg.so $(LIBDIR)
	-$(INSTALL) plugins/loadaycsg.tcl $(LIBDIR)
	-$(INSTALL) Makefile $(LIBDIR)
	$(MKDIR) $(DOCDIR)
	$(INSTALL) $(DOCFILES) $(DOCDIR)
	$(MKDIR) $(DOCDIR)/html
	$(MKDIR) $(DOCDIR)/html/pics
	$(INSTALL) ../doc/html/*.html $(DOCDIR)/html
	$(INSTALL) ../doc/html/pics/* $(DOCDIR)/html/pics
	@echo "Installation completed."
	@echo "Add $(BINDIR) to your PATH."
	@echo "Add $(LIBDIR) to your LD_LIBRARY_PATH."
	@echo "Use 'make -f $(LIBDIR)/Makefile uninstall' to uninstall."

uninstall:
	-rm -f $(BINDIR)/ayam
	-rm -rf $(LIBDIR)/metaobj.so
	-rm -rf $(LIBDIR)/metaobj.tcl $(LIBDIR)/metacomp.tcl
	-rm -rf $(LIBDIR)/mfio.so $(LIBDIR)/mfio.tcl $(LIBDIR)/loadmfio.tcl
	-rm -rf $(LIBDIR)/idr.so $(LIBDIR)/idr.tcl
	-rm -rf $(LIBDIR)/rrib.so $(LIBDIR)/rrib.tcl
	-rm -rf $(LIBDIR)/loadrrib.tcl $(LIBDIR)/loadidr.tcl
	-rm -rf $(LIBDIR)/ayslb.so $(LIBDIR)/loadayslb.tcl
	-rm -rf $(LIBDIR)/ayslo.so $(LIBDIR)/loadayslo.tcl
	-rm -rf $(LIBDIR)/ayslo3d.so $(LIBDIR)/loadayslo3d.tcl
	-rm -rf $(LIBDIR)/ayslx.so $(LIBDIR)/loadayslx.tcl
	-rm -rf $(LIBDIR)/ayso.so $(LIBDIR)/loadayso.tcl
	-rm -rf $(LIBDIR)/aysdr.so $(LIBDIR)/loadaysdr.tcl
	-rm -rf $(LIBDIR)/aycsg.so $(LIBDIR)/loadaycsg.tcl
	-cd $(DOCDIR); rm -rf Setup.txt Changes.txt License.txt refcard.tcl\
refcard.ps ayam.pdf html;
	-rm -rf $(LIBDIR)/Makefile
