# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# oldnames.tcl - make renamed commands accessible under old names

# add new elements in the scheme
# lappend cmds {<oldcommand> <newcommand>}

set cmds [list {revert revertC}]

lappend cmds {revertU revertuS}
lappend cmds {revertV revertvS}
lappend cmds {swapUV swapuvS}
lappend cmds {elevateNPV elevatevNP}
lappend cmds {elevateNPV elevatevNP}
lappend cmds {clampNPU clampuNP}
lappend cmds {clampNPV clampvNP}
lappend cmds {insknNPU insknuNP}
lappend cmds {insknNPV insknvNP}
lappend cmds {splitNPU splituNP}
lappend cmds {splitNPV splitvNP}
lappend cmds {rescaleKnNC rescaleknvNC}
lappend cmds {rescaleKnNP rescaleknNP}
lappend cmds {closeNPU closeuNP}
lappend cmds {closeNPV closevNP}
lappend cmds {cmovOb pasmovOb}
lappend cmds {show showOb}
lappend cmds {hide hideOb}
lappend cmds {selPoints selPnts}
lappend cmds {invSelPoints invPnts}
lappend cmds {movSel movPnts}
lappend cmds {rotSel rotPnts}
lappend cmds {scalSel scalPnts}

#lappend cmds {splitNPV splitvNP}


foreach cmd $cmds {
    set oldcmd [lindex $cmd 0]
    set newcmd [lindex $cmd 1]
    eval proc $oldcmd \{args\} \{ $newcmd \$args \}
}