# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# script.tcl - script objects Tcl code

global ay Script_props ScriptAttr ScriptAttrData 

set Script_props { Attributes Tags ScriptAttr }

array set ScriptAttr {
arr   ScriptAttrData
sproc setScriptp
gproc getScriptp
w     fScriptAttr

}
# array ScriptAttr

array set ScriptAttrData {
 Type 0
 Script ""
}
# array ScriptAttrData

# create ScriptAttr-UI:
set w [frame $ay(pca).$ScriptAttr(w)]
addMenu $w ScriptAttrData Type [list Inactive Run Create Modify]
pack [text $w.tScript -width 60 -height 60]
eval [subst "bindtags $w.tScript \{$w.tScript Text all\}"]

# Tcl -> C
proc setScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript
    set ScriptAttrData(Script) [$t get 0.0 end]
    setProp

 return;
}
# setScriptp

# C -> Tcl
proc getScriptp { } {
 global ay ScriptAttr ScriptAttrData

    set t $ay(pca).$ScriptAttr(w).tScript
    getProp
    $t delete 0.0 end
    $t insert 0.0 $ScriptAttrData(Script)

 return;
}
# getScriptp
global SCRIPTPLUGIN
if { [info exists SCRIPTPLUGIN] } {
# add menu entry to Create/Custom sub-menu
mmenu_addcustom Script "crtOb Script;uS;sL;"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) Script
}
return;