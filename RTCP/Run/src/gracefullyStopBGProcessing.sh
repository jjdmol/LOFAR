#!/bin/bash

source locations.sh

echo "cancel all" > /dev/tcp/$FIRSTPSET/4000 &&
echo "quit"       > /dev/tcp/$FIRSTPSET/4000 &&
sleep 5 # allow processes to quit
