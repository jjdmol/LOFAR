#!/bin/csh
#
# 03-09-2001  AXC
#
# script to run LofarRadioSample demo
#
# place where the samples and programs can be found.
setenv LOFARWORKDIR $HOME/ICT_KENNIS_DEMO/RadioCapture
setenv WAVEDIR $LOFARWORKDIR/WAVE

if (! -d $LOFARWORKDIR) then
    echo "Can not find workdirectory: " $LOFARWORKDIR
    exit(1)
endif

if (! -d WAVEDIR) then
   mkdir $WAVEDIR >& /dev/null
endif

# clean Workdir/wave dir from all files
#rm -rf $WAVEDIR/*

# start java script
setenv DISPLAY lofar2:0.0
java -DWORKDIR=$LOFARWORKDIR lofarRadioSampler >& lofar.out &
#java -DWORKDIR=$LOFARWORKDIR makePlot >& plot.out &
