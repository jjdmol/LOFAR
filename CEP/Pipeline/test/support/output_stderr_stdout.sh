#!/bin/bash

# Spam stderr, but not enough to fill a 4k buffer.
# Reading from stderr should not block.
for i in `seq 1 10`; do
  echo e${i}e 1>&2
done

# Now spam stdout, filling any buffer.
# If reading from stderr blocks in the previuos loop,
# we can't finish this loop.
for i in `seq 1 4096`; do
  echo o${i}o o${i}o o${i}o o${i}o o${i}o o${i}o
done
