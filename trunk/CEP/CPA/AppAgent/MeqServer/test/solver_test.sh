#!/bin/sh
REQUIRES="../src/meqserver ../pkginc/*/*.g"

for file in $REQUIRES; do
  localname=${file##*/}
  rm -f $localname
  ln -s $file .
done

glish -l ${srcdir}/solver_test.g -runtest &>solver_test.log

retval=$?
cat solver_test.log
echo "Glish exited with status $retval"
exit $retval
