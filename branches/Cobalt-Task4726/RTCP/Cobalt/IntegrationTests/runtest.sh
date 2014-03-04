#!/bin/bash

# Run a test and compare the output to the reference output.
# 
# Syntax: runtest.sh <test-name>

# Include some useful shell functions
. $(dirname $0)/testFuncs.sh

TESTNAME="${1}"
OUTDIR="${TESTNAME}.output"
REFDIR="${srcdir}/${OUTDIR}"
PARSET="${PWD}/${TESTNAME}.parset"

# Some host info
echo "Running test ${TESTNAME}"
echo "  as $(whoami)"
echo "  on $(hostname)"
echo "  in directory $(pwd)"

(
  # Create directory if it doesn't yet exist; make sure it's empty
  mkdir -p "${OUTDIR}" || error "Failed to create temporary directory ${OUTDIR}"
  cd "${OUTDIR}" || error "Failed to change directory to ${OUTDIR}"
  rm -rf * || error "Failed to cleanup temporary directory ${OUTDIR}"

  # run an observation
  runObservation.sh -C -F -l 2 "${PARSET}" || error "Observation failed!"

  # Expand wildcard pattern to null string, if there no matching files
  shopt -s nullglob

  # create script to accept output (ie. copy it to the source dir for check in)
  echo "#!/bin/sh
  cp ${PWD}/*.raw ${REFDIR}" > accept_output
  chmod a+x accept_output

  # GCC on x86_64 has std::numeric_limits<float>::epsilon() = 1.192092896e-07f
  numfp32eps=\(1.192092896/10000000\)

  # Generally, the first 5 decimals are ok; occasionally, the 5th is off.
  # Hence, 8*num_lim<float>::eps() should be OK. However, try larger epsilons
  # as well to see how large the error actually is.
  for eps_factor in 1024.0 512.0 256.0 128.0 64.0 32.0 16.0 8.0
  do
    EPSILON=$(echo $eps_factor \* $numfp32eps | bc -l)
    for t in float cfloat
    do
      for f in *.${t}.raw
      do
        cmpfloat --type=${t} --epsilon=${EPSILON} --verbose \
          "${f}" "${REFDIR}/${f}" || error "Output does not match" \
          "reference for eps_factor=${eps_factor}"
      done
    done
  done

) || exit 1

