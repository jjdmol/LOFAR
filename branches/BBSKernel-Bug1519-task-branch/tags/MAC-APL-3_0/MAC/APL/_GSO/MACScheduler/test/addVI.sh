#!/bin/sh
#
curdir=`pwd`
echo $curdir
cd ../build/gnu_debug/test
./addVI $curdir/$1 $2 $3 $4
cd $curdir
