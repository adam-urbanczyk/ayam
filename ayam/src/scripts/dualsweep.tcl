# dualsweep.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify"
proc crtSweep { x y z } {
    global transfPropData SweepAttrData
    copOb
    crtOb Sweep
    goDown -1
    pasOb -move
    selOb 0
    getTrafo
    set dx [expr -($transfPropData(Translate_X)+$x)]
    set dy [expr -($transfPropData(Translate_Y)+$y)]
    set dz [expr -($transfPropData(Translate_Z)+$z)]
    movOb $dx $dy $dz
    applyTrafo -all   
    goUp
    selOb -1
    getProp
    set SweepAttrData(Type) 2
    #set SweepAttrData(Sections) 30
    #set SweepAttrData(Interpolate) 1
    setProp
    notifyOb
    convOb -inplace   
}

convOb -inplace NCurve
withOb 0 { getPnt -e -r 0 x0 y0 z0; getPnt -e -r 1 x1 y1 z1; }
selOb 0 1
crtSweep $x0 $y0 $z0 
selOb 0 2
crtSweep $x1 $y1 $z1
selOb 3 4
makecompNP -u -l 1
crtOb NPatch -width 2 -height 2 -cv { 0 0 0 1  1 1 0 1  0 0 1 1  1 1 1 1 }
selOb 3 4 5
tweenNP
selOb 0 1 2 3 4 5
delOb
