# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2002 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ms.tcl - msgcat "aequivalent"

# ms_set:
#  fill
proc ms_set { lang name val } {

    set varname "${lang}(${name})"

    set ms::$varname $val

 return;
}
# ms_set


# ms_init:
#  
proc ms_init { lang } {

    array set ms::$lang { Dummy 1 }

 return;
}
# ms_init


# ms:
#  return language specific string for name
#
proc ms { name } {
    global ayprefs
    if { ![info exists ms::$ayprefs(Locale)($name)] } {
	# no, return english string
	return [eval subst "\$ms::en($name)"]
    } else {
	# yes, return language specific string
	return [eval subst "\$ms::$ayprefs(Locale)($name)"]
    }
}
# ms

# create namespace ms
namespace eval ms {}

# fill "en"-locale...
ms_init en
ms_set en ayprefse_Shaders "A list of paths where your compiled shaders reside"
ms_set en ayprefse_ScanShaders "Initiate rebuild of internal shader database."
ms_set en ayprefse_Locale "Language to use for balloon help texts.\
\nChanges will take effect after restart of Ayam!"
ms_set en ayprefse_AutoResize "Resize main window according to property gui?"
ms_set en ayprefse_TwmCompat "Is your Window Manager TWM compatible?"
ms_set en ayprefse_ListTypes "Show object types in the tree/list view?"
ms_set en ayprefse_AutoSavePrefs "Save preferences on exit?"
ms_set en ayprefse_LoadEnv "Load environment on startup?"
ms_set en ayprefse_NewLoadsEnv "Load environment on File/New?"
ms_set en ayprefse_EnvFile "Path and name of the environment."
ms_set en ayprefse_Scripts "A list of Tcls scripts to be executed on startup."
ms_set en ayprefse_Plugins "A list of paths where plugins reside."
ms_set en ayprefse_Docs "An URL that points to the documentation."
ms_set en ayprefse_TmpDir "A path where temporary files are to be saved."

ms_set en ayprefse_PickEpsilon "Maximum allowed distance from picked point\
to editable point;\n 0.0 means nearest point wins"
ms_set en ayprefse_HandleSize "Size of the handles of editable points."
ms_set en ayprefse_LazyNotify "Notify parent objects about changes just on\
mouse up?"
ms_set en ayprefse_EditSnaps "Snap coordinates of edited points to grid coordinates?"
ms_set en ayprefse_UndoLevels "Number of undoable modelling steps;\
\n-1 means Undo/Redo is disabled."

# Drawing
ms_set en ayprefse_Tolerance "Sampling tolerance used when tesselating\
NURBS curves or surfaces.\nSmaller values lead to slower rendering but higher\
 quality.\nNURBS objects may override this setting locally."
ms_set en ayprefse_DisplayMode "Determine how surfaces should be drawn\
\nSurface objects may override this setting locally."
ms_set en ayprefse_NCDisplayMode "Determine how curves should be drawn\
\nCurve objects may override this setting locally."
ms_set en ayprefse_UseMatColor "Use color of material for shaded views?"
ms_set en ayprefse_Background "Color to use for the background."
ms_set en ayprefse_Object "Color to use for unselected objects."
ms_set en ayprefse_Selection "Color to use for selected objects."
ms_set en ayprefse_Grid "Color to use for the grids."
ms_set en ayprefse_Tag "Color to use for tagged (selected) points."
ms_set en ayprefse_Shade "Color to use in shaded views when UseMatColor\
\nis not enabled or the object has no material or no material color."
ms_set en ayprefse_Light "Color to use for (unselected) light objects."

# RIB-Export
ms_set en ayprefse_RIBFile "Name of the RIB file to create on Export."
ms_set en ayprefse_Image "Name of the image file created, when rendering\
\nthe exported RIB file."
ms_set en ayprefse_ResInstances "Resolve all instance objects to normal\
\nobjects while exporting to a RIB?"
ms_set en ayprefse_CheckLights "Add a distant headlight to the scene,\
\nif no other light exists?"
ms_set en ayprefse_DefaultMat "Write a default material statement to the\
\nRIB, that will be used by all objects without material?"
ms_set en ayprefse_RIStandard "Omit all attributes and options that are not\
\ncontained in the RenderMan Interface Standard?"
ms_set en ayprefse_WriteIdent "Write an identificator derived from the\
objects name into the RIB?"
ms_set en ayprefse_ShadowMaps "Create a RIB that automatically renders\
ShadowMaps?"
ms_set en ayprefse_ExcludeHidden "Omit hidden objects on RIB export?"
ms_set en ayprefse_QRender "Name and parameters of the renderer to use for\
quick render previews.\n\\\"%s\\\" will be replaced by the filename\
of the RIB."
ms_set en ayprefse_QRenderUI "Enable user interface for quick rendering."
ms_set en ayprefse_QRenderPT "A template that helps to pick the progress\
from the output of the renderer.\n\\\"%d\\\" denotes the position of\
the progress number in the output."
ms_set en ayprefse_Render "Name and parameters of the renderer to use for\
render previews.\n\\\"%s\\\" will be replaced by the filename\
of the RIB."
ms_set en ayprefse_RenderUI "Enable user interface for rendering."
ms_set en ayprefse_RenderPT "A template that helps to pick the progress\
from the output of the renderer.\n\\\"%d\\\" denotes the position of\
the progress number in the output."
ms_set en ayprefse_PPRender "Renderer to use for the permanent preview feature."

