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
to editable point;\n 0.0 means nearest point wins."
ms_set en ayprefse_HandleSize "Size of the handles of editable points."
ms_set en ayprefse_LazyNotify "Notify parent objects about changes just on\
mouse up?"
ms_set en ayprefse_EditSnaps "Snap coordinates of edited points to grid\
coordinates?"
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
ms_set en ayprefse_ShadowMaps "Should ShadowMaps be used?\nAutomatic: Yes,\
create a RIB that automatically renders ShadowMaps all the time.\
\nManual: Yes, but the ShadowMaps will be rendered on user request only
(Menu: View/Create ShadowMaps)"
ms_set en ayprefse_ExcludeHidden "Omit hidden objects on RIB export?"
ms_set en ayprefse_RenderMode "How should the renderer be forced to render\
to the screen?"
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
ms_set en ayprefse_SMRender "Name and parameters of the renderer to use for\
shadow maps.\n\\\"%s\\\" will be replaced by the filename\
of the RIB."
ms_set en ayprefse_SMRenderUI "Enable user interface for shadow map rendering."
ms_set en ayprefse_SMRenderPT "A template that helps to pick the progress\
from the output of the shadow map renderer.\n\\\"%d\\\" denotes the position\
of the progress number in the output."
ms_set en ayprefse_PPRender "Renderer to use for the permanent preview feature."

# Misc
ms_set en ayprefse_RedirectTcl "Redirect all Tcl error messages to the\
console?"
ms_set en ayprefse_Logging "Log all messages to a file?"
ms_set en ayprefse_LogFile "Path and name of the file to log all messages to."
ms_set en ayprefse_SaveAddsMRU "Add the name of saved scenes to the\
\nMost-Recently-Used file menu entries?"
ms_set en ayprefse_ToolBoxTrans "Make the toolbox window transient?"
ms_set en ayprefse_ToolBoxShrink "Make the toolbox window shrink wrap around\
its contents,\nwhen the user resizes it?"
ms_set en ayprefse_RGTrans "Make all rendering user interfaces transient?"
ms_set en ayprefse_HideTmpTags "Hide all tags marked temporary from\
the tag property GUIs?"
ms_set en ayprefse_TclPrecision "Precision of Tcl mathematics."

# Mops-Import
ms_set en mopsi_options_ResetDM "Reset all DisplayMode attributes of\
imported objects to \\\"Global\\\"?"
ms_set en mopsi_options_ResetST "Reset all Tolerance attributes of\
imported objects to \\\"0.0\\\"?"

#
# fill "de"-locale
ms_init de

ms_set de ayprefse_Shaders "Eine Liste von Verzeichnissen, in denen sich\
\nübersetzte Shader befinden."
ms_set de ayprefse_ScanShaders "Baut interne Shader-Datenbank neu auf."
ms_set de ayprefse_AutoResize "Soll das Hauptfenster sich der Größe der\
Eigenschaften anpassen?"
ms_set de ayprefse_TwmCompat "Ist der verwendete Fenster-Manager\
zu TWM kompatibel?"
ms_set de ayprefse_ListTypes "Sollen die Objekttypen in der Listen bzw.\
\nBaumansicht angezeigt werden?"
ms_set de ayprefse_Locale "Sprache für Hilfe-Texte.\
\nÄnderungen werden erst nach Neustart von Ayam wirksam!"
ms_set de ayprefse_AutoSavePrefs "Sollen die Voreinstellungen beim Beenden\
gespeichert werden?"
ms_set de ayprefse_LoadEnv "Soll die Arbeitsumgebung beim Start geladen\
werden?"
ms_set de ayprefse_NewLoadsEnv "Soll die Arbeitsumgebung beim Erstellen\
einer neuen Szene geladen werden?"
ms_set de ayprefse_EnvFile "Vollständiger Dateiname der Arbeitsumgebung."
ms_set de ayprefse_Scripts "Eine Liste von Skripten, die beim Starten\
ausgeführt werden sollen."
ms_set de ayprefse_Plugins "Eine Liste von Verzeichnissen, in denen sich\
Plug-Ins befinden."
ms_set de ayprefse_Docs "Eine URL, die auf die Dokumentation verweist."
ms_set de ayprefse_TmpDir "Verzeichnis für temporäre Dateien."

# Modeling
ms_set de ayprefse_PickEpsilon "Größte erlaubte Entfernung zwischen\
ausgewähltem und editierbarem Punkt;\
\n0.0 wählt jedoch immer den nächsten Punkt."
ms_set de ayprefse_HandleSize "Größe der editierbaren Punkte."
ms_set de ayprefse_LazyNotify "Sollen die Eltern über Änderungen an den\
\nKindern nur am Ende einer Modellieraktion\nbenachrichtigt werden?"
ms_set de ayprefse_EditSnaps "Sollen editierte Punkte zunächst zu den\
\nGitter-Koordinaten bewegt werden?"
ms_set de ayprefse_UndoLevels "Anzahl zurücknehmbarer Modellierschritte;\
\n-1 schaltet das Undo-System aus."

