#!/bin/bash -x

# Run a parset and compare the output to that in the reference_output directory.
# 
# Syntax: testParset.sh parset [-r reference-output-directory] [-g minimal-gpu-efficiency]

# Set defaults for options
REFDIR=
GPUEFFICIENCY=0

echo "Invoked as" "$0" "$@"

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

shift $((OPTIND-1))

PARSET=$1

# Include some useful shell functions
. $srcdir/testFuncs.sh

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
OUTDIR=`basename "${PARSET%.parset}.output"`

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

  # be able to find the GPU kernels
  export LOFARROOT=$srcdir/.. &&

  # run correlator -- without profiling
  $RUNDIR/runrtcp.sh $PARSET > performance_normal.txt 2>&1 &&

  # compare output
  if [ "x" != "x$REFDIR" ]
  then
    # create script to accept output (ie. copy it to the source dir for check in)
    echo "#!/bin/bash
    cp `pwd`/*.MS $REFDIR" > accept_output
    chmod a+x accept_output

    for f in *.MS
    do
      ${srcdir}/cmpfloat.py `pwd`/$f $REFDIR/$f || exit 1
    done
  fi &&

  # run correlator -- with profiling
  $RUNDIR/runrtcp.sh -p $PARSET > performance_profiled.txt 2>&1 &&

  # check logs
  parse_logs performance_normal.txt performance_profiled.txt &&

  # toss output if everything is ok
  (cd $RUNDIR && rm -rf $OUTDIR)
) || exit 1

