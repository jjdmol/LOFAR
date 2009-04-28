#!/bin/bash

#
# Get version info from the TBB boards and compare this with the expected golden result.
#

rm -f tbb_version.log
rm -f tbb_version.diff
tbbctl --version > tbb_version.log
diff tbb_version.log gold/tbb_version.gold > tbb_version.diff
if [ -e tbb_version.log ] && [ -e gold/tbb_version.gold ] && [ -e tbb_version.diff ] && ! [ -s tbb_version.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB version test went OK"
else
  echo "TBB version test went wrong"
fi
