# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [25. Mar 2012]
# Makefile for the installation of Ayam (precompiled wrapped version)

# Choose an installation prefix.
PREFIX = /usr/local/ayam
#PREFIX = /opt/ayam
#PREFIX = /home/user/ayam

# Where shall the executable be installed?
BINDIR = $(PREFIX)/bin

# Where shall the plugins and custom objects be installed?
LIBDIR = $(PREFIX)/bin/plugins

# Where shall the documentation be installed?
DOCDIR = $(PREFIX)/doc

# Where shall the example scenes be installed?
SCNDIR = $(PREFIX)/scn

INSTALL = ./ninstall
#INSTALL = install

MKDIR = mkdir -p

# Nothing needs to be changed below this line.

DOCFILES = ../Setup.txt ../Changes.txt ../License.txt ../doc/refcard.tcl\
../doc/refcard.pdf ../doc/ayam.pdf

install:
	$(MKDIR) $(BINDIR)
	$(INSTALL) ayam $(BINDIR)
	$(INSTALL) ayam.sh $(BINDIR)
	$(INSTALL) Makefile $(BINDIR)
	$(INSTALL) ayam-desktop.desktop $(BINDIR)
	$(INSTALL) ayam-filetype.xml $(BINDIR)
	-$(INSTALL) ../lib/ayam_icon_32.png $(BINDIR)
	-$(MKDIR) $(BINDIR)/scripts
	-$(INSTALL) scripts/*.tcl $(BINDIR)/scripts
	-$(INSTALL) scripts/*.js $(BINDIR)/scripts
	-$(INSTALL) scripts/readme.txt $(BINDIR)/scripts
	-$(MKDIR) $(LIBDIR)
	-$(INSTALL) plugins/metaobj.so $(LIBDIR)
	-$(INSTALL) plugins/metaobj.tcl $(LIBDIR)
	-$(INSTALL) plugins/metacomp.tcl $(LIBDIR)
	-$(INSTALL) plugins/mfio.so $(LIBDIR)
	-$(INSTALL) plugins/mfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadmfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/mopsi.so $(LIBDIR)
	-$(INSTALL) plugins/mopsi.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadmopsi.tcl $(LIBDIR)
	-$(INSTALL) plugins/objio.so $(LIBDIR)
	-$(INSTALL) plugins/objio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadobjio.tcl $(LIBDIR)
	-$(INSTALL) plugins/dxfio.so $(LIBDIR)
	-$(INSTALL) plugins/dxfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loaddxfio.tcl $(LIBDIR)
	-$(INSTALL) plugins/x3dio.so $(LIBDIR)
	-$(INSTALL) plugins/x3dio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadx3dio.tcl $(LIBDIR)
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
	-$(INSTALL) plugins/aysdr.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadaysdr.tcl $(LIBDIR)
	-$(INSTALL) plugins/aygso.so $(LIBDIR)
	-$(INSTALL) plugins/loadaygso.tcl $(LIBDIR)
	-$(INSTALL) plugins/ayslc.so $(LIBDIR)
	-$(INSTALL) plugins/loadayslc.tcl $(LIBDIR)
	-$(INSTALL) plugins/aycsg.so $(LIBDIR)
	-$(INSTALL) plugins/aycsg.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadaycsg.tcl $(LIBDIR)
	-$(INSTALL) plugins/onio.so $(LIBDIR)
	-$(INSTALL) plugins/onio.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadonio.tcl $(LIBDIR)
	-$(INSTALL) plugins/sdnpatch.so $(LIBDIR)
	-$(INSTALL) plugins/sdnpatch.tcl $(LIBDIR)
	-$(INSTALL) plugins/loadsdnpatch.tcl $(LIBDIR)
	-$(INSTALL) plugins/subdiv.so $(LIBDIR)
	-$(INSTALL) plugins/loadsubdiv.tcl $(LIBDIR)
	-$(INSTALL) plugins/jsinterp.so $(LIBDIR)
	-$(INSTALL) plugins/loadjsinterp.tcl $(LIBDIR)
	-$(INSTALL) plugins/luainterp.so $(LIBDIR)
	-$(INSTALL) plugins/loadluainterp.tcl $(LIBDIR)
	-$(INSTALL) plugins/sfcurve.so $(LIBDIR)
	-$(INSTALL) plugins/loadsfcurve.tcl $(LIBDIR)
	-$(INSTALL) plugins/printps.so $(LIBDIR)
	$(MKDIR) $(DOCDIR)
	$(INSTALL) $(DOCFILES) $(DOCDIR)
	$(MKDIR) $(DOCDIR)/html
	$(MKDIR) $(DOCDIR)/html/pics
	$(INSTALL) ../doc/html/*.html $(DOCDIR)/html
	$(INSTALL) ../doc/html/pics/* $(DOCDIR)/html/pics
	-$(MKDIR) $(SCNDIR)
	-$(INSTALL) ../scn/*.ay $(SCNDIR)
	-$(INSTALL) ../scn/*.xhtml $(SCNDIR)
	-$(MKDIR) $(SCNDIR)/objects
	-$(INSTALL) ../scn/objects/*.ay $(SCNDIR)/objects
	-$(INSTALL) ../scn/objects/*.ttf $(SCNDIR)/objects
	-$(MKDIR) $(SCNDIR)/scripts
	-$(INSTALL) ../scn/scripts/*.ay $(SCNDIR)/scripts
	-$(MKDIR) $(SCNDIR)/shaders
	-$(INSTALL) ../scn/shaders/*.sl ../scn/shaders/Makefile ../scn/shaders/README.txt $(SCNDIR)/shaders
	@echo "Installation of Ayam completed."
	@echo "Add $(BINDIR) to your PATH."
	@echo "Add $(LIBDIR) to your LD_LIBRARY_PATH (or use ayam.sh)."
	@echo "Use 'make -f $(BINDIR)/Makefile uninstall' to uninstall."

# add Gnome/KDE/... user desktop shortcut
desktop:
	-sed -e "s:/usr/local/ayam:$(PREFIX):g" $(BINDIR)/ayam-desktop.desktop > $(BINDIR)/ayam.desktop
	-chmod 755 $(BINDIR)/ayam.desktop
	-xdg-desktop-icon install --novendor $(BINDIR)/ayam.desktop

# add Gnome/KDE/... user menu shortcut
menu:
	-sed -e "s:/usr/local/ayam:$(PREFIX):g" $(BINDIR)/ayam-desktop.desktop > ~/.local/share/applications/ayam.desktop
	-chmod 755 ~/.local/share/applications/ayam.desktop

# add Gnome/KDE/... systemwide menu shortcut
sysmenu:
	-sed -e "s:/usr/local/ayam:$(PREFIX):g" $(BINDIR)/ayam-desktop.desktop > /usr/share/applications/ayam.desktop
	-chmod 755 /usr/share/applications/ayam.desktop

# add Gnome/KDE/... file type
filetype:
	-xdg-mime install $(BINDIR)/ayam-filetype.xml
	-xdg-mime default ayam.desktop application/ayam
	-xdg-icon-resource install --novendor --context mimetypes --size 32 $(BINDIR)/ayam_icon_32.png application-ayam

uninstall:
	-rm -f $(BINDIR)/ayam
	-rm -f $(BINDIR)/ayam.sh
	-rm -f $(BINDIR)/ayam-desktop.desktop
	-rm -f $(BINDIR)/ayam_icon_32.png
	-rm -rf $(BINDIR)/scripts
	-rm -rf $(LIBDIR)
	-rm -rf $(DOCDIR)
	-rm -rf $(SCNDIR)
	-xdg-desktop-icon uninstall ayam.desktop
	-xdg-mime uninstall $(BINDIR)/ayam-filetype.xml
	-rm -f  $(BINDIR)/ayam-filetype.xml
	-rm -f /usr/share/applications/ayam.desktop
	-rm -f ~/.local/share/ayam.desktop
	-rm -f $(BINDIR)/Makefile
	@echo "Uninstallation of Ayam completed."
