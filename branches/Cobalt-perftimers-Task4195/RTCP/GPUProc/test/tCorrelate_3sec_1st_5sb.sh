#!/bin/bash
TESTNAME=`basename "$0" | sed 's/[.][^.]*$//'`
./runctest.sh $TESTNAME > `basename "$0"`.log 2>&1

