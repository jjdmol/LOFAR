#!/bin/bash

#
# Get version info from the TBB boards and compare this with the expected golden result.
#

rm -f tbb_size.log
rm -f tbb_size.diff
tbbctl --size > tbb_size.log
diff tbb_size.log gold/tbb_size.gold > tbb_size.diff
if [ -e tbb_size.log ] && [ -e gold/tbb_size.gold ] && [ -e tbb_size.diff ] && ! [ -s tbb_size.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory size test went OK"
else
  echo "TBB memory size test went wrong"
fi
