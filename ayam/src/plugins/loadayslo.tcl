set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc ayslo.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the ayslo-plugin:
#set ayprefs(Shaders) "your shader searchpath here"

# uncomment the following line, if you want to scan for shader immediately
# after loading the ayslo-plugin:
#shader_scanAll

return;