# Drawing
ms_set de ayprefse_Tolerance "Bestimmt die Darstellungsqualität von NURBS\
Kurven und Flächen.\nKleinere Werte führen zu höherer Qualität aber\
langsamerer Darstellung.\nObjekte\
können diesen Wert lokal anpassen."
ms_set de ayprefse_DisplayMode "Darstellungsmodus von Flächen.\
\nFlächen können den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_NCDisplayMode "Darstellungsmodus von Kurven.\
\nKurven können den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_UseMatColor "Soll die Materialfarbe für schattierte\
\nObjekte benutzt werden?"
ms_set de ayprefse_Background "Farbe des Hintergrundes."
ms_set de ayprefse_Object "Farbe nicht selektierter Objekte."
ms_set de ayprefse_Selection "Farbe selektierter Objekte."
ms_set de ayprefse_Grid "Farbe des Gitters."
ms_set de ayprefse_Tag "Farbe selektierter Punkte."
ms_set de ayprefse_Shade "Farbe für schattierte Objekte,\
wenn UseMatColor nicht aktiviert ist oder\ndas Objekt kein Material oder\
keine Materialfarbe hat."
ms_set de ayprefse_Light "Farbe für nicht selektierte Lichtquellen."

# RIB-Export
ms_set de ayprefse_RIBFile "Name der RIB-Datei für den RIB-Export"
ms_set de ayprefse_Image "Name der Bilddatei, die beim Rendern\nder\
exportierten RIB-Datei erzeugt wird."
ms_set de ayprefse_ResInstances "Sollen alle Instanzen-Objekte während des\
Exportierens\nin normale Objekte umgewandelt werden?"
ms_set de ayprefse_CheckLights "Soll eine Standardlichtquelle hinzugefügt\
werden,\nwenn keine andere Lichtquelle existiert?"
ms_set de ayprefse_DefaultMat "Standard-Material, das für alle Objekte\nohne\
eigenes Material benutzt wird."
ms_set de ayprefse_RIStandard "Sollen alle Attribute und Optionen,\ndie nicht\
im RenderMan Standard vorkommen,\nbeim RIB-Export ausgelassen werden?"
ms_set de ayprefse_WriteIdent "Sollen Identifikatoren, basierend auf den\
Objektnamen,\nin die RIB-Datei geschrieben werden?"
ms_set de ayprefse_ShadowMaps "Sollen ShadowMaps verwendet werden?"
ms_set de ayprefse_ExcludeHidden "Sollen alle versteckten Objekte beim
RIB-Export ausgelassen werden?"
ms_set de ayprefse_RenderMode "Wie soll der Renderer zum Rendern auf den\
Bildschirm gezwungen werden?"
ms_set de ayprefse_QRender "Name und Aufrufparameter des Renderers,
der für schnelle Voransicht verwendet werden soll.\n\\\"%s\\\" wird
durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_QRenderUI "Soll das Render-Fenster für schnelle\
Voransichten aktiviert werden?"
ms_set de ayprefse_QRenderPT "Beschreibung der Ausgabe des\
Rendering-Fortschritts durch den Renderer\
\n\\\"%d\\\" beschreibt die Position des prozentualen Fortschrittswertes,
der dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_Render "Name und Aufrufparameter des Renderers,
der für Voransichten verwendet werden soll.\n\\\"%s\\\" wird
durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_RenderUI "Soll das Render-Fenster für Voransichten\
aktiviert werden?"
ms_set de ayprefse_RenderPT "Beschreibung der Ausgabe des\
Rendering-Fortschritts durch den Renderer\
\n\\\"%d\\\" beschreibt die Position des prozentualen Fortschrittswertes,
der dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_SMRender "Name und Aufrufparameter des Renderers,
der für ShadowMaps verwendet werden soll.\n\\\"%s\\\" wird\
durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_SMRenderUI "Soll das Render-Fenster für ShadowMaps\
aktiviert werden?"
ms_set de ayprefse_SMRenderPT "Beschreibung der Ausgabe des\
Rendering-Fortschritts durch den ShadowMap-Renderer\
\n\\\"%d\\\" beschreibt die Position des prozentualen Fortschrittswertes,
der dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_PPRender "Renderer, der für die permanente Voransicht\
verwendet werden soll."

