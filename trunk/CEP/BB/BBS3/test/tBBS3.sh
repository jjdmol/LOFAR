#!/bin/sh

# Runs as:
#     tBBS3.sh [niter]
#
# default niter is 1.
# It is only used if the given tBBS3.test file does not contain an niter line.


# Initialize AIPS++
. /data/aips++/weekly/aipsinit.sh

cdir=`pwd`
echo "Doing tBBS3 tests in $cdir"

# Do test1
echo ""
echo "Doing test1"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test1.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test1.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test1 $1 >& test1.out

# Do test2
echo ""
echo "Doing test2"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test2.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test2.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test2 $1 >& test2.out

# Do test3
echo ""
echo "Doing test3"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test3.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test3.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test3 $1 >& test3.out

# Do test4
echo ""
echo "Doing test4"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test4.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test4.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test4 $1 >& test4.out

# Do test5
echo ""
echo "Doing test5"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test5.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test5.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test5 $1 >& test5.out

# Do test6
echo ""
echo "Doing test6"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test6.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test6.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test6 $1 >& test6.out

# Do test7
echo ""
echo "Doing test7"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparms.g; glish setej.g ) >& test7.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test7.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test7 $1 >& test7.out

# Do test8
echo ""
echo "Doing test8"
echo "  Generating parm tables ..."
time ( glish initgsm3-pert8.g; glish setgsm3.g; glish initparms.g ) >& test8.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test8.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test8 $1 >& test8.out

# Do test9
echo ""
echo "Doing test9"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish initparms-pert8.g; glish setej.g ) >& test9.log
rm -f /tmp/test.demo3_gsm-1.MEP
rm -f /tmp/test.demo3-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test9.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test9 $1 >& test9.out

# Do test10
echo ""
echo "Doing test10"
echo "  Generating parm tables ..."
time ( glish initgsm10.g; glish setgsm10.g; glish initparms.g ) >& test10.log
rm -f /tmp/test.demo10_gsm-1.MEP
rm -f /tmp/test.demo10-1.MEP
ln -s $cdir/demo10_gsm.MEP /tmp/test.demo10_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo10-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test10.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test10 $1 >& test10.out

# Do test11
echo ""
echo "Doing test11"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparmsp.g ) >& test11.log
rm -f /tmp/test.demo3p_gsm-1.MEP
rm -f /tmp/test.demo3p-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3p_gsm-1.MEP
ln -s $cdir/demop.MEP /tmp/test.demo3p-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test11.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test11 $1 >& test11.out

# Do test12
echo ""
echo "Doing test12"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparmsp.g ) >& test12.log
rm -f /tmp/test.demo3p_gsm-1.MEP
rm -f /tmp/test.demo3p-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3p_gsm-1.MEP
ln -s $cdir/demop.MEP /tmp/test.demo3p-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test12.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test12 $1 >& test12.out

# Do test13
echo ""
echo "Doing test13"
echo "  Generating parm tables ..."
time ( glish initgsm3.g; glish setgsm3.g; glish initparms.g ) >& test13.log
rm -f /tmp/test.demo3p_gsm-1.MEP
rm -f /tmp/test.demo3p-1.MEP
ln -s $cdir/demo3_gsm.MEP /tmp/test.demo3p_gsm-1.MEP
ln -s $cdir/demo.MEP /tmp/test.demo3p-1.MEP
echo "  Preparing BlackBoard ..."
time ./prepareBBD >> test13.log
echo "  Starting solve ..."
time ./tBBS3 tBBS3.test13 $1 >& test13.out

rm -f /tmp/test.demo*
