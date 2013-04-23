#!/bin/sh

# Remove the shared memory region if the test crashes
trap "ipcrm -M 0x12345678 2>/dev/null || true" EXIT

./runctest.sh tSampleBuffer > tSampleBuffer.log 2>&1
