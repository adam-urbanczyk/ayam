set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc mfio.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd
return;