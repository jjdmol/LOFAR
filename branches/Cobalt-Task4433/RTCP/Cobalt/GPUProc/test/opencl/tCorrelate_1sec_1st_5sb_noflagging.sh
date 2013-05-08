#!/bin/bash -x
TESTNAME=`basename "${0%%.sh}"`
./runctest.sh $TESTNAME > $TESTNAME.log 2>&1

