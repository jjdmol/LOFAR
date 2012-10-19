if [[ "$1" == "" ]] ; then
    echo Syntax: $0 \<ms\>
else
    for (( a=1 ; $a<81 ; a++ )) ; do
	if (( $a<10 )) ; then
	    node="node00$a"
	else
	    node="node0$a"
	fi
	if [[ "`ssh ${node} -C ls /data?/users/lofareor/|grep $1`" != "" ]] ; then
	    nodesubs="`ssh ${node} -C \"find /data?/users/lofareor/$1/\"|grep SB|grep \\\.MS\/ -v`"
	    for n in ${nodesubs} ; do
		echo /net/${node}$n
	    done
	fi
    done
fi
