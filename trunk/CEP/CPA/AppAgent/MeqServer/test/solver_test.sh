#!/bin/sh
if [ ! -L meqserver ]; then
  ln -s ../src/meqserver .
fi
glish -l ${srcdir}/solver_test.g -runtest &>solver_test.log
retval=$?
cat solver_test.log
echo "Glish exited with status $retval"
exit $retval
