#! /bin/bash
rm -f baseline-slopes.txt
rm -fr out-*x* -rf
for (( a1=0 ; ${a1} < 27 ; a1++ )) ; do
    for (( a2=${a1}+1 ; ${a2} < 27 ; a2++ )) ; do
	echo Baseline ${a1}x${a2}...
	./collect-baseline.sh ${a1}x${a2} 2> /dev/null
	echo Baseline ${a1}x${a2} >> baseline-slopes.txt
	./process.sh ${a1}x${a2}|grep slope >> baseline-slopes.txt
	tail -n 2 baseline-slopes.txt
	rm out-${a1}x${a2} -rf
    done
done
