#! /bin/bash
declare -i iteration=0
if [[ ! -d out ]] ; then
    ./collect.sh
fi
while true ; do
    clear
    iteration=${iteration}+1
    for l in `ls -1 out|grep node` ; do
	if [[ -f out/$l/progress.txt ]] ; then
	    cd out/$l
	    last=`ls -1|sort -g|tail -n 1`
	    cd ../..
	    if [[ -f out/$l/${last}/stdout.txt ]] ; then
		echo $l: `cat out/$l/progress.txt` `cat out/$l/${last}/stderr.txt|grep \%|tail -n 1`
	    else
		echo $l: `cat out/$l/progress.txt` -
	    fi
	fi
    done
    echo End of iteration.
    sleep 60
    collect.sh
    sleep 1
done
