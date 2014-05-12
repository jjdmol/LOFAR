#!/bin/bash
#
# Determine reference output for osu_bw test, by running osu_bw 20 times and
# calculating the averge throughput figures.
#
# $Id$

for i in $(seq 1 20)
do
  ./osu_bw.test && \
  awk '{print $2}' osu_bw.out > $i
  echo -n "."
done
paste $(seq 1 20) | grep '^[0-9]' | \
  sed -e 's,\t,+,g' -e 's,^,(,' -e 's,$,)/20.,' | bc -l | \
  sed 's,0\+$,,' > avg

{ head -2 osu_bw.out;
  paste <(awk '{print $1}' osu_bw.out | grep '^[0-9]') <(cat avg);
} > osu_bw.ref
