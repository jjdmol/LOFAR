#!/bin/bash

#
# Verify part - Test the SERDES ring between the RSP by verifing the crosslet statistics.
#

rm -f *.dat
rm -f *.diff

# Capture crosscorrelation data for 1 sec

rspctl --xcstat --duration=1


# Verify the captured data, at 200 MHz it is necessary to distinghuis between even or odd sec

xc_dat=$(ls *.dat)
diff $xc_dat gold/xst_200_even.gold > xst.diff
if [ -e $xc_dat ] && [ -e gold/xst_200_even.gold ] && [ -e xst.diff ] && ! [ -s xst.diff ]; then
  # The files exists AND the diff has size 0
  echo "RSP serdes crosscorrelation test even second went OK"
else
  diff $xc_dat gold/xst_200_odd.gold > xst.diff
  if [ -e $xc_dat ] && [ -e gold/xst_200_odd.gold ] && [ -e xst.diff ] && ! [ -s xst.diff ]; then
    # The files exists AND the diff has size 0
    echo "RSP serdes crosscorrelation test odd second went OK"
  else
    echo "RSP serdes crosscorrelation test went wrong"
  fi
fi

