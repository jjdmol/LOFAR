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
#include <aips/Tables/Table.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/TableParse.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/ArrayUtil.h>
#include <aips/Exceptions/Error.h>
#include "Calibrator.h"

#include <iostream>

// RMME: const int DefaultAntennaCount = 21;

InitDebugContext(Calibrator,"Calibrator");

Calibrator::Calibrator (const string & MSName, const string & MEPName, 
    const string & GSMName, const string & DBType, const string & DBName, 
    const string & DBPasswd)
{
  // Initialize default values for the PSS3 Calibrater object.
  // These values are committed to itsMeqCalImpl when the Calibrator
  // ::Initialize () is called.

  itsMSName              = MSName;		// MS  (glish table)
  itsMEPName             = MEPName;		// MEQ (PL table)
  itsGSMName             = GSMName;		// GSM (PL table)
  itsDBType              = DBType;		// Database type
  itsDBName              = DBName;		// Database account
  itsDBPasswd            = DBPasswd;
  itsModelType           = "LOFAR.RI";
  itsDDID                = 0; // TODO: GvD What is this?
  itsScenario            = "solve";
  itsSolvParms           = "{RA,DEC,StokesI}.*";
  itsStChan              = 0;
  itsEndChan             = 0;
  itsSelStr              = "all([ANTENNA1,ANTENNA2] in 4*[0:20])";
  itsCalcUVW             = false;
  itsTimeInterval        = 3600.0;
  itsDataColumn          = "CORRECTED_DATA";
  itsCorrectedDataColumn = "CORRECTED_DATA";

  itsPSS3CalibratorImpl  = NULL;

  std::cout << "***************" << itsMSName << "-" << itsMEPName << "-" << itsGSMName
	    << "-" << DBType << "=" << itsDBType << "-" << itsDBName << "-" << itsDBPasswd << endl;
}


Calibrator::~Calibrator () {
  if (itsPSS3CalibratorImpl != NULL) {
    delete itsPSS3CalibratorImpl;
  }
}


void Calibrator::setTimeInterval (double secs) {
  itsTimeInterval = secs;
}


void Calibrator::Initialize (void) {
  // Set up antennae
  Vector<Int> ant1, ant2;
  {
    Table tab;
    if (itsSelStr.empty()) {
	tab = Table(itsMSName+".MS");
    } else {
	tab = tableCommand("SELECT FROM " + itsMSName + 
			   ".MS WHERE " + itsSelStr);
    }
    Table sortab = tab.sort ("ANTENNA1", Sort::Ascending,
			       Sort::QuickSort | Sort::NoDuplicates);
    ant1 = ROScalarColumn<Int>(sortab, "ANTENNA1").getColumn();
    sortab = tab.sort ("ANTENNA2", Sort::Ascending,
			 Sort::QuickSort | Sort::NoDuplicates);
    ant2 = ROScalarColumn<Int>(sortab, "ANTENNA2").getColumn();
  }

  // Check if the database members have been initialized correctly.
  if (itsDBName == "tanaka") {
    cerr << "Calibrator::Initialize (); WARNING: You have possibly forgotten "
	 << "to initialize the database type, name and password. Using "
	 << "default values 'postgres', 'tanaka' and '' respectively. The "
	 << "program you are running may exhibit unpredictable behaviour "
	 << "this way." << endl;
  }

  // Instantiate PSS3 calibration engine

  if (itsPSS3CalibratorImpl != NULL)
    delete itsPSS3CalibratorImpl;

  itsPSS3CalibratorImpl = new MeqCalibrater (itsMSName+".MS", 
					     itsMEPName, 
					     itsGSMName, 
					     itsDBType, 
					     itsDBName, 
					     itsDBPasswd,
					     itsDDID, 
					     ant1, 
					     ant2, 
					     itsModelType, 
					     itsCalcUVW,
					     itsDataColumn, 
					     itsCorrectedDataColumn);


  if (itsStChan >= 0  || itsEndChan >= 0  ||  !itsSelStr.empty()) {
    if (itsStChan < 0) {
	itsStChan = 0;
    }
    if (itsEndChan < 0) {
	itsEndChan = itsPSS3CalibratorImpl -> getNrChan() - 1;
	cout << " endchan (modified): " << itsEndChan << endl;
    }
    itsPSS3CalibratorImpl -> select (itsSelStr, itsStChan, itsEndChan);
  }

  // Choose domain
  itsPSS3CalibratorImpl -> setTimeInterval (itsTimeInterval);

  // In the MeqCalImpl object, initialize the set of solvable parms to zero.
  itsPSS3CalibratorImpl -> clearSolvableParms ();
}


void Calibrator::ShowSettings (void) {
    cout << "Calibrator settings:" << endl;
    cout << "  msname:    " << itsMSName << endl;
    cout << "  mepname:   " << itsMEPName << endl;
    cout << "  gsmname:   " << itsGSMName << endl;
    cout << "  dbtype:    " << itsDBType << endl;
    cout << "  dbname:    " << itsDBName << endl;
    cout << "  modeltype: " << itsModelType << endl;
    cout << "  scenario:  " << itsScenario << endl;
    cout << "  solvparms: " << itsSolvParms << endl;
    cout << "  stchan:    " << itsStChan << endl;
    cout << "  endchan:   " << itsEndChan << endl;
    cout << "  selection: " << itsSelStr << endl;
    cout << "  calcuvw  : " << itsCalcUVW << endl;
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


void Calibrator::addSolvableParm (string parmNames) {
  itsSolvableParms.push_back (parmNames);
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
  itsPSS3CalibratorImpl -> solve (true);
}

void Calibrator::Run (vector<string>& resultParmNames,
                      vector<double>& resultParmValues,
                      Quality& resultQuality) {
  itsPSS3CalibratorImpl -> solve (false, resultParmNames, resultParmValues, 
                                  resultQuality);
}

void Calibrator::SubtractOptimizedSources (void) {
  itsPSS3CalibratorImpl -> saveResidualData ();
}

void Calibrator::CommitOptimizedParameters (void) {
  itsPSS3CalibratorImpl -> saveParms ();
}


void Calibrator::getParmValues (vector<string>& names, 
				vector<double>& values) {
  itsPSS3CalibratorImpl -> getParmValues (names, values);
}


void Calibrator::setParmValues (const vector<string>& names, 
				const vector<double>& values) {
  itsPSS3CalibratorImpl -> setParmValues (names, values);
}






























