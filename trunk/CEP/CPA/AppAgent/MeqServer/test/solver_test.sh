#!/bin/sh
if [ ! -L meqserver ]; then
  ln -s ../src/meqserver .
fi
glish -l ${srcdir}/solver_test.g -runtest 2>&1 | tee solver_test.log
