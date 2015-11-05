#!/bin/bash

# Run a parset and compare the output to that in the reference_output directory.
# 
# Syntax: testParset.sh parset [-r reference-output-directory] [-g minimal-gpu-efficiency]

# Set defaults for options
REFDIR=
GPUEFFICIENCY=0

# Parse options
while getopts "r:g:" opt
do
  case $opt in
    r)
      REFDIR=$OPTARG
      ;;

    g)
      GPUEFFICIENCY=$OPTARG
      ;;

    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;

    :)
      echo "Option needs argument: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

PARSET=$1

# Include some useful shell functions
. $srcdir/testFuncs.sh

BINDIR=`pwd`/../../src

# Some host info
echo "Running as `whoami`"
echo "Running on `hostname`"
echo "Working directory is `pwd`"

# Check for GPU
haveGPU || exit 3

# Check for input files
if [ ! -e /var/scratch/mol/test_sets ]
then
  echo "No input files found -- aborting test." >&2
  exit 3
fi

echo "Testing $PARSET"

RUNDIR=`pwd`
OUTDIR=`basename "${PARSET%.parset}.in_output"`

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

  if [ "$GPUUSAGE" -lt $GPUEFFICIENCY ]
  then
    echo "ERROR: GPU usage < $GPUEFFICIENCY% -- considering test a failure." >&2
    return 1
  fi

  return 0
}

(
  # create output dir
  mkdir -p $OUTDIR &&
  cd $OUTDIR &&

  # enable debugging
  echo "Global 20" >> rtcp.debug &&

  # run correlator -- without profiling
  mpirun -H localhost -np 3 $BINDIR/rtcp_opencl $PARSET > performance_normal.txt 2>&1 &&
  # run correlator -- with profiling
  mpirun -H localhost -np 3 $BINDIR/rtcp_opencl -p $PARSET > performance_profiled.txt 2>&1 &&

  # compare output
  if [ "x" != "x$REFDIR" ]
  then
    # create script to accept output (ie. copy it to the source dir for check in)
    echo "#!/bin/bash
    cp `pwd`/*.MS $REFDIR" > accept_output
    chmod a+x accept_output

    for f in *.MS
    do
      ${srcdir}/cmpfloat.py $f $REFDIR/$f || exit 1
    done
  fi &&

  # check logs
  parse_logs performance_normal.txt performance_profiled.txt
) || exit 1

cd $RUNDIR

# toss output
rm -rf $OUTDIR

