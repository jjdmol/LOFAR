#! /bin/sh
#
# $Id$
#
echo "set terminal png color" > bla.plot
echo "set output \"bla.blob\"" >>bla.plot
echo -n "plot " >>bla.plot
for i in `echo "select engine_id from workloads group by engine_id" | psql -A -t bb`
do
 for j in 1 2 3 4 5 6
 do
  echo "select extract(epoch from creation),parameterset[${j}] from workloads where engine_id = ${i} order by oid" | psql -t -q -A -F " " bb  > d.${i}.${j}
# echo "select creation,parameterset[${j}] from workloads where engine_id = ${i} order by oid" | psql -t -q -A -F " " bb  | tr "," " " | sed "s/ /-/" | tr -d "{}:-" > d.${i}.${j}
  echo -n "\"d.${i}.${j}\" with lp," >> bla.plot
 done
done
echo "4,8,12,16,20,24" >> bla.plot
echo "pause -1 \"paused, hit the <any>-key\"" >> bla.plot
gnuplot bla.plot
