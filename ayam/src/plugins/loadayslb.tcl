set oldcd [pwd]
cd [file dirname [info nameofexecutable]]/plugins
catch {io_lc ayslb.so} result
if { $result != "" } {
    puts $result
}
cd $oldcd

# uncomment the following line, if you want to change the Shaders
# preference setting when loading the ayslb-plugin:
#set ayprefs(Shaders) "/tmp/air/shaders"

# uncomment the following line, if you want to scan for shader immediately
# after loading the ayslb-plugin:
#shader_scanAll
return;