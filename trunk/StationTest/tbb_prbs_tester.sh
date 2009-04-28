#!/bin/bash

#
# Test the LVDS interfaces between RCU -> RSP -> TBB using the Pseudo Random generator in the RCUs.
#

let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
let nof_rcu=8*$rspboards

rm -f *.log
rm -f *.diff
rm -f ./prbs/*.*


# Set up RCU and RSP, make sure waveform generator is off
rspctl --rcuprsg=1
sleep 1
rspctl --tbbmode=transient

sleep 5
# set up TBB
nof_slices=10   # one slice contains 1024 transient (raw data) samples

tbbctl --free
tbbctl --alloc
tbbctl --rec

sleep 0.1

# Freeze and get the captured data from TBB
cd ./prbs
tbbctl --stop
for ((i = 0; i < $nof_rcu; i++)) do
  tbbctl --readpage=$i,0,$nof_slices
done
cd ..
# Verify the PRBS
python prbs_dir_test.py


echo ""
diff prbs_dir_test.log ./gold/prbs_dir_test.gold > prbs_dir_test.diff
if [ -e prbs_dir_test.log ] && [ -e ./gold/prbs_dir_test.gold ] && [ -e prbs_dir_test.diff ] && ! [ -s prbs_dir_test.diff ]; then
  # The files exists AND has the diff size 0
  echo "RCU -> RSP -> TBB interfaces test went OK"
else
  echo "RCU -> RSP -> TBB interfaces test went wrong"
fi
