#!/bin/sh

#
# Check whether nc (netcat) is available,
# it is required to run the tests
#
nc -h > /dev/null 2>&1
if [ $? -ne 1 ]; then
    echo "This testsuite requires the 'nc' utility. Please install it."
    exit 1
fi

#
# Parse arguments
#
if [ $# -eq 1 ]; then
    case $1 in
	-h) echo "usage: ./test.sh [-h | -genref]"
	    echo " -h      this help"
	    echo " -genref generate reference files"
	    exit
	    ;;
	-genref)
	    ref=".ref"
	    ;;
    esac
fi

#
# Definitions
#
OD='od -An -tx1'
NC='nc -l -p 3800'
STUB='./jtagctl -T'
JT='./jtagctl -h localhost'

#
# Global variables
#
declare -i passcount=0
declare -i failcount=0

#
# checkresult
#
# Compare output with reference output
# Keep pass/fail statistics
#
function checkresult
{
    filename=$1
    shift

    cmp $filename $filename.ref > /dev/null 2>&1
    if [ $? -ne 0 ]; then
	echo "FAIL ($filename)"
	failcount=$failcount+1
	diff $filename $filename.ref
    else
	echo "PASS ($filename)"
	passcount=$passcount+1
        rm -f $filename
    fi
}

#
# finalize
# check test results
#
function finalize
{
    if [ $# -ge 1 ]; then testname=$1; shift; fi
    if [ $# -ge 1 ]; then ref=$1;      shift; fi

    if [ "x$ref" = "x" ]; then checkresult $testname.stub; fi
    if [ "x$ref" = "x" ]; then checkresult $testname.dat;  fi
}

#
# usage
#
testname=usage
$STUB > $testname.stub$ref 2>&1 &
$JT   > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# ipupdate
#
testname=ipupdate
$STUB -i 1.2.3.4 | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -i 1.2.3.4             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# savefile
#
testname=savefile
$STUB -s testdata.bin | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -s testdata.bin             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# testexec
#
testname=testexec
$STUB -g genfile -a aplfile | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -g genfile -a aplfile             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# erase (-tg)
#
testname=erase_tg
$STUB -e genfile -tg | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -e genfile -tg             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# erase (-ta)
#
testname=erase_ta
$STUB -e genfile -ta | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -e genfile -ta             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# erase (-tc)
#
testname=erase_tc
$STUB -e genfile -tc | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -e genfile -tc             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

#
# listfiles (erase=false)
#
testname=listfiles
$STUB -l | $NC | $OD > $testname.stub$ref 2>&1 &
$JT   -l             > $testname.dat$ref  2>&1
wait; finalize $testname $ref

# report statistics
if [ $failcount -gt 0 ]; then
    echo "$failcount tests FAILED"
fi
if [ $passcount -gt 0 ]; then
    echo "$passcount tests PASSED"
fi
