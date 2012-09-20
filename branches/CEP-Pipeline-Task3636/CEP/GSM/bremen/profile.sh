#!/bin/bash
rm sql.log
./pipeline_runner_test.py -q -p -D test $1 | tail -n 1 | cut -d " " -f 3 > tmp.time
cut -f 8 -d " " < sql.log | tab_calc -c sum > tmp.sql
mv sql.log sql_pgsql.log
./pipeline_runner_test.py -q -M -p -D test $1 | tail -n 1 | cut -d " " -f 3 > tmp.time.m
cut -f 8 -d " " < sql.log | tab_calc -c sum > tmp.sql.m
echo $1 $(paste tmp.time tmp.sql tmp.time.m tmp.sql.m) >> profiler.dat
mv sql.log sql_monetdb.log

