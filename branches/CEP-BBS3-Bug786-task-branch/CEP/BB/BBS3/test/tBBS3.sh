#!/bin/sh

# Runs as:
#     tBBS3.sh [niter] [username]
#
# default niter is 1.
# default username is $USER
# It is only used if the given tBBS3.test file does not contain an niter line.


# Initialize AIPS++
. /data/aips++/weekly/aipsinit.sh

# import file with parameter database initializations
. ./initparms


niter=$1
shift
if [ "$niter" = "" ]; then
  niter=1
fi
usernm=$1
shift
if [ "$usernm" = "" ]; then
  usernm=$USER
fi

echo "gathering info about run environment"	 
./generalinfo.sh

cdir=`pwd`
echo "Doing tBBS3 tests in $cdir with niter=$niter and usernm=$usernm"
rm -rf /tmp/$usernm.*

for t in run.*; do
  # subtract run. to get the testnamen
  testName=${t#run.}
  echo ""
  echo "Running $testName"
  time (./$t $niter $usernm) >& ${testName}.log
done

