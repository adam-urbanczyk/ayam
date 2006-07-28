# OpenDocument:
#  this first version of OpenDocument is overwritten again in io.tcl!
#  (quickly) save the name of double-clicked documents connected to
#  Ayam.app in a global variable, application startup sequence will
#  later load the files
proc ::tk::mac::OpenDocument { args } {
 global tk_mac_OpenDocuments

    eval [subst "set tk_mac_OpenDocuments {$args}"]

 return;
}
# OpenDocument

if {[string first "-psn" [lindex $argv 0]] == 0} { set argv [lrange $argv 1 end]}
catch {console show}
#tkwait window .
if [catch {source [file dirname [info script]]/tcl/ayam.tcl}] { puts $errorInfo}
