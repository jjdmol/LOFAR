#!/bin/sh
#
curdir=`pwd`
macprot="$HOME/workspace/LOFAR/MAC/Test/TestHarness/TSE/scripts/MACall.tse.prot"

cd $HOME/workspace/LOFAR/installed/gnu_debug/bin
tse $macprot $curdir/GSOtest.io $curdir/GSOtest.btsw $curdir/GSOtest.log
cd $curdir
