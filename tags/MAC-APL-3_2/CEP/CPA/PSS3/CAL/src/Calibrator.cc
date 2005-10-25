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

#include <CAL/MeqCalibraterImpl.h>
#include <Common/lofar_iostream.h>
#include <Calibrator.h>


const int DefaultAntennaCount = 21;

InitDebugContext(Calibrator,"Calibrator");

Calibrator::Calibrator () {
  int i;

  // Initialize default values for the PSS3 Calibrater object.
  // These values are committed to itsMeqCalImpl when the Calibrator
  // ::Initialize () is called.

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

  TRACERF2 ("Calibrator constructed.");
}


Calibrator::~Calibrator () {
  if (itsPSS3CalibratorImpl != NULL) {
    delete itsPSS3CalibratorImpl;
  }

  TRACERF2 ("Calibrator destroyed.");
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


void Calibrator::clearSolvableParms (void) {
  itsSolvableParms.erase (itsSolvableParms.begin (), itsSolvableParms.end ());

  TRACERF2 ("MeqCalImpl -> clearSolvableParms ()");
  itsPSS3CalibratorImpl -> clearSolvableParms ();
}


void Calibrator::addSolvableParm (string parmName, int srcNo) {
  AssertStr (parmName == "StokesI" || parmName == "RA" || parmName == "DEC",
	     "parmName must be StokesI, RA or DEC.")

  ostringstream parm;
  parm << parmName << ".CP" << srcNo;

  itsSolvableParms.push_back (parm.str ());
}


void Calibrator::commitSolvableParms (void) {
  // Create AIPS data structures to hold params
  Vector <String> pp (itsSolvableParms.size ());
  Vector <String> ep (itsSolvableParms.size ());
  ostringstream oss [itsSolvableParms.size ()];

  vector<string> :: iterator i;
  int idx = 0;

  for (i = itsSolvableParms.begin (); i != itsSolvableParms.end (); ++ i) {
    oss [idx] << *i;
    pp [idx] = oss[idx].str ();
    TRACERF4 (idx << " -> " << *i);
    idx ++;
  }

  TRACERF2 ("MeqCalImpl -> setSolvableParms ()");  
  itsPSS3CalibratorImpl -> setSolvableParms (pp, ep, true);
}


void Calibrator::resetTimeIntervalIterator (void) {
  TRACERF2 ("MeqCalImpl -> resetIterator ()");
  itsPSS3CalibratorImpl -> resetIterator();
}


bool Calibrator::advanceTimeIntervalIterator (void) {
  TRACERF2 ("MeqCalImpl -> nextInterval ()");
  cout << "Next interval" << endl;
  return itsPSS3CalibratorImpl -> nextInterval ();
}


void Calibrator::clearPeelSources (void) {
  itsPeelSources.erase (itsPeelSources.begin (), itsPeelSources.end ());
}


void Calibrator::clearPeelMasks (void) {
  itsPeelMasks.erase (itsPeelMasks.begin (), itsPeelMasks.end ());
}


void Calibrator::addPeelSource (int srcNo) {
  itsPeelSources.push_back (srcNo - 1);
  // The peel sources list is 0-based, in contrast to the
  // rest of the inputs for MeqCalImpl.
}

void Calibrator::addPeelMask (int srcNo) {
  itsPeelMasks.push_back (srcNo - 1);
  // The peel mask list is 0-based, in contrast to the
  // rest of the inputs for MeqCalImpl.
}


void Calibrator::commitPeelSourcesAndMasks (void) {
  // Create AIPS data structures to hold params
  Vector <Int> srcList (itsPeelSources.size ());
  Vector <Int> ignoreList (itsPeelMasks.size ());

  vector<int> :: iterator i;
  int idx = 0;

  for (i = itsPeelSources.begin (); i != itsPeelSources.end (); ++ i) {
    srcList [idx] = *i;
    TRACERF4 ("source: " << idx << " -> " << *i);
    idx ++;
  }

  idx = 0;

  for (i = itsPeelMasks.begin (); i != itsPeelMasks.end (); ++ i) {
    ignoreList [idx] = *i;
    TRACERF4 ("ignore: " << idx << " -> " << *i);
    idx ++;
  }

  TRACERF2 ("MeqCalImpl -> peel ()");  
  itsPSS3CalibratorImpl -> peel (srcList, ignoreList);
}


void Calibrator::Run (void) {
  itsPSS3CalibratorImpl -> solve (false);
}

void Calibrator::SubtractOptimizedSources (void) {
  itsPSS3CalibratorImpl -> saveResidualData ();
}

void Calibrator::CommitOptimizedParameters (void) {
  itsPSS3CalibratorImpl -> saveParms ();
}

void Calibrator::ExamplePSS3Run (void) {

}


void Calibrator::ExperimentalOptimizeSource (int src) {
  cout << "Next iteration" << endl;
  /*
  Vector <Int> srcList (1);
  Vector <Int> ignoreList (0);
  srcList [0] = src - 1; // NB: GvD: 0-based!!
  cout << "mc.peel ()..." << endl;
  itsPSS3CalibratorImpl -> peel (srcList, ignoreList);
  */
  // Calculate the optimal value for parameter
  itsPSS3CalibratorImpl -> solve(false);

  // Subtract the found sources from MS:
  itsPSS3CalibratorImpl -> saveResidualData();

  // Save the data (?) and display output:
  itsPSS3CalibratorImpl -> saveParms();

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





























