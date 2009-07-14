set oldcd [pwd]
cd [file dirname [info script]]
global ay
set pluginname sdnpatch.$ay(soext)
catch {io_lc $pluginname} result
if { $result != "" } {
    puts $result
}
cd $oldcd
return;