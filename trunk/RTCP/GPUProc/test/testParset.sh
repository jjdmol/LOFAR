#!/bin/bash

# Run a parset and compare the output to that in the reference_output directory.

BINDIR=`pwd`/../src

# Some host info
echo "Running as `whoami`"
echo "Running on `hostname`"
echo "Working directory is `pwd`"

# Check for GPU
if ! lspci | grep -E "VGA|3D" | grep -E "ATI|NVIDIA"
then
  echo "No ATI/NVIDIA graphics cards detected -- aborting test."
  exit 3
fi

# Check for input files
if [ ! -e /var/scratch/mol/test_sets ]
then
  echo "No input files found -- aborting test."
  exit 3
fi

PARSET=$1

echo "Testing $PARSET"

RUNDIR=`pwd`
OUTDIR=`basename "${0%.run}.in_output"`/$PARSET

(
  # create output dir
  mkdir -p $OUTDIR &&
  cd $OUTDIR &&

  # enable debugging
  echo "Global 20" >> RTCP_new.debug &&
  echo "Global 20" >> RTCP.debug &&

  # run correlator
  $BINDIR/RTCP_new $RUNDIR/$PARSET &&

  # compare output
  for f in *.MS
  do
    ${srcdir}/cmpfloat.py $f $RUNDIR/tCorrelateRealData.in_reference/$PARSET/$f || exit 1
  done
) || exit 1

cd $RUNDIR

# toss output
rm -rf $OUTDIR

