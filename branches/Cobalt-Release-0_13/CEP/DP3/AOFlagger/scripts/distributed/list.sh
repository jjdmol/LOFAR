if [[ "$1" == "" ]] ; then
    MS="L2010_20981"
#MS="L2010_08450"
else
    MS="$1"
fi
for (( a=1 ; $a<23 ; a++ )) ; do
    if (( $a<10 )) ; then
	lse="lse00$a"
    else
	lse="lse0$a"
    fi
    if [[ "`ssh ${lse} -C ls /data3/|grep ${MS}`" != "" ]] ; then
	lsesubs="`ssh ${lse} -C \"find /data3/${MS}/\"|grep SB|grep \\\.MS\/ -v`"
	for n in ${lsesubs} ; do
	    echo ${lse}\:$n
	done
    fi
done