# Misc
ms_set de ayprefse_RedirectTcl "Sollen alle Fehlermeldungen von Tcl auf\
die Konsole umgelenkt werden?"
ms_set de ayprefse_Logging "Sollen alle Mitteilungen in einer Logdatei\
mitgeschrieben werden?"
ms_set de ayprefse_LogFile "Pfad und Name der Logdatei."
ms_set de ayprefse_SaveAddsMRU "Sollen die Namen abgespeicherter Szenen\
den\nzuletzt-benutzte-Dateien-Einträgen im\nHauptmenü hinzugefügt werden?"
ms_set de ayprefse_ToolBoxTrans "Soll das Werkzeugfenster als transient\
markiert werden?"
ms_set de ayprefse_ToolBoxShrink "Soll das Werkzeugfenster sich dem\
Inhalt anpassen, wenn es in der Größe verändert wird?"
ms_set de ayprefse_RGTrans "Sollen alle Rendering Fenster als transient\
markiert werden?"
ms_set de ayprefse_HideTmpTags "Sollen temporäre Tags aus den Tag\
Eigenschaften ausgeblendet werden?"
ms_set de ayprefse_TclPrecision "Genauigkeit der Wandlung von Gleitkommazahlen von Tcl."

# Mops-Import
ms_set de mopsi_options_ResetDM "Sollen alle DisplayMode-Attribute von\
importierten Objekten auf \\\"Global\\\" zurückgesetzt werden?"
ms_set de mopsi_options_ResetST "Sollen alle Tolerance-Attribute von\
importierten Objekten auf \\\"0.0\\\" zurückgesetzt werden?"


#
# fill "fr"-locale...
ms_init fr
ms_set fr ayprefse_Shaders " Liste de chemins où résident vos shaders\
compilés."
ms_set fr ayprefse_ScanShaders "Initialise la reconstruction de la base\
interne Shader."
ms_set fr ayprefse_Locale "Langue des bulles d'aide.\
\nLe changement prendra effet après le redémarrage d'Ayam!"
ms_set fr ayprefse_AutoResize "Redéfinir la taille de la fenêtre principale\
selon les propriétés par défaut."
ms_set fr ayprefse_TwmCompat "Votre gestionnaire de fenêtre est-il\
compatible TWM?"
ms_set fr ayprefse_ListTypes "Montrer les types d'objets dans la vue en\
arbre/liste?"
ms_set fr ayprefse_AutoSavePrefs "Sauvegarder les paramètres en quittant?"
ms_set fr ayprefse_LoadEnv "Charger l'environnement au démarrage?"
ms_set fr ayprefse_NewLoadsEnv "Charger l'environnement lors de\
l'ouverture/la création d'un fichier?"
ms_set fr ayprefse_EnvFile "Chemin et nom de l'environnement."
ms_set fr ayprefse_Scripts "Liste de scripts TCL à exécuter au démarrage."
ms_set fr ayprefse_Plugins "Liste de chemins où se trouvent les plugins."
ms_set fr ayprefse_Docs "URL poitant vers la documentation."
ms_set fr ayprefse_TmpDir "Chemin vers le dossier de sauvegarde des fichiers\
temporaire."

ms_set fr ayprefse_PickEpsilon "Distance maximale autorisée du point\
sélectionné vers le point éditable;\n0.0 signifie le point le plus proche."
ms_set fr ayprefse_HandleSize "Taille des pognées des points éditables."
ms_set fr ayprefse_LazyNotify "Notifier les objets parent à propos des\
changement simplement au survol de souris?"
ms_set fr ayprefse_EditSnaps "Faire correspondre les coordonnées des points\
édités avec celles de la grille?"
ms_set fr ayprefse_UndoLevels "Nombre de pas d'annulation dans l'historique\
de modélisation;\n-1 correspond à la désactivation de l'historique."

# Drawing
ms_set fr ayprefse_Tolerance "Tolérance utilisée pour les courbes NURBS\
ou les surface surfaces.\nLes petites valeurs induisent un rendu plus lent\
mais une meilleure qualité.\nLes objets NURBS peuvent outrepasser\
localement cette configuration."
ms_set fr ayprefse_DisplayMode "Détermine comment les surfaces seront tracées\
\nLes objets Surface objects peuvent outrepasser localement cette\
configuration."
ms_set fr ayprefse_NCDisplayMode "Détermine comment les surfaces seront\
tracées\nLes objets Courbe objects peuvent outrepasser localement\
cette configuration."
ms_set fr ayprefse_UseMatColor "Utiliser les couleurs du matériau pour\
les vue ombrées?"
ms_set fr ayprefse_Background "Couleur de fond."
ms_set fr ayprefse_Object "Couleur pour les objets non sélectionnés."
ms_set fr ayprefse_Selection "Couleur pour les objets sélectionnés."
ms_set fr ayprefse_Grid "Couleur pour les grilles"
ms_set fr ayprefse_Tag "Couleur pour les points (é)sélectionnés."
ms_set fr ayprefse_Shade "Couleur pour les vues ombrées lorsque que\
UseMatColor\nn'est as activé ou que l'objet n'a pas de matériau\
affecté\nou pas de coueur de matériau."
ms_set fr ayprefse_Light "Couleur pour les objets Lumière (sélectionnés)."

