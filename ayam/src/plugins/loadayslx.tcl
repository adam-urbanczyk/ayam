set oldcd [pwd]
cd [file dirname [info script]]
global ay
set pluginname ayslx.$ay(soext)
catch {io_lc $pluginname} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the ayslx-plugin:
#set ayprefs(Shaders) "your shader searchpath here"

# uncomment the following line, if you want to scan for shaders
# immediately after loading the ayslx-plugin:
#shader_scanAll

return;