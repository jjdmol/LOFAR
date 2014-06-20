#!/bin/sh
declare phaserad
declare startidx
declare stopidx
declare -i nrcus=16
for ((idx = 0; idx < $nrcus; idx++)) do
    phaserad=$(eval "echo \"($idx * 2 * 3.141592654) / $nrcus\" | bc -l")
    ampl=$(eval "echo \"$idx/$nrcus\" | bc -l")
    eval "rspctl --wg=50e6 --select=$idx --ampli=$ampl --phase=$phaserad"
#    eval "rspctl --wg=40e6 --select=$idx --phase=$phaserad"
done
rspctl --xcsubband=256

