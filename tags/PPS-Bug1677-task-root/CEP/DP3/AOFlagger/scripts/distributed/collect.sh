#! /bin/bash
nodes="`cat nodes.txt`"
mkdir -p out
declare -i runningcount=0
declare -i copyingcount=0
declare -i notrunningcount=0
for n in ${nodes} ; do
    runningrfi="`ssh $n -C \"ps aux\"|grep rficonsole|grep ${USER}|grep -v grep`"
    if [[ "${runningrfi}" != "" ]] ; then
	runningcount=${runningcount}+1
    else
	runningscp="`ssh $n -C \"ps aux\"|grep scp|grep ${USER}|grep -v grep`"
	if [[ "${runningscp}" != "" ]] ; then
	    copyingcount=${copyingcount}+1
	    echo -n -e "$n: copying "
	else
	    notrunningcount=${notrunningcount}+1
	    echo -n -e "$n: NOT running! "
	fi
    fi
    mkdir -p "out/${n}"
    scp -q "${n}:/data/scratch/${USER}/stats/out/${n}/*.txt" "out/${n}/"
    nodeprocs=`ssh ${n} -C "ls -1 /data/scratch/offringa/stats/out/${n}/|grep -v txt"`
    for p in ${nodeprocs} ; do
	mkdir -p "out/${n}/${p}"
	scp -rq "${n}:/data/scratch/${USER}/stats/out/${n}/${p}/*count*.txt" "out/${n}/${p}/"
	scp -rq "${n}:/data/scratch/${USER}/stats/out/${n}/${p}/stdout.txt" "out/${n}/${p}/"
	scp -rq "${n}:/data/scratch/${USER}/stats/out/${n}/${p}/stderr.txt" "out/${n}/${p}/"
    done
    echo -e -n .
done
wait
echo -n -e "\nRunning: ${runningcount} Copying: ${copyingcount} Stopped: ${notrunningcount}\n"