# Misc
ms_set en ayprefse_RedirectTcl "Redirect all Tcl error messages to the\
console?"
ms_set en ayprefse_Logging "Log all messages to a file?"
ms_set en ayprefse_LogFile "Path and name of the file to log all messages to."
ms_set en ayprefse_MIResetDM "Reset all DisplayMode attributes of\
all objects on import\nfrom a Mops scene to Global?"
ms_set en ayprefse_MIResetST "Reset all Tolerance attributes of\
all objects on import\nfrom a Mops scene to 0.0?"
ms_set en ayprefse_SaveAddsMRU "Add the name of saved scenes to the\
\nMost-Recently-Used file menu entries?"
ms_set en ayprefse_ToolBoxTrans "Make the toolbox window transient?"
ms_set en ayprefse_ToolBoxShrink "Make the toolbox window shrink wrap around\
its contents,\nwhen the user resizes it?"
ms_set en ayprefse_RGTrans "Make all rendering user interfaces transient?"
ms_set en ayprefse_HideTmpTags "Hide all tags marked temporary from\
the tag property GUIs?"
ms_set en ayprefse_TclPrecision "Precision of Tcl mathematics."

# fill "de"-locale
ms_init de

ms_set de ayprefse_Shaders "Eine Liste von Pfaden, in denen sich übersetzte\
Shader befinden"
ms_set de ayprefse_ScanShaders "Baut interne Shader-Datenbank neu auf."
ms_set de ayprefse_AutoResize "Soll das Hauptfenster sich der Grösse der\
Eigenschaften anpassen?"
ms_set de ayprefse_ListTypes "Sollen die Objekttypen in der Listen bzw.\
Baumansicht angezeigt werden?"
ms_set de ayprefse_Locale "Sprache fuer Hilfe-Texte.\
\nAenderungen werden erst nach Neustart von Ayam wirksam!"
ms_set de ayprefse_AutoSavePrefs "Sollen die Voreinstellungen beim Beenden\
gespeichert werden?"
ms_set de ayprefse_LoadEnv "Soll die Arbeitsumgebung beim Start geladen\
werden?"
ms_set de ayprefse_NewLoadsEnv "Soll die Arbeitsumgebung beim Erstellen\
einer neuen Szene geladen werden?"
ms_set de ayprefse_EnvFile "Pfad und Dateiname der Arbeitsumgebung."
ms_set de ayprefse_Scripts "Eine Liste von Skripten, die beim Starten\
ausgeführt werden sollen."
ms_set de ayprefse_Plugins "Eine Liste von Pfaden, in denen sich\
Plug-Ins befinden."
ms_set de ayprefse_Docs "Eine URL, die auf die Dokumentation verweist."
ms_set de ayprefse_TmpDir "Verzeichnis für temporäre Dateien (z.B. RIBs)."
ms_set de ayprefse_PickEpsilon "Grösste erlaubte Entfernung zwischen\
ausgewähltem und editierbarem Punkt\n
0.0 heisst: nimm immer den nächsten Punkt"
ms_set de ayprefse_HandleSize "Grösse der editierbaren Punkte."
ms_set de ayprefse_LazyNotify "Sollen die Eltern über Änderungen an den\
Kindern nur am Ende\neiner Modellieraktion benachrichtigt werden?"
ms_set de ayprefse_EditSnaps "Sollen editierte Punkte zunächst zu den\
Gitter-Koordinaten bewegt werden?"
ms_set de ayprefse_UndoLevels "Anzahl zurücknehmbarer Modellierschritte;\
\n-1 schaltet das Undo-System aus"

