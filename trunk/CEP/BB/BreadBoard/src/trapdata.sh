#! /bin/sh
#
# $Id$
#
echo "set terminal png color" > trapped.plot
echo "set output \"trapped.png\"" >>trapped.plot
echo "set xdata time" >>trapped.plot
echo "set timefmt \"%Y-%m-%d %H:%M:%S\"" >>trapped.plot
echo -n "set xrange [\"" >>trapped.plot
echo -n `echo "select to_char(min(creation),'YYYY-MM-DD HH24:MI:SS') from workloads" | psql -t -q -A -F " " bb` >> trapped.plot
echo -n "\":\"" >>trapped.plot
echo -n `echo "select to_char(max(creation),'YYYY-MM-DD HH24:MI:SS') from workloads" | psql -t -q -A -F " " bb` >> trapped.plot
echo  "\"] " >>trapped.plot
echo "set format x \"%Y-%m-%d\\\\n%H:%M:%S\"" >> trapped.plot
echo "set timefmt \"%Y-%m-%d %H:%M:%S\"" >>trapped.plot
echo -n "plot " >>trapped.plot
for i in `echo "select engine_id from workloads group by engine_id" | psql -A -t bb`
do
 for j in 1 2 3 4 5 6
 do
# echo "select extract(epoch from creation),parameterset[${j}] from workloads where engine_id = ${i} order by oid" | psql -t -q -A -F " " bb  > d.${i}.${j}
# echo "select '\"',creation,'\"',parameterset[${j}] from workloads where engine_id = ${i} order by oid" | psql -t -q -A -F " " bb  | tr "," " " | sed -e "s/^\" /\"/" | sed -e "s/ \"/\"/" | tr -d "{}" > d.${i}.${j}
  echo "select to_char(creation,'YYYY-MM-DD HH24:MI:SS'),parameterset[${j}] from workloads where engine_id = ${i} order by oid" | psql -t -q -A -F " " bb  | tr "," " " | sed -e "s/^\" /\"/" | sed -e "s/ \"/\"/" | tr -d "{}" > d.${i}.${j}
  echo -n "\"d.${i}.${j}\" using 1:3 t 'engine ${i} param ${j}' with lp," >> trapped.plot
 done
done
echo "4,8,12,16,20,24" >> trapped.plot
gnuplot trapped.plot
