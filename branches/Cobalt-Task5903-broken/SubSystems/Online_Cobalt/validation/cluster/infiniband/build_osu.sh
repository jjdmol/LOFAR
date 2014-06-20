#!/bin/sh
#
# Build the OSU Micro Benchmark, which can be downloaded from
# http://mvapich.cse.ohio-state.edu/benchmarks/
#
# $Id$

# Use our own custom MPI compiler
export CC=/opt/openmpi/bin/mpicc

# Specify the name and version of the OSU Micro Benchmark
OSU_NAME=osu-micro-benchmarks
OSU_VERSION=4.3
OSU_PKG=$OSU_NAME-$OSU_VERSION

error()
{
  STATUS=$?
  echo >&2 "ERROR: $@"
  exit $STATUS
}

perform()
{
  eval "$@" > /dev/null || error "$@"
}

# Untar the tar ball
perform "[ -d $OSU_PKG ] || tar xf $OSU_PKG.tar.gz"

# Configure and build. Note that we only need to build osu_bw
perform "cd $OSU_PKG"
perform "[ -f Makefile ] || ./configure"
perform "cd mpi/pt2pt"
perform "make"
echo "$PWD/osu_bw"

