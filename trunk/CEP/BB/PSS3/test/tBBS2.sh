#!/bin/sh

# Runs as:
#     tBBS2.sh [niter] [username]
#
# default niter is 1.
# default username is $USER
# It is only used if the given tBBS2.test file does not contain an niter line.


# Initialize AIPS++
. /data/aips++/weekly/aipsinit.sh

niter=$1
shift
if [ "$niter" = "" ]; then
  niter=1
fi
usernm=$1
shift
if [ "$usernm" = "" ]; then
  usernm=$USER
fi

echo "gathering info about run environment"	 
./generalinfo.sh

cdir=`pwd`
echo "Doing tBBS2 tests in $cdir with niter=$niter and usernm=$usernm"
rm -rf /tmp/$usernm.*

# Do test1
echo ""
echo "Doing test1"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test1.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test1.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test1 $niter $usernm >& test1.out

# Do test2
echo ""
echo "Doing test2"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test2.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test2.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test2 $niter $usernm >& test2.out

# Do test3
echo ""
echo "Doing test3"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test3.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test3.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test3 $niter $usernm >& test3.out

# Do test4
echo ""
echo "Doing test4"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test4.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test4.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test4 $niter $usernm >& test4.out

# Do test5
echo ""
echo "Doing test5"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test5.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test5.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test5 $niter $usernm >& test5.out

# Do test6
echo ""
echo "Doing test6"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test6.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test6.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test6 $niter $usernm >& test6.out

# Do test7
echo ""
echo "Doing test7"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparms.g; glish setej.g ) >& test7.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test7.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test7 $niter $usernm >& test7.out

# Do test8
echo ""
echo "Doing test8"
echo "  Generating parm tables ..."
time ( glish initgsm3-pert8.g; glish setgsm3.g; glish initparms.g ) >& test8.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test8.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test8 $niter $usernm >& test8.out

# Do test9
echo ""
echo "Doing test9"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparms-pert8.g; glish setej.g ) >& test9.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test9.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test9 $niter $usernm >& test9.out

# Do test10
echo ""
echo "Doing test10"
echo "  Generating parm tables ..."
time ( glish initgsm10.g; glish setgsm10.g; glish initparms.g ) >& test10.log
rm -f /tmp/$usernm.demo10_gsm-1.MEP
rm -f /tmp/$usernm.demo10-1.MEP
ln -s $cdir/demo10_gsm.MEP /tmp/$usernm.demo10_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo10-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test10.log
echo "  Starting solve ..."
# time ./tBBS2 tBBS2.test10 $niter $usernm >& test10.out

# Do test11
echo ""
echo "Doing test11"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparmsp.g ) >& test11.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3p-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demop.MEP /tmp/$usernm.demo3p-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test11.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test11 $niter $usernm >& test11.out

# Do test12
echo ""
echo "Doing test12"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparmsp.g; glish setejp.g ) >& test12.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3p-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demop.MEP /tmp/$usernm.demo3p-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test12.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test12 $niter $usernm >& test12.out

# Do test13
echo ""
echo "Doing test13"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparms.g; glish setej.g ) >& test13.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> test13.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.test13 $niter $usernm >& test13.out

# Do testNoise
echo ""
echo "Doing testNoise"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test14.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> testNoise.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.testNoise $niter $usernm >& testNoise.out

# Do testFileMap
echo ""
echo "Doing testFileMap"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& testFileMap.log
rm -f /tmp/$usernm.demo3_gsm-1.MEP
rm -f /tmp/$usernm.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/$usernm.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/$usernm.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> testFileMap.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.testFileMap $niter $usernm >& testFileMap.out

# Do testBDB
echo ""
echo "Doing testBDB"
echo "  Generating parm tables ..."
time ( ./createParamTables -dbtype bdb -user $usernm; ./clearParamTables -dbtype bdb -user $usernm; ./initparms -dbtype bdb -user $usernm ) >& testBDB.log
echo "  Preparing BlackBoard ..."
time ./clearBB -user $usernm >> testBDB.log
echo "  Starting solve ..."
time ./tBBS2 tBBS2.testBDB $niter $usernm >& testBDB.out

rm -rf /tmp/$usernm.*
