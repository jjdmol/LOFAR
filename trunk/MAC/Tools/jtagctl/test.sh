#!/bin/sh

nc -h > /dev/null 2>&1
if [ $? -ne 1 ]; then
    echo "This testsuite requires the 'nc' utility. Please install it."
    exit 1
fi

OD='od -An -tx1'
NC='nc -l -p 3800'
STUB='./jtagctl -T'
JT='./jtagctl -h localhost'

declare -i passcount=0
declare -i failcount=0

function checkresult
{
    filename=$1
    shift

    cmp $filename $filename.ref > /dev/null 2>&1
    if [ $? -ne 0 ]; then
	echo "FAIL ($filename)"
	failcount=$failcount+1
    else
	echo "PASS ($filename)"
	passcount=$passcount+1
    fi

    rm -f $filename
}

testname=listfiles
$STUB -l | $NC | $OD > $testname.stub 2>&1 &
$JT   -l             > $testname.dat  2>&1
wait
checkresult $testname.stub
checkresult $testname.dat

testname=savefile
$STUB -s testdata.bin | $NC | $OD > $testname.stub 2>&1 &
$JT   -s testdata.bin             > $testname.dat  2>&1
wait
checkresult $testname.stub
checkresult $testname.dat

testname=testexec
$STUB -g genfile -a aplfile | $NC | $OD > $testname.stub 2>&1 &
$JT   -g genfile -a aplfile             > $testname.dat  2>&1
wait
checkresult $testname.stub
checkresult $testname.dat

# report statistics
if [ $failcount -gt 0 ]; then
    echo "$failcount tests FAILED"
fi
if [ $passcount -gt 0 ]; then
    echo "$passcount tests PASSED"
fi