# RIB-Export
ms_set fr ayprefse_RIBFile "Nom du fichier RIB à créer lors de l'Exportation."
ms_set fr ayprefse_Image "Nom du fichier image lors du rendu du fichier\
RIB exporté."
ms_set fr ayprefse_ResInstances "Résoudre toutes les instance des objets\
à des objets\nnormaux lors de l'exportation RIB?"
ms_set fr ayprefse_CheckLights "Ajouter une source lumineuse distante,\
\nsi aucune source n'a été définie?"
ms_set fr ayprefse_DefaultMat "Définir un matériau par défaut dans le fichier\
\nRIB; il sera utilisé par tous les objets sans matériau?"
ms_set fr ayprefse_RIStandard "Omettre tous les attributs et toutes les\
options non définis\nen standard dans l'Interface RenderMan?"
ms_set fr ayprefse_WriteIdent "Ecrire un identificateur dérivé\
du nom des objets dans le fichier RIB?"
ms_set fr ayprefse_ShadowMaps "Utiliser les ShadowMaps?\nAutomatic: Oui,\
Créer un RIB activant tout le temps le rendu ShadowMaps.\
\nManual: Oui, mais le rendu des ShadowMaps sera activé par l'utilisateur\
seulement\n(Menu: View/Create ShadowMaps)"
ms_set fr ayprefse_ExcludeHidden "Omettre les objets cachés à l'exportation\
RIB?"
ms_set fr ayprefse_RenderMode "Mode de rendu à l'écran?"
ms_set fr ayprefse_QRender "Nom et paramètres du moteur de rendu à utiliser\
pour les rendus d'aperçu rapides\n\\\"%s\\\" sera remplacé par le nom\
du fichier du RIB."
ms_set fr ayprefse_QRenderUI "Activer l'interface utilisateur lors le\
rendu rapide."
ms_set fr ayprefse_QRenderPT "Modèle pour la progression à partir de la\
sortie du moteur de rendu.\n\\\"%d\\\" montre la position du numéro\
de progression dans la sortie."
ms_set fr ayprefse_Render "Nom et paramètres du moteur de rendu à utiliser\
pour les rendus d'aperçu.\n\\\"%s\\\" sera remplacé par le nom du fichier du\
RIB."
ms_set fr ayprefse_RenderUI "Activer l'interface utilisateur lors le rendu."
ms_set fr ayprefse_RenderPT "Modèle pour la progression à partir de la\
sortie du moteur de rendu.\n\\\"%d\\\" montre la position du numéro de\
progression dans la sortie."
ms_set fr ayprefse_SMRender "Nom et paramètres du moteur de rendu à utiliser\
pour les shadow maps.\n\\\"%s\\\" sera remplacé par le nom du fichier du\
RIB."
ms_set fr ayprefse_SMRenderUI "Enable user interface for shadow map rendering."
ms_set fr ayprefse_SMRenderPT "Modèle pour la progression à partir de la\
sortie du moteur de rendu.\n\\\"%d\\\" montre la position du numéro de\
progression dans la sortie."
ms_set fr ayprefse_PPRender "Moteur de rendu à utiliser pour l'aperçu\
permanent."

# Misc
ms_set fr ayprefse_RedirectTcl "Rediriger tous les messages d'erreur\
TCL vers la console?"
ms_set fr ayprefse_Logging "Sauvegarder tous les message dans un fichier?"
ms_set fr ayprefse_LogFile "Chemin et nom du fichier de sauvegarde des\
messages?"
ms_set fr ayprefse_SaveAddsMRU "Ajouter le nom des scène sauvegardées à\
l'entrée de menu\nMost-Recently-Used files?"
ms_set fr ayprefse_ToolBoxTrans "Rendre la fenêtre Boîte-à-Outils transient?"
ms_set fr ayprefse_ToolBoxShrink "Rendre la fenêtre Boîte-à-Outils ajustable\
à son contenu,\nlorsque l'utilisateur en modifie la taille?"
ms_set fr ayprefse_RGTrans "Rendre toutes les interface utilisateur transient?"
ms_set fr ayprefse_HideTmpTags "Cacher tous les étiquette marquées\
temporaires dans les propriétés de l'interface graphique?"
ms_set fr ayprefse_TclPrecision "Precision des mathématiques Tcl."

ms_set fr mopsi_options_ResetDM "Réinitialiser à Global tous les attributs\
DisplayMode de tous les objets lors de l'importation\ndepuis une scène Mops?"
ms_set fr mopsi_options_ResetST "Réinitialiser à 0.0 tous les attributs\
Tolerance de tous les objets lors de l'importation\ndepuis une scène Mops?"
