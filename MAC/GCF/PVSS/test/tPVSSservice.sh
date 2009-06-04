#!/bin/sh
PVSS00ascii -in tPVSSservice.dpl
$lofar_sharedir/runtest.sh tPVSSservice 2>&1 > tPVSSservice.log
