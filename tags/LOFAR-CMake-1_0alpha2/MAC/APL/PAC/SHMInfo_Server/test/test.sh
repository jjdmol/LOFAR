#!/bin/sh
#

curdir=`pwd`
macprot=$curdir/MACall.tse.prot

lofar_home=`echo $curdir | sed -e "s%/LOFAR/.*%/LOFAR%"`
cd $lofar_home/installed/gnu_debug/bin
tse $macprot $curdir/mis.io $curdir/mis.btsw $curdir/test.log
cd $curdir
