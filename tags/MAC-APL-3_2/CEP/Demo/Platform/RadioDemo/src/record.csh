#!/bin/csh
#
# 03-09-2001  AXC
#
# script to start cleaning the wavedir
#
# place where the samples and programs can be found.
setenv LOFARWORKDIR $HOME/RadioDemo
setenv WAVEDIR $LOFARWORKDIR/WAVE
setenv MACHINE1 astron5

#remote start recording

rsh $MACHINE1 $LOFARWORKDIR/record_sample.csh antenna1 >& a1.out &



