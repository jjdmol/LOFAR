#!/bin/sh
echo "client localhost 31340" > tConverterStress.in
$lofar_sharedir/runtest.sh tConverterStress > tConverterStress.log
