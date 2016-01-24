#! /usr/bin/env python
from lofar.sas.xmlgenerator.xmlgen import dms2deg, hms2deg
import sys

#testing for bug #8134 Negative zero degrees were turned into positive ones
test_dms = str(dms2deg("-00:30:00.0")) == "-0.5"
test_hms = str(hms2deg("-00:30:00.0")) == "-7.5"

if test_dms and test_hms:
  sys.exit(0)
else:
  sys.exit(1)
