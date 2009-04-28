#!/bin/bash

#
# Get version info from the TBB boards and compare this with the expected golden result.
#

rm -f tbb_memory*.log
rm -f tbb_memory*.diff
tbbctl --testddr=0 > tbb_memory0.log
tbbctl --testddr=1 > tbb_memory1.log
tbbctl --testddr=2 > tbb_memory2.log
tbbctl --testddr=3 > tbb_memory3.log
tbbctl --testddr=4 > tbb_memory4.log
tbbctl --testddr=5 > tbb_memory5.log
tbbctl --testddr=6 > tbb_memory6.log
tbbctl --testddr=7 > tbb_memory7.log
tbbctl --testddr=8 > tbb_memory8.log
tbbctl --testddr=9 > tbb_memory9.log
tbbctl --testddr=10 > tbb_memory10.log
tbbctl --testddr=11 > tbb_memory11.log




diff tbb_memory0.log gold/tbb_memory0.gold > tbb_memory0.diff
if [ -e tbb_memory0.log ] && [ -e gold/tbb_memory0.gold ] && [ -e tbb_memory0.diff ] && ! [ -s tbb_memory0.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (0) test went OK"
else
  echo "TBB memory (0) test went wrong"
fi


diff tbb_memory1.log gold/tbb_memory1.gold > tbb_memory1.diff
if [ -e tbb_memory1.log ] && [ -e gold/tbb_memory1.gold ] && [ -e tbb_memory1.diff ] && ! [ -s tbb_memory1.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (1) test went OK"
else
  echo "TBB memory (1) test went wrong"
fi

diff tbb_memory2.log gold/tbb_memory2.gold > tbb_memory2.diff
if [ -e tbb_memory2.log ] && [ -e gold/tbb_memory2.gold ] && [ -e tbb_memory2.diff ] && ! [ -s tbb_memory2.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (2) test went OK"
else
  echo "TBB memory (2) test went wrong"
fi

diff tbb_memory3.log gold/tbb_memory3.gold > tbb_memory3.diff
if [ -e tbb_memory3.log ] && [ -e gold/tbb_memory3.gold ] && [ -e tbb_memory3.diff ] && ! [ -s tbb_memory3.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (3) test went OK"
else
  echo "TBB memory (3) test went wrong"
fi

diff tbb_memory4.log gold/tbb_memory4.gold > tbb_memory4.diff
if [ -e tbb_memory4.log ] && [ -e gold/tbb_memory4.gold ] && [ -e tbb_memory4.diff ] && ! [ -s tbb_memory4.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (4) test went OK"
else
  echo "TBB memory (4) test went wrong"
fi

diff tbb_memory5.log gold/tbb_memory5.gold > tbb_memory5.diff
if [ -e tbb_memory5.log ] && [ -e gold/tbb_memory5.gold ] && [ -e tbb_memory5.diff ] && ! [ -s tbb_memory5.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (5) test went OK"
else
  echo "TBB memory (5) test went wrong"
fi

diff tbb_memory6.log gold/tbb_memory6.gold > tbb_memory6.diff
if [ -e tbb_memory6.log ] && [ -e gold/tbb_memory6.gold ] && [ -e tbb_memory6.diff ] && ! [ -s tbb_memory6.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (6) test went OK"
else
  echo "TBB memory (6) test went wrong"
fi

diff tbb_memory7.log gold/tbb_memory7.gold > tbb_memory7.diff
if [ -e tbb_memory7.log ] && [ -e gold/tbb_memory7.gold ] && [ -e tbb_memory7.diff ] && ! [ -s tbb_memory7.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (7) test went OK"
else
  echo "TBB memory (7) test went wrong"
fi

diff tbb_memory8.log gold/tbb_memory8.gold > tbb_memory8.diff
if [ -e tbb_memory8.log ] && [ -e gold/tbb_memory8.gold ] && [ -e tbb_memory8.diff ] && ! [ -s tbb_memory8.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (8) test went OK"
else
  echo "TBB memory (8) test went wrong"
fi

diff tbb_memory9.log gold/tbb_memory9.gold > tbb_memory9.diff
if [ -e tbb_memory9.log ] && [ -e gold/tbb_memory9.gold ] && [ -e tbb_memory9.diff ] && ! [ -s tbb_memory9.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (9) test went OK"
else
  echo "TBB memory (9) test went wrong"
fi

diff tbb_memory10.log gold/tbb_memory10.gold > tbb_memory10.diff
if [ -e tbb_memory10.log ] && [ -e gold/tbb_memory10.gold ] && [ -e tbb_memory10.diff ] && ! [ -s tbb_memory10.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (10) test went OK"
else
  echo "TBB memory (10) test went wrong"
fi

diff tbb_memory11.log gold/tbb_memory11.gold > tbb_memory11.diff
if [ -e tbb_memory11.log ] && [ -e gold/tbb_memory11.gold ] && [ -e tbb_memory11.diff ] && ! [ -s tbb_memory11.diff ]; then
  # The files exists AND has the diff size 0
  echo "TBB memory (11) test went OK"
else
  echo "TBB memory (11) test went wrong"
fi
