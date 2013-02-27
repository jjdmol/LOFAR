#!/bin/sh
echo "client localhost 31340" > tConverterStress.in
./runctest.sh tConverterStress > tConverterStress.log
