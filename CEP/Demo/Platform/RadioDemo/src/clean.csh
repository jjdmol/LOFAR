#!/usr/bin/csh
#
# 03-09-2001  AXC
#
# script to start cleaning the wavedir
#
# place where the samples and programs can be found.
setenv LOFARWORKDIR $HOME/ICT_KENNIS_DEMO/RadioCapture
setenv WAVEDIR $LOFARWORKDIR/WAVE

rm -f $WAVEDIR/antenna1.ready
rm -f $WAVEDIR/antenna2.ready
