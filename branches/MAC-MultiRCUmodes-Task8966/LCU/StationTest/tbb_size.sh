#!/bin/bash

#
# Get version info from the TBB boards and compare this with the expected golden result.
#
# Modified for INT stations, 14-10-2010, M.J.Norden

rm -f tbb_size*.log
rm -f tbb_size*.diff
tbbctl --size > tbb_size.log

let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

if [ $tbboards == 6 ]; then
     # This is a NL station
     diff tbb_size.log gold/tbb_size.gold > tbb_size.diff
     if [ -e tbb_size.log ] && [ -e gold/tbb_size.gold ] && [ -e tbb_size.diff ] && ! [ -s tbb_size.diff ]; then
     # The files exists AND has the diff size 0
         echo "TBB memory size test went OK"
     else
         echo "TBB memory size test went wrong"
     fi
else
     # This is a INT station
     diff tbb_size.log gold/tbb_size_int.gold > tbb_size_int.diff
     if [ -e tbb_size.log ] && [ -e gold/tbb_size_int.gold ] && [ -e tbb_size_int.diff ] && ! [ -s tbb_size_int.diff ]; then
     # The files exists AND has the diff size 0
         echo "TBB memory size test went OK"
     else
         echo "TBB memory size test went wrong"
     fi
fi
