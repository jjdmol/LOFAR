#!/bin/sh

# this script collect some information about this host

FILE=testgeneralinfo.out

echo "GENERAL INFORMATION FOR BBSTESTANALYZER" > $FILE

echo "BBSTest: hostname" >> $FILE
hostname >> $FILE

echo "BBSTest: runDate" >> $FILE
date >> $FILE

echo "BBSTest: loadAverage" >> $FILE
cat /proc/loadavg >> $FILE

echo "BBSTest: memory" >> $FILE
free >> $FILE

echo "BBSTest: kernel" >> $FILE
uname -a >> $FILE
cat /proc/cmdline >> $FILE

echo "BBSTest: users" >> $FILE
users >> $FILE

echo "BBSTest: CPUs" >> $FILE
cat /proc/cpuinfo >> $FILE


