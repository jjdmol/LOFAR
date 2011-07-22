#!/bin/bash
#
# BBS test script that runs individual testbbs python scripts
#
# File:         bbstests.sh
# Author:       Sven Duscha (duscha@astron.nl)
# Date:         2011-07-21
# Last change:  2011-07-21



bbstestdir='/globaldata/bbs/tests'
wd='/data/scratch/bbstests'
verbosity=0


usage()
{
  echo "usage: ${0} <options> <tests>"
  echo "<options> are "
  echo "--wd          set working directory to execute tests in (default=/data/scratch/bbstests"
  echo "--verbosity   display verbose information on test progress"
  echo "--help        show this help information"
  echo "<tests> to perform"
  echo "calibration   perform a gain calibration on 3C196"
  echo "simulation    simulate a 3C196 two-source-model"
  echo "directional   solve and substract for CasA, Cyga, substract these and correct for 3C196"
}


# Parse command line arguments
args=$@     # keep the arguments
while test $# -ne 0
do
  if test ${1} = "--wd"; then
    if test $# -le 1; then
        error "${1} needs an additional argument"
    fi
    shift
    wd=$1
    shift
  elif test ${1} = "-v" -o "${1}" = "--verbose"; then
    verbosity=1
    shift
  elif test ${1} = "-h" -o "${1}" = "--help"; then
    usage
    exit 0
  else
    shift
  fi
done

# Check if tests working directory does exist
if [ ! -d ${wd}  ]; then
  if [ ${verbosity} = 1 ]; then
    echo "mkdir ${wd}"
  fi
  mkdir ${wd}
fi
# MS directory containing reference Measurementsets used for tests
if [ ! -d ${wd}/MS ]; then
  if [ ${verbosity} = 1 ]; then
    echo "mkdir ${wd}/MS"
  fi
  mkdir ${wd}/MS
fi

# Copy over MS files (if we have to many different MS files
# we might prceed to copy only those needed for individual tests;
# for now we copy the whole MS directory
if [ {$verbosity} = 1 ]; then
  echo "rsync -avz ${bbstestdir}/MS ${wd}/"
  #rsync -avz ${bbstestdir}/MS ${wd}/
else
  echo ""
  #rsync -az ${bbstestdir}/MS ${wd}/
fi

# DEBUG
#for a in $args
#do
#  echo $a
#done
#exit 0

# Loop over remaining command line arguments
# to copy test files to working directory
for arg in $args
do
  if [ $arg = "all" ]; then
    if [ ${verbosity} -eq 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation {$wd}/"
      echo "rsync -avz ${bbstestdir}/calibration {$wd}/"
      echo "rsync -avz ${bbstestdir}/directional {$wd}/"
      #rsync -avz ${bbstestdir}/simulation {$wd}/
      #rsync -avz ${bbstestdir}/calibration {$wd}/
      #rsync -avz ${bbstestdir}/directional {$wd}/
    fi
    #rsync -az ${bbstestdir}/simulation {$wd}/
    #rsync -az ${bbstestdir}/calibration {$wd}/
    #rsync -az ${bbstestdir}/directional {$wd}/
    break
  elif [ $arg = "calibration" ]; then
    if [ ${verbosity} = 1 ]; then 
      echo "rsync -avz ${bbstestdir}/calibration {$wd}/"
      #rsync -avz ${bbstestdir}/calibration {$wd}/
    fi
    #rsync -az ${bbstestdir}/calibration {$wd}/
  elif [ $arg = "simulation" ]; then
    if [ ${verbosity} = 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation {$wd}/"    
      #rsync -avz ${bbstestdir}/simulation {$wd}/
    fi
    #rsync -az ${bbstestdir}/simulation {$wd}/
  elif [ $arg = "directional" ]; then
    if [ ${verbosity} = 1 ]; then 
      echo "rsync -avz ${bbstestdir}/directional {$wd}/"    
      #rsync -avz ${bbstestdir}/directional {$wd}/
    fi
    #rsync -az ${bbstestdir}/directional {$wd}/
  fi
done


# Execute python test scripts
for arg in $@
do
  if [ $arg = "all" ]; then
    if [ ${verbosity} -eq 1 ]; then 
      echo "$wd}/calibration/testBBS_3C196_calibration.py"
      echo "$wd}/simulation/testBBS_3C196_simulation.py"
      echo "{$wd}/directional/testBBS_3C196_direction.py"
      #$wd}/simulation/testBBS_3C196_simulation.py --verbose
      #$wd}/calibration/testBBS_3C196_calibration.py --verbose
      #{$wd}/directional/testBBS_3C196_direction.py --verbose
    fi
    #$wd}/simulation/testBBS_3C196_simulation.py
    #$wd}/calibration/testBBS_3C196_calibration.py
    #{$wd}/directional/testBBS_3C196_direction.py
    break
  if [ $arg = "calibration" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "$wd}/calibration/testBBS_3C196_calibration.py"
      #$wd}/calibration/testBBS_3C196_calibration.py --verbose
    fi
    #rsync -avz ${bbstestdir}/calibration {$wd}/
  elif [ $arg = "simulation" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "$wd}/simulation/testBBS_3C196_simulation.py"
      #$wd}/simulation/testBBS_3C196_simulation.py --verbose
    fi
    #$wd}/simulation/testBBS_3C196_simulation.py
  fi
  elif [ $arg = "directional" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "rsync -avz ${bbstestdir}/directional {$wd}/" 
      #{$wd}/directional/testBBS_3C196_direction.py --verbose
    fi
    #{$wd}/directional/testBBS_3C196_direction.py
  fi
done