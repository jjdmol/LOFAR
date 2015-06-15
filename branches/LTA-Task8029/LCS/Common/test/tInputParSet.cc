//# tInputParSet.cc: Test program for class InutParSet
//#
//# Copyright (C) 2011
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
#include <Common/InputParSet.h>
#include <Common/StreamUtil.h>

using namespace LOFAR;

void testInput (int argc, char* argv[])
{
  InputParSet inputs;
  // define the input structure
  inputs.setVersion("2012Jun11-GvD");
  inputs.create ("ms", "",
                 "Name of input MeasurementSet",
                 "string");
  inputs.create ("image", "",
                 "Name of output image file (default is <msname-stokes-mode-nchan>.img)",
                 "string");
  inputs.create ("nscales", "5",
                 "Scales for MultiScale Clean",
                 "int");
  inputs.create ("noise", "1.0",
                 "Noise (in Jy) for briggsabs weighting",
                 "float");
  inputs.create ("nonoise", "",
                 "Noise (in Jy) for briggsabs weighting",
                 "float");
  inputs.create ("spwid", "0",
                 "spectral window id(s) to be used",
                 "int vector");
  inputs.create ("fixed", "False",
                 "Keep clean model fixed",
                 "bool");
  inputs.create ("uservector", "1.1, 3.4",
                 "user-defined scales for MultiScale clean",
                 "float vector");

  // Fill the input structure from the command line.
  inputs.readArguments (argc, argv);

  // Get the input specification.
  bool fixed   = inputs.getBool("fixed");
  int nscales  = inputs.getInt("nscales");
  double noise = inputs.getDouble("noise");
  double nnoise= inputs.getDouble("nonoise");
  string ms    = inputs.getString("ms");
  string image = inputs.getString("image");
  vector<int> spwid(inputs.getIntVector("spwid"));
  vector<double> uservector(inputs.getDoubleVector("uservector"));

  cout << "fixed=" << fixed << endl;
  cout << "nscales=" << nscales << endl;
  cout << "noise=" << noise << endl;
  cout << "nonoise=" << nnoise << endl;
  cout << "ms=" << ms << endl;
  cout << "image=" << image << endl;
  cout << "spwid=" << spwid << endl;
  cout << "uservector=" << uservector << endl;
}

int main (int argc, char* argv[])
{
  try {
    // The file tInputParSet.in contains more than 1 parameter, so the
    // tInputParSet.run in casacore style has more than 1 parameter. 
#ifndef HAVE_AIPSPP
    if (argc > 2) {
      return 0;
    }
#endif
    testInput (argc, argv);
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
