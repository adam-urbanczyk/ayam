# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [7. Jun 2002]
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

INSTALL = install
MKDIR = mkdir -p

DOCFILES = ../Setup.txt ../Changes.txt ../License.txt ../doc/refcard.tcl\
../doc/refcard.ps ../doc/ayam.pdf

install:
	$(MKDIR) $(BINDIR)
	$(INSTALL) ayam $(BINDIR)
	-$(LN) $(LIBDIR) $(BINDIR)/plugins
	$(MKDIR) $(LIBDIR)
	-$(INSTALL) plugins/metaobj.so $(LIBDIR)
	-$(INSTALL) plugins/metaobj.tcl $(LIBDIR)
	-$(INSTALL) plugins/metacomp.tcl $(LIBDIR)
	-$(INSTALL) plugins/mfio.so plugins/mfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/idr.so plugins/idr.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadidr.tcl $(LIBDIR)
	-$(INSTALL) plugins/rrib.so plugins/rrib.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadrrib.tcl $(LIBDIR)
	$(MKDIR) $(DOCDIR)
	$(INSTALL) $(DOCFILES) $(DOCDIR)
	$(MKDIR) $(DOCDIR)/html
	$(MKDIR) $(DOCDIR)/html/pics
	$(INSTALL) ../doc/html/*.html $(DOCDIR)/html
	$(INSTALL) ../doc/html/pics/* $(DOCDIR)/html/pics
	@echo "Installation completed."
	@echo "Add $(BINDIR) to your PATH."
	@echo "Add $(LIBDIR) to your LD_LIBRARY_PATH."

uninstall:
	-rm -f $(BINDIR)/ayam
	-rm -rf $(LIBDIR)/metaobj.so
	-rm -rf $(LIBDIR)/metaobj.tcl $(LIBDIR)/metacomp.tcl
	-rm -rf $(LIBDIR)/mfio.so $(LIBDIR)/mfio.tcl
	-rm -rf $(LIBDIR)/idr.so $(LIBDIR)/idr.tcl
	-rm -rf $(LIBDIR)/rrib.so $(LIBDIR)/rrib.tcl
	-cd $(DOCDIR); rm -rf Setup.txt Changes.txt License.txt refcard.tcl\
refcard.ps ayam.pdf html;

