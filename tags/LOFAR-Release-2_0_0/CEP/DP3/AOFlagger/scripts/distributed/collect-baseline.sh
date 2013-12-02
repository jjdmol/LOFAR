#! /bin/bash
nodes="`cat nodes.txt`"
baseline=$1
if [[ "${baseline}" == "" ]] ; then
    echo Syntax: collect-baseline.sh \<baseline\>
    echo \<baseline\> has format like 5x7.
else
    echo Collecting baseline ${baseline}
    for n in ${nodes} ; do
	echo -n -e "${n}... "
	mkdir -p "out-${baseline}/${n}"
	nodeprocs=`ssh ${n} -C "ls -1 /data/scratch/${USER}/stats/out/${n}/|grep -v txt"`
	for p in ${nodeprocs} ; do
	    mkdir -p "out-${baseline}/${n}/${p}"
	    scp -rq "${n}:/data/scratch/${USER}/stats/out/${n}/${p}/baseline-${baseline}*.txt" "out-${baseline}/${n}/${p}/" &
	done
    done
    wait
fi
