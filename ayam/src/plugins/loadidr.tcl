set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc idr.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd
return;