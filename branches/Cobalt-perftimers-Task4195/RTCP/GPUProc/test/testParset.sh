#!/bin/bash

# Run a parset and compare the output to that in the reference_output directory.

# Include some useful shell functions
. $srcdir/testFuncs.sh

BINDIR=`pwd`/../src

# Some host info
echo "Running as `whoami`"
echo "Running on `hostname`"
echo "Working directory is `pwd`"

# Check for GPU
haveGPU || exit 3

# Check for input files
if [ ! -e /var/scratch/mol/test_sets ]
then
  echo "No input files found -- aborting test."
  exit 3
fi

PARSET=$1

echo "Testing $PARSET"

RUNDIR=`pwd`
OUTDIR=`basename "${0%.run}.in_output"`/`basename "$PARSET"`

function parse_logs
{
  NORMAL=$1
  PROFILED=$2

  test -r $NORMAL || return 1
  test -r $PROFILED || return 1

  # obtain wall time
  WALLTIME=`<$NORMAL perl -ne 'if (/Wall seconds spent.*?([0-9.]+)$/) { print $1; }'`
  # obtain GPU cost
  GPUCOST=`<$PROFILED perl -ne 'if (/GPU  seconds spent computing.*?([0-9.]+)$/) { print $1; }'`

  # log efficiency
  GPUUSAGE=`echo "scale=0;100*$GPUCOST/$WALLTIME" | bc -l`
  echo "Total processing time: $WALLTIME s"
  echo "GPU usage            : $GPUUSAGE %"
}

(
  # create output dir
  mkdir -p $OUTDIR &&
  cd $OUTDIR &&

  # enable debugging
  echo "Global 20" >> RTCP_new.debug &&
  echo "Global 20" >> RTCP.debug &&

  # run correlator -- without profiling
  $BINDIR/RTCP_new $PARSET > performance_normal.txt 2>&1 &&
  # run correlator -- with profiling
  $BINDIR/RTCP_new -p $PARSET > performance_profiled.txt 2>&1 &&

  # check logs
  parse_logs performance_normal.txt performance_profiled.txt &&

  # compare output
  for f in *.MS
  do
    ${srcdir}/cmpfloat.py $f $RUNDIR/tCorrelateRealData.in_reference/`basename "$PARSET"`/$f || exit 1
  done
) || exit 1

cd $RUNDIR

# toss output
rm -rf $OUTDIR

