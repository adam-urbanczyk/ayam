#! /bin/tcsh
foreach i (html/*.html)
cat $i | sed -e "s/<FIGURE>/<CENTER><FIGURE>/g" >! ga
cat ga | sed -e "s/<\/FIGURE>/<\/FIGURE><\/CENTER>/g" >! ga2
cat ga2 | sed -e "s/<CAPTION>/<CAPTION><P>/g" >! $i
end
rm ga ga2
