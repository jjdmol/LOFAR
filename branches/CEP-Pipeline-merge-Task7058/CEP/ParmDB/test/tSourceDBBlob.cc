//# tSourceDBBlob.cc: Test program for class tSourceDBBlob
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/SourceDB.h>
#include <Common/StreamUtil.h>
#include <Common/Exception.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;

int main()
{
  // It should recognize that the data is in blob format.
  SourceDB sdb(ParmDBMeta("", "tSourceDBBlob_tmp.src"), false);
  SourceData sdata;
  while (! sdb.atEnd()) {
    sdb.getNextSource (sdata);
    cout << endl;
    cout << "Source name:    " << sdata.getInfo().getName() << endl;
    cout << "Patch name:     " << sdata.getPatchName() << endl;
    cout << "Source type:    " << sdata.getInfo().getType() << endl;
    cout << "Reftype:        " << sdata.getInfo().getRefType() << endl;
    cout << "RA:             " << sdata.getRa() << endl;
    cout << "DEC:            " << sdata.getDec() << endl;
    cout << "I:              " << sdata.getI() << endl;
    cout << "Q:              " << sdata.getQ() << endl;
    cout << "U:              " << sdata.getU() << endl;
    cout << "V:              " << sdata.getV() << endl;
    cout << "Major axis:     " << sdata.getMajorAxis() << endl;
    cout << "Minor axis:     " << sdata.getMinorAxis() << endl;
    cout << "Orientation:    " << sdata.getOrientation() << endl;
    cout << "Spectral index: " << sdata.getInfo().getSpectralIndexNTerms()
         << "  " << sdata.getSpectralIndex() << endl;
    cout << "SpInx RefFreq:  " << sdata.getInfo().getSpectralIndexRefFreq() << endl;
    cout << "Use RM:         " << sdata.getInfo().getUseRotationMeasure() << endl;
    cout << "PolAngle:       " << sdata.getPolarizationAngle() << endl;
    cout << "PolFrac:        " << sdata.getPolarizedFraction() << endl;
    cout << "RM:             " << sdata.getRotationMeasure() << endl;
    cout << "Shapelet I:     " << sdata.getInfo().getShapeletCoeffI() << endl;
    cout << "Shapelet Q:     " << sdata.getInfo().getShapeletCoeffQ() << endl;
    cout << "Shapelet U:     " << sdata.getInfo().getShapeletCoeffU() << endl;
    cout << "Shapelet V:     " << sdata.getInfo().getShapeletCoeffV() << endl;
  }

  // Get all patch names and info.
  cout << "Patch info: " << sdb.getPatchInfo() << endl;
  cout << "Patch names: " << sdb.getPatches() << endl;
  cout << "Patch names: " << sdb.getPatches(-1, "cy*") << endl;
  cout << "Patch names: " << sdb.getPatches(1) << endl;
  cout << "Patch names: " << sdb.getPatches(2) << endl;
  cout << "Patch names: " << sdb.getPatches(-1, "", -1, 0.5) << endl;
  cout << "Patch names: " << sdb.getPatches(-1, "", 0.5, 2.25) << endl;
}
