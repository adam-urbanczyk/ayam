set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc aysdr.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the aysdr-plugin:
#set ayprefs(Shaders) "/usr/local/shaders"

# uncomment the following line, if you want to scan for shaders
# immediately after loading the aysdr-plugin:
#shader_scanAll

return;