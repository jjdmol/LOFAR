// Calibrator.cc: Wrapper for MeqCallibratorImpl class for BB
//
// Copyright (C) 2003
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Id: 

#include <PSS3/MeqCalibraterImpl.h>
#include <Common/lofar_iostream.h>
#include <PSS3/Calibrator.h>

const int DefaultAntennaCount = 21;

Calibrator::Calibrator () {

  int i;

  cout << "Initializing Calibrator." << endl;

  // Initialize default values for the PSS3 Calibrater object.

  for (i = 0; i < DefaultAntennaCount; i ++) {
    itsPrimaryAntennae.push_back (4 * i);
    itsSecondaryAntennae.push_back (4 * i);
  }

  itsNrOfIterations = 10;

  itsTblMeasurementSet = "demo.MS";
  itsTblMeqModel = "demo";
  itsTblSkyModel = "demo_gsm";

  itsDDID = 0;
  itsModelType = "LOFAR.RI";

  itsCalcUVW = false;

  itsDataColumn = "CORRECTED_DATA";
  itsCorrectedDataColumn = "CORRECTED_DATA";

  itsTimeInterval = 3600.0;

  itsPSS3CalibratorImpl = NULL;
}


Calibrator::~Calibrator () {
  if (itsPSS3CalibratorImpl != NULL) {
    delete itsPSS3CalibratorImpl;
  }
}


void Calibrator::Initialize (void) {
  Vector<int> ant1 (itsPrimaryAntennae.size ());
  Vector<int> ant2 (itsSecondaryAntennae.size ());
  unsigned int i;

  for (i = 0; i < itsPrimaryAntennae.size (); i ++) 
    ant1 [i] = itsPrimaryAntennae [i];

  for (i = 0; i < itsSecondaryAntennae.size (); i ++) 
    ant2 [i] = itsSecondaryAntennae [i];

  itsPSS3CalibratorImpl = new MeqCalibrater (
					     itsTblMeasurementSet,
					     itsTblMeqModel,
					     itsTblSkyModel,
					     itsDDID,
					     ant1,
					     ant2,
					     itsModelType,
					     itsCalcUVW,
					     itsDataColumn,
					     itsCorrectedDataColumn
  );

  itsPSS3CalibratorImpl -> setTimeInterval (itsTimeInterval);
}


void Calibrator::OptimizeSource (int src, int nIterations) {

  Vector <String> pp (3);
  Vector <String> ep (3);
  ostringstream oss[3];

  oss[0] << "StokesI.CP" << src;        pp[0] = oss[0].str();
  oss[1] << "RA.CP"      << src;        pp[1] = oss[1].str();
  oss[2] << "DEC.CP"     << src;        pp[2] = oss[2].str();

  itsPSS3CalibratorImpl -> setSolvableParms (pp, ep, true);
    
  itsPSS3CalibratorImpl -> resetIterator();
  while (itsPSS3CalibratorImpl -> nextInterval ()) {
    cout << "Next interval" << endl;
    for (int i = 0; i < nIterations; i ++) {
      cout << "Next iteration" << endl;

      Vector <Int> srcList (1);
      Vector <Int> ignoreList (0);
      srcList [0] = src - 1; // NB: GvD: 0-based!!
      cout << "mc.peel ()..." << endl;
      itsPSS3CalibratorImpl -> peel (srcList, ignoreList);

      // Calculate the optimal value for parameter
      itsPSS3CalibratorImpl -> solve(false);

      // Subtract the found sources from MS:
      itsPSS3CalibratorImpl -> saveResidualData();

      // Save the data (?) and display output:
      itsPSS3CalibratorImpl -> saveParms();

      cerr << "(" << i << ") ";
    }
  }
}

void Calibrator::OptimizeSourceWOSaveRes (int src, int nIterations) {

  Vector <String> pp (3);
  Vector <String> ep (3);
  ostringstream oss[3];

  oss[0] << "StokesI.CP" << src;        pp[0] = oss[0].str();
  oss[1] << "RA.CP"      << src;        pp[1] = oss[1].str();
  oss[2] << "DEC.CP"     << src;        pp[2] = oss[2].str();

  itsPSS3CalibratorImpl -> setSolvableParms (pp, ep, true);
    
  itsPSS3CalibratorImpl -> resetIterator();
  while (itsPSS3CalibratorImpl -> nextInterval ()) {
    cout << "Next interval" << endl;
    for (int i = 0; i < nIterations; i ++) {
      cout << "Next iteration" << endl;

      Vector <Int> srcList (1);
      Vector <Int> ignoreList (0);
      srcList [0] = src - 1; // NB: GvD: 0-based!!
      cout << "mc.peel ()..." << endl;
      itsPSS3CalibratorImpl -> peel (srcList, ignoreList);

      // Calculate the optimal value for parameter
      itsPSS3CalibratorImpl -> solve(false);

      // No subtraction of  the found sources from MS:
      // OFF!  itsPSS3CalibratorImpl -> saveResidualData();

      // Save the data (?) and display output:
      itsPSS3CalibratorImpl -> saveParms();

      cerr << "[" << i << "] ";
    }
  }
}





























