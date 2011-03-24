#! /bin/bash
if [[ "$1" == "" ]] ; then
    cd out
    ~/LOFAR/build/gnu_opt/CEP/DP3/AOFlagger/src/rfistatcollect `ls -1 */*/*counts*.txt|grep -v lce056/2|grep -v lce056/1|grep -v lce056/0`
else
    echo Processing stats for prefix $1...
    cd out
    ~/LOFAR/build/gnu_opt/CEP/DP3/AOFlagger/src/rfistatcollect `ls -1 */*/$1*counts*.txt|grep -v lce056/2|grep -v lce056/1|grep -v lce056/0`
fi
