#!/usr/bin/env python

# Script that adds imaging columns to a MS
#
# File:        addImagingColumns.py
# Author:      Sven Duscha (duscha@astron.nl)
# Date:        2011-02-14
# Last change: 2011-10-29

import sys
try:
  import pyrap.tables as pt
except ImportError:
  print "addImagingColumns.py: could not import pyrap.tables"
  print "WARN: No imaging columns added"
  sys.exit(1)

if len(sys.argv)> 2:
  print "addImagingColumns.py: Too many arguments"
  sys.exit(1)
elif len(sys.argv)==1:
  print "addImagingColumns.py: No MS given"
  sys.exit(1)
else:
  filename=sys.argv[1]                # MS filename is by default first sys.argv
  pt.addImagingColumns(filename)      # Add imaging columns
  sys.exit(0)