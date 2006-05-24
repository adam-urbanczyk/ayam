set oldcd [pwd]
cd [file dirname [info script]]
global ay
set pluginname aygso.$ay(soext)
catch {io_lc $pluginname} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the aygso-plugin:
#set ayprefs(Shaders) "your shader searchpath here"

# uncomment the following line, if you want to scan for shaders
# immediately after loading the aygso-plugin:
#shader_scanAll

return;