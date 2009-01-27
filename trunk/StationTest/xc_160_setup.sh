#!/bin/bash

#
# Setup part - Test the SERDES ring between the RSP by verifing the crosslet statistics.
#

pi=$(echo "4*a(1)" | bc -l)

nof_subbands=512

# Start waveform generators for crosscorrelation measurement

nof_rcu=32         # nof RCU in a subrack
sample_freq=160000000
rf_freq=40000000
xc_subband=$(echo "$nof_subbands * $rf_freq / ($sample_freq/2)" | bc -l)

for ((i = 0; i < $nof_rcu; i++)) do
    phs=$(echo "($i * 2 * $pi) / $nof_rcu" | bc -l)
    amp=$(echo "0.498 * $i / $nof_rcu" | bc -l)              # use 0.498 for deterministic result, see bug 767
    rspctl --wg=$rf_freq --select=$i --ampli=$amp --phase=$phs
done

rspctl --xcsubband=$xc_subband
