// CalibratorOld.cc: Wrapper for MeqCallibratorImpl class for BB
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
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableParse.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include "CalibratorOld.h"

namespace LOFAR
{

// RMME: const int DefaultAntennaCount = 21;

InitDebugContext(CalibratorOld,"CalibratorOld");

CalibratorOld::CalibratorOld (const string & MSName, const string & MEPName, 
    const string & GSMName, const string & DBType, const string & DBName, 
    const string & DBHost, const string & DBPasswd)
{
  // Initialize default values for the PSS3 Calibrater object.
  // These values are committed to itsMeqCalImpl when the CalibratorOld
  // ::Initialize () is called.

  itsMSName              = MSName;		// MS  (glish table)
  itsMEPName             = MEPName;		// MEQ (PL table)
  itsGSMName             = GSMName;		// GSM (PL table)
  itsDBType              = DBType;		// Database type
  itsDBName              = DBName;		// Database account
  itsDBHost              = DBHost;              // Database host
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
}


CalibratorOld::~CalibratorOld () {
  if (itsPSS3CalibratorImpl != NULL) {
    delete itsPSS3CalibratorImpl;
  }
}


void CalibratorOld::setTimeInterval (double secs) {
  itsTimeInterval = secs;
}


void CalibratorOld::Initialize (void) {
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
    cerr << "CalibratorOld::Initialize (); WARNING: You have possibly forgotten "
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
					     itsDBHost, 
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


void CalibratorOld::ShowSettings (void) {
    cout << "CalibratorOLD settings:" << endl;
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


void CalibratorOld::showCurrentParms (void) {
  vector <double> vals;
  getParmValues (itsSolvableParms, vals);
  char str [20];
  char str2 [20];

  vector<string> :: iterator i;
  for (i = itsSolvableParms.begin (); i != itsSolvableParms.end (); ++ i) {
    strcpy (str2, i -> c_str ());
    str2[5] = 0;
    sprintf (str, "%4s ", str2);
    cout << str;
  }
  cout << endl;

  vector<double> :: iterator j;
  for (j = vals.begin (); j != vals.end (); ++ j) {
    sprintf (str, "%1.3f ", * j);
    cout << str;
  }
  cout << endl;

}

void CalibratorOld::clearSolvableParms (void) {
  itsSolvableParms.erase (itsSolvableParms.begin (), itsSolvableParms.end ());

  TRACERF2 ("MeqCalImpl -> clearSolvableParms ()");
  itsPSS3CalibratorImpl -> clearSolvableParms ();
}

void CalibratorOld::addSolvableParm (string parmName, int srcNo) {
  ostringstream parm;
  parm << parmName << ".CP" << srcNo;

  itsSolvableParms.push_back (parm.str ());
}


void CalibratorOld::addSolvableParm (string parmNames) {
  itsSolvableParms.push_back (parmNames);
}


void CalibratorOld::commitSolvableParms (void) {
  // Create AIPS data structures to hold params
  Vector <String> pp (itsSolvableParms.size ());
  Vector <String> ep (0);

  vector<string> :: iterator i;
  int idx = 0;

  for (i = itsSolvableParms.begin (); i != itsSolvableParms.end (); ++ i) {
    ostringstream oss;
    oss << *i;
    pp [idx] = oss.str ();
    idx ++;
  }

  TRACERF2 ("MeqCalImpl -> setSolvableParms ()");  
  itsPSS3CalibratorImpl -> setSolvableParms (pp, ep, true);
}


void CalibratorOld::resetTimeIntervalIterator (void) {
  TRACERF2 ("MeqCalImpl -> resetIterator ()");
  itsPSS3CalibratorImpl -> resetIterator();
}


bool CalibratorOld::advanceTimeIntervalIterator (bool callReadPolcs) {
  TRACERF2 ("MeqCalImpl -> nextInterval ()");
  return itsPSS3CalibratorImpl -> nextInterval (callReadPolcs);
}


void CalibratorOld::clearPeelSources (void) {
  itsPeelSources.erase (itsPeelSources.begin (), itsPeelSources.end ());
}


void CalibratorOld::clearPeelMasks (void) {
  itsPeelMasks.erase (itsPeelMasks.begin (), itsPeelMasks.end ());
}


void CalibratorOld::addPeelSource (int srcNo) {
  itsPeelSources.push_back (srcNo - 1);
  // The peel sources list is 0-based, in contrast to the
  // rest of the inputs for MeqCalImpl.
}

void CalibratorOld::addPeelMask (int srcNo) {
  itsPeelMasks.push_back (srcNo - 1);
  // The peel mask list is 0-based, in contrast to the
  // rest of the inputs for MeqCalImpl.
}


void CalibratorOld::commitPeelSourcesAndMasks (void) {
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


void CalibratorOld::Run (void) {
  itsPSS3CalibratorImpl -> solve (true);
}

void CalibratorOld::Run (vector<string>& resultParmNames,
			 vector<double>& resultParmValues,
			 Quality& resultQuality) {
  itsPSS3CalibratorImpl -> solve (false, resultParmNames, resultParmValues, 
                                  resultQuality);
}

void CalibratorOld::SubtractOptimizedSources (void) {
  itsPSS3CalibratorImpl -> saveResidualData ();
}

void CalibratorOld::CommitOptimizedParameters (void) {
  itsPSS3CalibratorImpl -> saveParms ();
}

void CalibratorOld::commitAllSolvableParameters (void) {
  itsPSS3CalibratorImpl -> saveAllSolvableParms ();
}

void CalibratorOld::getParmValues (vector<string>& names, 
				vector<double>& values) {
  itsPSS3CalibratorImpl -> getParmValues (names, values);
}


void CalibratorOld::setParmValues (const vector<string>& names, 
				const vector<double>& values) {
  itsPSS3CalibratorImpl -> setParmValues (names, values);
}

} // end namespace LOFAR





























