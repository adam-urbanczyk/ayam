#! /bin/sh
echo "#! /bin/sh" &> ./ayam 
echo "$1/ayamsh $2/ayam.tcl \$@" >> ./ayam 2>&1
