#!/usr/bin/env python

# Script that adds imaging columns to a MS
#
# File:        addImagingColumns.py
# Author:      Sven Duscha (duscha@astron.nl)
# Date:        2011-02-14
# Last change: 2011-02-14

import sys
import pyrap.tables as pt


filename=sys.argv[1]                # MS filename is by default first sys.argv
pt.addImagingColumns(filename)      # Add imaging columns
