set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc rrib.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd
return;