# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [5. Jun 2002]
# Makefile for the installation of Ayam (precompiled wrapped version)

# Choose an installation prefix.
PREFIX = /usr/local/ayam
#PREFIX = /opt/ayam
# if you use one of the following, be sure to change LIBDIR
# and DOCDIR to a more suitable (Ayam specific) path
#PREFIX = /usr/freeware
#PREFIX = /usr

# Where shall the executable be installed?
BINDIR = ${PREFIX}/bin

# Where shall the plugins and custom objects be installed?
LIBDIR = ${PREFIX}/lib
#LIBDIR = ${PREFIX}/lib/ayam

# Where shall the documentation be installed?
DOCDIR = ${PREFIX}/doc
#DOCDIR = ${PREFIX}/doc/ayam

INSTALL = install
MKDIR = mkdir -p

DOCFILES = ../Setup.txt ../Changes ../License ../doc/refcard.tcl\
../doc/refcard.ps ../doc/ayam.pdf

install: ayamsh
	$(MKDIR) $(BINDIR)
	$(INSTALL) ayam $(BINDIR)
	$(MKDIR) $(LIBDIR)
	-$(INSTALL) plugins/metaobj.so $(LIBDIR)
	-$(INSTALL) plugins/metaobj/metaobj.tcl $(LIBDIR)
	-$(INSTALL) plugins/metaobj/metacomp.tcl $(LIBDIR)
	-$(INSTALL) plugins/mfio.so plugins/mfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/idr.so plugins/idr.tcl $(LIBDIR)
	-$(INSTALL) plugins/rrib.so plugins/rrib.tcl $(LIBDIR)
	$(INSTALL) $(DOCFILES) $(DOCDIR)
	$(MKDIR) $(DOCDIR)/html
	$(MKDIR) $(DOCDIR)/html/pics
	$(INSTALL) ../doc/html/*.html $(DOCDIR)/html
	$(INSTALL) ../doc/html/pics/* $(DOCDIR)/html/pics

uninstall:
	-rm -f $(BINDIR)/ayam
	-rm -rf $(LIBDIR)/metaobj.so
	-rm -rf $(LIBDIR)/metaobj.tcl $(LIBDIR)/metacomp.tcl
	-rm -rf $(LIBDIR)/mfio.so $(LIBDIR)/mfio.tcl
	-rm -rf $(LIBDIR)/idr.so $(LIBDIR)/idr.tcl
	-rm -rf $(LIBDIR)/rrib.so $(LIBDIR)/rrib.tcl
	-cd $(DOCDIR); rm -rf Setup.txt Changes License refcard.tcl\
refcard.ps ayam.pdf html;

