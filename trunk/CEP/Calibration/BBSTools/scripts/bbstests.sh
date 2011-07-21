#!/bin/sh
#
# BBS test script that runs individual testbbs python scripts
#
# File:         bbstests.sh
# Author:       Sven Duscha (duscha@astron.nl)
# Date:         2011-07-21
# Last change:  2011-07-21



bbstestdir='/globaldata/bbs/tests'
wd = '/data/scratch/bbstests'
verbose = 0


function usage()
{
  echo "usage: ${0} <options> <tests>"

}


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
    verbose=1
    shift
  elif [ test "${1}" = "--help" ]; then
    usage
    exit(0)
done


# Check if tests working directory does exist
if [ ! -d ${wd}  ]; then
  if [ verbose = 1 ]; then
    echo "mkdir ${wd}"
  fi
  mkdir ${wd}
fi
# MS directory containing reference Measurementsets used for tests
if [ ! -d ${wd}/MS ]; then
  if [ verbose = 1 ]; then
    echo "mkdir ${wd}/MS"
  fi
  mkdir ${wd}/MS
fi


# Loop over remaining command line arguments
# to copy test files to working directory
for arg in $@
do
  # Copy over MS files
  if [ $verbose -eq 1 ]; then
  
    #rsync -avz ${bbstestdir}/MS ${wd}/
  else
    #rsync -az ${bbstestdir}/MS ${wd}/
  fi


  if [ $arg = "all" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation {$wd}/"
      echo "rsync -avz ${bbstestdir}/calibration {$wd}/"
      echo "rsync -avz ${bbstestdir}/directional {$wd}/"
      echo "rsync -avz ${bbstestdir}/MS ${wd}/"
      #rsync -avz ${bbstestdir}/simulation {$wd}/
      #rsync -avz ${bbstestdir}/calibration {$wd}/
      #rsync -avz ${bbstestdir}/directional {$wd}/
    fi
    #rsync -az ${bbstestdir}/simulation {$wd}/
    #rsync -az ${bbstestdir}/calibration {$wd}/
    #rsync -az ${bbstestdir}/directional {$wd}/
    break
  if [ $arg = "calibration" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "rsync -avz ${bbstestdir}/calibration {$wd}/"
      #rsync -avz ${bbstestdir}/calibration {$wd}/
    fi
    #rsync -az ${bbstestdir}/calibration {$wd}/
  elif [ $arg = "simulation" ]; then
    if [ ${verbose} -eq 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation {$wd}/"    
      #rsync -avz ${bbstestdir}/simulation {$wd}/
    fi
    #rsync -az ${bbstestdir}/simulation {$wd}/
  fi
  elif [ $arg = "directional" ]; then
    if [ ${verbose} -eq 1 ]; then 
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
    if [ ${verbose} -eq 1 ]; then 
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