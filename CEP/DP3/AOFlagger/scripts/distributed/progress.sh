#! /bin/bash
declare -i iteration=0
if [[ ! -d out ]] ; then
    ./collect.sh
fi
while true ; do
    clear
    iteration=${iteration}+1
    for l in `ls -1 out|grep lce` ; do
	if [[ -f out/$l/progress.txt ]] ; then
	    cd out/$l
	    last=`ls -1|sort -g|tail -n 1`
	    cd ../..
	    if [[ -f out/$l/${last}/stdout.txt ]] ; then
		echo $l: `cat out/$l/progress.txt` `cat out/$l/${last}/stdout.txt|grep \%|tail -n 1`
	    else
		echo $l: `cat out/$l/progress.txt` -
	    fi
	fi
    done
    sleep 60
    collect.sh
    sleep 1
done
