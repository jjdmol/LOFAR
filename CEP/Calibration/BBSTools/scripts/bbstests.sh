#!/bin/sh
#
# BBS test script that runs individual testbbs python scripts
#
# File:         bbstests.sh
# Author:       Sven Duscha (duscha@astron.nl)
# Date:         2011-07-21
# Last change:  2011-07-21



wd = '/data/scratch/bbstests'
verbose = 0

# Parse command line arguments
while test $# -ne 0
do
  if [ test "${$1}" = "--wd" ]; then
    if [ test $# -le 1 ]; then
        error "${1} needs an additional argument"
    fi
    shift
    wd=${1}
    shift
  elif [ test "${1}" = "--verbose" ]; then
    if test $# -le 1; then
        error "${1} needs an additional argument"
    fi
    shift
    verbose=1
    shift
done



# Loop over remaining command line arguments
# to copy test files to working directory
for arg in $@
do
  if []


done