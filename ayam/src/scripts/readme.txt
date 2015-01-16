[readme.txt - 16. Jan 2015]

This directory contains a collection of various scripts for Ayam.

This file gives just an overview about what is here; for more complete
documentation refer to the scripts or the main Ayam documentation.

The scripts can be grouped in three categories, external helpers, internal
helpers, and script object scripts.

External Helpers
----------------

Those scripts are designed to run in any Tcl/Tk shell, not necessarily
inside Ayam.

bgconvert.tcl - use Ayam as background 3D file format converter

ayrepair.tcl - repair the Ayam application state (emergency use only!)

winheader.tcl - helper to create header files from the original sources
 (for compilation on MS Windows)


Internal Helpers
----------------

Many of those scripts can be added to the preference setting Scripts.

GUI helpers:
2lcons.tcl - restrict the console to two lines height
colfocus.tcl - color the focus ring
intfd.tcl - use internal file dialogs (MacOSX only!)
kdialog.tcl - use KDE dialogs (kdialog)
zdialog.tcl - use Gnome dialogs (zenity)
myicons.tcl - replace icon image with user defined ones

showwin.tcl - show all application windows
zap.tcl - hide all application windows


Modelling Helpers:
easysweep.tcl - add helper to custom menu for easy sweep creation from
 only one selected curve (which becomes the trajectory of the sweep)
tonpatch.tcl - add helper to custom menu that converts all objects in place
 to plain NURBS patches
topoly.tcl - add helper to custom menu that converts all objects in place
 to plain PolyMesh objects
useaqsisapp.tcl - use Aqsis.app as renderer (MacOSX only!)
usepixie.tcl - use Pixie as renderer (MacOSX only!)
aac.tcl - automatically invoke the about center variants of actions
apnt.tcl - automatically invoke the point modelling mode after each point
 selection
rc.tcl - revert cursor key bindings for parallel (modelling) views
ssp.tcl - save point selection to tags

Misc:
aytest.tcl - test Ayam

setglobal.tcl - demonstrates how to set global variables not reachable
 via the preferences

oldnames.tcl - add aliases to make scripts written before Ayam 1.16 work,
 also comprehensively lists which commands actually have been renamed
 (from 1.15 to 1.16)


Script Object Scripts
---------------------

To make use of any of these, add the script code to a Script object
of the designated type (see the script).

addmyst.tcl - add chordal parameterized texture coordinates (as PV tag)
 to a NURBS patch object (type: Modify)
bendcur.tcl - bend a curve according to the shape of a second curve
 (type: Modify)
conscir.tcl - create a circular B-Spline whose control points are
 confined to a radial configuration (type: Run)
crtlinegui.tcl - create line, with GUI (type: Create)
crtspiral.tcl - create spirals (type: Create)
crtspiralgui.tcl - create spirals with GUI (type: Create)
polyhedron.js - create polyhedrons from Conway notation
                (needs jsinterp plugin!) (type: Create)
tcone.tcl - truncated cone primitive with GUI (type: Create)
