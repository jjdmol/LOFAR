#!/bin/bash

#
# Test the SERDES ring between the RSP by verifing the crosslet statistics.
#

./xc_160_setup.sh

sleep 2

./xc_160_verify.sh