# Drawing
ms_set de ayprefse_Tolerance "Bestimmt die Darstellungsqualitaet von NURBS\
Kurven und Flaechen.\nKleinere Werte fuehren zu hoeherer Qualitaet aber\
langsamerer Darstellung.\nObjekte\
koennen diesen Wert lokal anpassen."
ms_set de ayprefse_DisplayMode "Darstellungsmodus von Flaechen.\
Flaechen koennen den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_NCDisplayMode "Darstellungsmodus von Kurven.\
Kurven koennen den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_UseMatColor "Soll die Materialfarbe fuer schattierte\
Objekte benutzt werden?"
ms_set de ayprefse_Background "Farbe des Hintergrundes."
ms_set de ayprefse_Object "Farbe nicht selektierter Objekte."
ms_set de ayprefse_Selection "Farbe selektierter Objekte."
ms_set de ayprefse_Grid "Farbe des Gitters."
ms_set de ayprefse_Tag "Farbe selektierter Punkte."
ms_set de ayprefse_Shade "Farbe fuer schattierte Objekte,\
wenn UseMatColor nicht aktiviert ist\noder das Objekt kein Material oder\
keine Materialfarbe hat."
ms_set de ayprefse_Light "Farbe fuer nicht selektierte Lichtquellen."

# RIB-Export
ms_set de ayprefse_RIBFile "Name der RIB-Datei fuer den RIB-Export"
ms_set de ayprefse_Image "Name der Bilddatei, die beim Rendern der\
exportierten RIB-Datei erzeugt wird."

ms_set de ayprefse_ResInstances "Sollen alle Instanzen-Objekte waehrend des\
Exportierens in eine RIB-Datei zu normalen Objekten gewandelt werden?"
ms_set de ayprefse_CheckLights "Soll eine Standardlichtquelle hinzugefuegt
werden, wenn keine andere Lichtquelle existiert?"
ms_set de ayprefse_DefaultMat "Standard-Material, das fuer alle Objekte ohne\
eigenes Material benutzt wird."
ms_set de ayprefse_RIStandard "Sollen alle Attribute und Optionen, die nicht\
im RenderMan Standard vorkommen,\nbeim RIB-Export ausgelassen werden?"
ms_set de ayprefse_WriteIdent "Sollen Identifikatoren, basierend auf den\
Objektnamen, in die RIB-Datei\ngeschrieben werden?"
ms_set de ayprefse_ShadowMaps "Sollen ShadowMaps verwendet werden?"
ms_set de ayprefse_ExcludeHidden "Sollen alle versteckten Objekte beim
RIB-Export ausgelassen werden?"
ms_set de ayprefse_QRender "Name und Aufrufparameter des Renderers,
der fuer schnelle Voransicht verwendet werden soll.\n\\\"%s\\\" wird
durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_QRenderUI "Soll das Render-Fenster fuer schnelle\
Voransichten aktiviert werden?"
ms_set de ayprefse_QRenderPT "Beschreibung der Ausgabe des\
Rendering-Fortschritts durch den Renderer\
\n\\\"%d\\\" beschreibt die Position des prozentualen Fortschrittswertes,
der dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_Render "Name und Aufrufparameter des Renderers,
der fuer Voransichten verwendet werden soll.\n\\\"%s\\\" wird
durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_RenderUI "Soll das Render-Fenster fuer Voransichten\
aktiviert werden?"
ms_set de ayprefse_RenderPT "Beschreibung der Ausgabe des\
Rendering-Fortschritts durch den Renderer\
\n\\\"%d\\\" beschreibt die Position des prozentualen Fortschrittswertes,
der dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_PPRender "Renderer, der fuer die permanente Voransicht\
verwendet werden soll."

# Misc
ms_set de ayprefse_RedirectTcl "Sollen alle Fehlermeldungen von Tcl auf\
die Konsole umgelenkt werden?"
ms_set de ayprefse_Logging "Sollen alle Mitteilungen in einer Logdatei\
mitgeschrieben werden?"
ms_set de ayprefse_LogFile "Pfad und Name der Logdatei."
ms_set de ayprefse_MIResetDM "Sollen alle DisplayMode Werte beim Import von\
Mops Szenen auf Global zurueckgesetzt werden?"
ms_set de ayprefse_MIResetST "Sollen alle Tolerance Werte beim Import von\
Mops Szenen auf 0.0 zurueckgesetzt werden?"
ms_set de ayprefse_SaveAddsMRU "Sollen die Namen abgespeicherter Szenen\
den zuletzt-benutzte-Dateien-Eintraegen im Menu hinzugefuegt werden?"
ms_set de ayprefse_ToolBoxTrans "Soll das Werkzeugfenster als transient\
markiert werden?"
ms_set de ayprefse_ToolBoxShrink "Soll das Werkzeugfenster sich dem\
Inhalt anpassen, wenn es in der Groesse veraendert wird?"
ms_set de ayprefse_RGTrans "Sollen alle Rendering Fenster als transient\
gekennzeichnet werden?"
ms_set de ayprefse_HideTmpTags "Sollen temporaere Tags nicht in den Tag\
Eigenschaften dargestellt werden?"
ms_set de ayprefse_TclPrecision "Genauigkeit der Wandlung von Fliesskommazahlen von Tcl."

