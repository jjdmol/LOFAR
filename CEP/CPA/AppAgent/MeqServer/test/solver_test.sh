#!/bin/bash
REQUIRES="../src/meqserver ../pkginc/*/*.g"

for file in $REQUIRES; do
  localname=${file##*/}
  rm -f $localname
  ln -s $file .
done

echo "Running solver_test.g"
glish -l ${srcdir}/solver_test.g -runtest 2>&1 | tee solver_test.log
retval=$?

echo "Glish exited with status $retval"
exit $retval
