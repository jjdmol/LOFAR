#!/bin/bash

#
# Get version info from the TBB boards and compare this with the expected golden result.
#

rm -f tbb_memory*.log
rm -f tbb_memory*.diff

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  exit 1
fi


for ((ind=0; ind < $tbboards; ind++)) do
        tbbctl --testddr=$ind > tbb_memory$ind.log
      done

for ((ind=0; ind < $tbboards; ind++)) do
            diff tbb_memory$ind.log gold/tbb_memory$ind.gold > tbb_memory$ind.diff
            if [ -e tbb_memory$ind.log ] && [ -e gold/tbb_memory$ind.gold ] && [ -e tbb_memory$ind.diff ] && ! [ -s tbb_memory$ind.diff ]; then
              # The files exists AND has the diff size 0
              echo "TBB memory ($ind) test went OK"
            else
              echo "TBB memory ($ind) test went wrong"
            fi
      done

