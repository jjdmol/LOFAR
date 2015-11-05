#!/bin/bash
#
# Determine reference output for osu_bw test, by running osu_bw on each of
# the Cobalt nodes and calculating the averge throughput figures.
#
# $Id$

HOSTS=$(for i in $(seq 1 8); do printf "cbt%03d " $i; done)

rm -f *.bw
for host in $HOSTS
do
  ssh $host $PWD/osu_bw.test
  for target in $HOSTS
  do
    [ "$host" == "$target" ] && continue;
    f="osu_bw.$target.out"
    awk '{print $2}' $f > $host.$f.bw
  done
done

n=$(echo *.bw | wc -w)

paste *.bw | grep '^[0-9]' | \
  sed -e 's,\t,+,g' -e 's,^,scale=2; (,' -e 's,$,)/'$n',' | \
  bc -l > avg

{ head -2 $f;
  paste <(awk '{print $1}' $f | grep '^[0-9]') <(cat avg);
} > osu_bw.ref
