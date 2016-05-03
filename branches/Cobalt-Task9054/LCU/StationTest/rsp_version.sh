#!/bin/bash
#
# Get version info from the RSP boards and compare this with the expected golden result.
#
# Modified voor INT stations, M.J.Norden 14-10-2010

let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

rm -f rsp_version*.log
rm -f rsp_version*.diff
rspctl --version > rsp_version.log

if [ $rspboards == 12 ]; then
     # This is a NL station
     diff rsp_version.log gold/rsp_version.gold > rsp_version.diff
     if [ -e rsp_version.log ] && [ -e gold/rsp_version.gold ] && [ -e rsp_version.diff ] && ! [ -s rsp_version.diff ]; then
     # The files exists AND the diff has size 0
         echo "RSP version test went OK"
     else
         echo "RSP version test went wrong"
     fi
else
     # This is a INT station
     diff rsp_version.log gold/rsp_version_int.gold > rsp_version_int.diff
     if [ -e rsp_version.log ] && [ -e gold/rsp_version_int.gold ] && [ -e rsp_version_int.diff ] && ! [ -s rsp_version_int.diff ]; then
     # The files exists AND the diff has size 0
         echo "RSP version test went OK"
     else
         echo "RSP version test went wrong"
     fi
fi