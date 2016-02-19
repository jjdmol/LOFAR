#!/usr/bin/env python
#
# Script that prints all BF-raw files of a h5 meta data file of
# a BF observation
#
# File:         bfexternalfiles.py
# Author:       Sven Duscha (duscha_at_astron.nl)
# Date:         2012-06-17
# Last change:  2012-06-17


import sys
import DAL


filename=sys.argv[1]
fh=DAL.BF_File(filename)


for sapNr in range(0, fh.nofSubArrayPointings().value):
  if fh.subArrayPointing(sapNr).exists():
    sap=fh.subArrayPointing(sapNr)
    for beamNr in range(0, sap.nofBeams().value):
      if sap.beam(beamNr).exists():
        beam=sap.beam(beamNr)
        for stokesNr in range(0, beam.nofStokes().value):
          if beam.stokes(stokesNr).exists():
            list=beam.stokes(stokesNr).externalFiles()
#          print list
            for i in range(0, list.size()):
              print list[i]
