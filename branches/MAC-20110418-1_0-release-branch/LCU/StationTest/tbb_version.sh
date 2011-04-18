#!/bin/bash
#
# Get version info from the TBB boards and compare this with the expected golden result.
#
# Modified voor INT stations, M.J.Norden 14-10-2010

let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

rm -f tbb_version*.log
rm -f tbb_version*.diff
tbbctl --version > tbb_version.log

if [ $tbboards == 6 ]; then
     # This is a NL station
     diff tbb_version.log gold/tbb_version.gold > tbb_version.diff
     if [ -e tbb_version.log ] && [ -e gold/tbb_version.gold ] && [ -e tbb_version.diff ] && ! [ -s tbb_version.diff ]; then
     # The files exists AND has the diff size 0
         echo "TBB version test went OK"
     else
         echo "TBB version test went wrong"
     fi
else
     # This is a INT station
     diff tbb_version.log gold/tbb_version_int.gold > tbb_version_int.diff
     if [ -e tbb_version.log ] && [ -e gold/tbb_version_int.gold ] && [ -e tbb_version_int.diff ] && ! [ -s tbb_version_int.diff ]; then
     # The files exists AND has the diff size 0
         echo "TBB version test went OK"
     else
         echo "TBB version test went wrong"
     fi
fi
