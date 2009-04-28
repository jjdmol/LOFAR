#!/bin/sh
# 1.0 ppsctl check of the pps system
# 31-03-09, M.J Norden

echo "check 1 puls per second"
sudo ppsctl -m -ta -p/dev/oncore.pps.0

