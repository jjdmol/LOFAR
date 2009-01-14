#!/bin/bash

#
# Test the LVDS interfaces between RCU -> RSP -> TBB using the Pseudo Random generator in the RCUs.
#

nof_rcu=32

rm -f *.log
rm -f *.diff
rm -f *.dat
rm -f *.nfo


# Set up RCU and RSP, make sure waveform generator is off
rspctl --wg=0
rspctl --rcuprsg=1
rspctl --tbbmode=transient

# Set up TBB
nof_slices=10   # one slice contains 1024 transient (raw data) samples

tbbctl --free
tbbctl --alloc
tbbctl --rec

sleep 5

# Freeze and get the captured data from TBB
tbbctl --stop
for ((i = 0; i < $nof_rcu; i++)) do
  tbbctl --readpage=$i,0,$nof_slices
done

# Verify the PRBS
python prbs_dir_test.py


echo ""
diff prbs_dir_test.log prbs_dir_test.gold > prbs_dir_test.diff
if [ -e prbs_dir_test.log ] && [ -e prbs_dir_test.gold ] && [ -e prbs_dir_test.diff ] && ! [ -s prbs_dir_test.diff ]; then
  # The files exists AND has the diff size 0
  echo "RCU -> RSP -> TBB interfaces test went OK"
else
  echo "RCU -> RSP -> TBB interfaces test went wrong"
fi
