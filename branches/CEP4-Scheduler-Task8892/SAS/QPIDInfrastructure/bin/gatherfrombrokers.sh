#!/bin/bash


# insert mcu005 exchanges

#for i in $(qpid-stat -e -b mcu005.control.lofar |grep lofar |awk ' { print $1 } ') 
#do
#    echo ./AddHostToQPIDDB.py -b mcu005.control.lofar -e $i
#    ./AddHostToQPIDDB.py -b mcu005.control.lofar -e $i
#done

for host in lhn001.cep2.lofar ccu001.control.lofar mcu001.control.lofar lcs023.control.lofar sas001.control.lofar locus{001..094}.cep2.lofar
do
    for i in $(qpid-stat -q -b $host |grep -v '_' |grep "task\|mom" |awk ' { print $1 } ') 
    do
	echo ./addtoQPIDDB.py -b $host -q $i
	./addtoQPIDDB.py -b $host -q $i
    done
done
