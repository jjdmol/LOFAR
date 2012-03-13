#!/bin/bash

source locations.sh

echo "cancel all" > /dev/tcp/$FIRSTPSET/4000 2>/dev/null &&
echo "quit"       > /dev/tcp/$FIRSTPSET/4000 2>/dev/null &&
sleep 5 # allow processes to quit
