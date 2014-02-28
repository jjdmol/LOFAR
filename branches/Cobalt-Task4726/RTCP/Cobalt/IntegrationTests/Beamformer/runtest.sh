#!/bin/bash

# Run a parset and compare the output to that in the reference_output directory.
# 
# Syntax: testParset.sh parset [-r reference-output-directory]

# Include some useful shell functions
. $srcdir/../testFuncs.sh

# Set exit status of piped commands to that of the last failed command.
set -o pipefail

TESTNAME="$1"
PARSET="${PWD}/${TESTNAME}.parset"
#OUTDIR="${TESTNAME}.output"

# Some host info
echo "Running test ${TESTNAME}"
echo "  as $(whoami)"
echo "  on $(hostname)"
echo "  in directory $(pwd)"

# Check for GPU
haveGPU || exit 3

# Replace output keys in parset (for now append; TODO: replace)
echo "Observation.DataProducts.Output_CoherentStokes.filenames=" \
     "[${TESTNAME}_beam0.raw, ${TESTNAME}_beam1.raw]" >> ${PARSET}
echo "Observation.DataProducts.Output_CoherentStokes.locations=" \
     "[2*localhost:${PWD}]" >> ${PARSET}

(
  # run an observation
  runObservation.sh -C -F -l 2 ${PARSET} || error "Observation failed!"

  # Bail out with an error, if there are no files to compare
  shopt -s failglob

  # GCC on x86_64 has std::numeric_limits<float>::epsilon() = 1.192092896e-07f
  numfp32eps=\(1.192092896/10000000\)

  # Generally, the first 5 decimals are ok; occasionally, the 5th is off.
  # Hence, 8*num_lim<float>::eps() should be OK. However, try larger epsilons
  # as well to see how large the error actually is.
  for eps_factor in 1024.0 512.0 256.0 128.0 64.0 32.0 16.0 8.0
  do
    EPSILON=$(echo $eps_factor \* $numfp32eps | bc -l)
    for f in ${TESTNAME}_beam*.raw
    do
      cmpfloat --type=float --epsilon=$EPSILON --verbose \
	"${f}" "${srcdir}/${f}" || error "Output does not match " \
	"reference for eps_factor=${eps_factor}"
    done
  done

) || exit 1

