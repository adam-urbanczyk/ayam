set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc ayslo3d.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the ayslo3d-plugin:
#set ayprefs(Shaders) "your shader searchpath here"

# uncomment the following line, if you want to scan for shader immediately
# after loading the ayslo3d-plugin:
#shader_scanAll

return;