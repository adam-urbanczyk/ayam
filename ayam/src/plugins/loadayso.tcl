set oldcd [pwd]
cd [file dirname [info script]]
global ay
set pluginname ayso.$ay(soext)
catch {io_lc $pluginname} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the ayso-plugin:
#set ayprefs(Shaders) "/usr/local/shaders"

# uncomment the following line, if you want to scan for shaders
# immediately after loading the ayso-plugin:
#shader_scanAll

return;