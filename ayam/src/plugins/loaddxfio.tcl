set oldcd [pwd]
cd [file dirname [info script]]
global ay
set pluginname dxfio.$ay(soext)
catch {io_lc $pluginname} result
if { $result != "" } {
    puts $result
}
cd $oldcd
return;