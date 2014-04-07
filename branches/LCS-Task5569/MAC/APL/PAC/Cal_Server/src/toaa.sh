#!/bin/sh

# script to convert list of coordinates to format
# suitable for inclusion in AntennaArrays.conf.in

# input format: one 2d coordinate per RCU pair per line
# 1,23 4,56
# 7,89 0,12
#
# output format: two 3d coordinates per RCU pair, all on one line
# comma (,) replaced by dot (.)
# 1.23 4.56 0 1.23 4.56 0 7.89 0.12 0 7.89 0.12 0
#
# Usage: cat CS010C_positions.dat | ./toaa.sh

cat | tr "," "." | awk '{ printf $1 " " $2 " " 0 " " $1 " " $2 " " 0 " "; }'

