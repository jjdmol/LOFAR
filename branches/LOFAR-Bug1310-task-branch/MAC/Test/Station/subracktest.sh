#!/bin/bash

#
# Run the tests for a LOFAR station subrack
#

# Verify LCU - RSP, LCU - TBB ethernet links by getting the version info
./rsp_version.sh
./tbb_version.sh

# Verify the RSP internal interfaces (Ethernet LCU, Ethernet CEP, 10 Gb Serdes, LVDS ring) by running the BIST
python verify.py --brd rsp0,rsp1,rsp2,rsp3 --rep 1 -v 11 --te tc/bist.py

# Verify the RSP - SPU, TD I2C interface by reading the sensor data
python i2c_spu.py
python i2c_td.py

# Verify the RSP - RSP SERDES links using pseudo random data
python verify.py --brd rsp0,rsp1,rsp2,rsp3 --rep 1 -v 11 --te tc/serdes.py --mode tx_rx

# Verify the RSP - RSP SERDES links by capturing cross correlation statistics
./rsp_xc_200.sh

# Verify the RCU -> RSP LVDS interfaces by capturing pseudo random data on RSP
python verify.py --brd rsp0,rsp1,rsp2,rsp3 --fpga blp0,blp1,blp2,blp3 --pol x,y --rep 1 -v 11 --te tc/prsg.py

# Verify the RCU -> RSP -> TBB LVDS interfaces by capturing pseudo random data on TBB
./tbb_prbs_tester.sh

