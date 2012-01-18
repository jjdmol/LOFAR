#!/bin/sh
PVSS00ascii -in tPVSSservice.dpl
./runctest.sh tPVSSservice 2>&1 > tPVSSservice.log
