#!/bin/bash

# Run a parset and compare the output to that in the reference_output directory.
# 
# Syntax: testParset.sh parset [-r reference-output-directory]

# Include some useful shell functions
. $srcdir/../testFuncs.sh

# Set exit status of piped commands to that of the last failed command.
set -o pipefail

# Set defaults for options
REFDIR=

echo "Invoked as" "$0" "$@"

# Parse command-line options
while getopts "r:" opt
do
  case $opt in
    r)
      REFDIR=$OPTARG
      ;;

    \?)
      error "Invalid option: -$OPTARG"
      ;;

    :)
      error "Option needs argument: -$OPTARG"
      ;;
  esac
done

shift $((OPTIND-1))

PARSET=$1

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

OUTDIR=`basename "${PARSET%.parset}.output"`

(
  # Create temporary output directory and cd into it.
  mkdir -p "$OUTDIR" || error "Failed to create temporary directory $OUTDIR"
  cd "$OUTDIR"

  # run an observation
  runObservation.sh -C -F -l 2 $PARSET || error "Observation failed!"

  # compare output
  if [ -n "$REFDIR" ]
  then
    # create script to accept output (ie. copy it to the source dir for check in)
    echo "#!/bin/bash
    cp `pwd`/*.MS $REFDIR" > accept_output
    chmod a+x accept_output

    # GCC on x86_64 has std::numeric_limits<float>::epsilon() = 1.192092896e-07f
    numfp32eps=\(1.192092896/10000000\)

    # Generally (tCorrelate_*), the first 5 decimals are ok; occasionally,
    # the 5th is off. Hence, 8*num_lim<float>::eps() should be OK. However,
    # try bigger epsilons as well to see how big the error actually is.
    for eps_factor in 1024.0 512.0 256.0 128.0 64.0 32.0 16.0 8.0
    do
      EPSILON=$(echo $eps_factor \* $numfp32eps | bc -l)

      for f in *.MS
      do
        cmpfloat --type=cfloat --epsilon=$EPSILON --verbose \
	    "`pwd`/$f" "$REFDIR/$f" || error "Output does not match " \
	    "reference for eps_factor=$eps_factor"
      done
    done
  fi

  # toss output if everything is ok, but do not fail test if removal fails
#  rm -rf $testdir/$OUTDIR || true # Comment this line for output
) || exit 1

