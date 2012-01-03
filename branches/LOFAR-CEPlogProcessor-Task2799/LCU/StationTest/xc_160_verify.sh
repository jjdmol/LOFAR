#!/bin/bash

#
# Verify part - Test the SERDES ring between the RSP by verifing the crosslet statistics.
#

rm -f *.dat
rm -f *.diff

# Capture crosscorrelation data for 1 sec

rspctl --xcstat --duration=1


# Verify the captured data

xc_dat=$(ls *.dat)
diff $xc_dat gold/xst_160.gold > xst.diff
if [ -e $xc_dat ] && [ -e gold/xst_160.gold ] && [ -e xst.diff ] && ! [ -s xst.diff ]; then
  # The files exists AND the diff has size 0
  echo "RSP serdes crosscorrelation test at 160 MHz went OK"
else
  echo "RSP serdes crosscorrelation test at 160 MHz went wrong"
fi

