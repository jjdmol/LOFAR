#!/bin/sh
#

curdir=`pwd`
macprot="$HOME/workspace/LOFAR/MAC/Test/TestHarness/TSE/scripts/MACall.tse.prot"

cd $HOME/workspace/LOFAR/installed/gnu_debug/bin
tse $macprot $curdir/LCU_increment2.io $curdir/LCU_increment2.btsw $curdir/test.log
cd $curdir
