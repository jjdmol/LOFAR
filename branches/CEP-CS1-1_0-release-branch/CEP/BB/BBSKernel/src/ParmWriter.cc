//#  ParmWriter.cc: writes/updates parameters in ParmTables
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSKernel/ParmWriter.h>
#include <BBSKernel/ParmData.h>
#include <BBSKernel/MNS/MeqDomain.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmValue.h>
#include <Common/LofarLogger.h>
#include <iomanip>


using namespace std;

namespace LOFAR
{
namespace BBS 
{

ParmWriter::ParmWriter()
{}

ParmWriter::~ParmWriter()
{}

void ParmWriter::write (const ParmDataInfo& pDataInfo,
			double fStart, double fEnd,
			double tStart, double tEnd)
{
  const vector<ParmData>& pData = pDataInfo.parms();
  LOFAR::ParmDB::ParmDomain pdomain(fStart, fEnd, tStart, tEnd);
  // Store all parameters, all domains in their own ParmTable
  // Use one ParmTable at the time to avoid reopening the same table.
  vector<char> done(pData.size(), 0);
  uint nrDone = 0;
  while (nrDone < pData.size()) {
    int lastDBnr = -1;
    vector<string> parmNames;
    vector<int>    parmIndex;
    parmNames.reserve (pData.size());
    parmIndex.reserve (pData.size());
    for (uint i=0; i<pData.size(); ++i) {
      if (done[i] == 0) {
	if (lastDBnr < 0) {
	  lastDBnr = pData[i].getParmDBSeqNr();
	}
	if (pData[i].getParmDBSeqNr() == lastDBnr) {
	  LOG_TRACE_FLOW_STR("Writing parm " << pData[i].getName() << endl);
	  parmNames.push_back (pData[i].getName());
	  parmIndex.push_back (i);
	  done[i] = 1;
	  nrDone++;
	}
      }
    }
    DBGASSERT (lastDBnr >= 0);
    // Get the ParmDB object for this ParmDB index.
    // It requires that Prediffer and Controller open the ParmDBs in
    // the same order.
    LOFAR::ParmDB::ParmDB pdb = LOFAR::ParmDB::ParmDB::getParmDB (lastDBnr);
    map<string,LOFAR::ParmDB::ParmValueSet> vals;
    pdb.getValues (vals, parmNames, pdomain);
    // Add entries for parms not present in the map.
    // They are clearly new parms.
    // Set the value for all parms.
    for (uint i=0; i<parmNames.size(); i++) {
      const ParmData& parmd = pData[parmIndex[i]];
      map<string,LOFAR::ParmDB::ParmValueSet>::iterator pos = vals.find(parmNames[i]);
      if (pos == vals.end()) {
	LOFAR::ParmDB::ParmValueSet pset(parmNames[i]);
	LOFAR::ParmDB::ParmValue pval = pdb.getDefValue (parmNames[i]);
	for (int j=0; j<parmd.size(); ++j) {
	  //// set correct domain
	  setCoeff (pval.rep(), parmd.getCoeff(j));
	  pset.getValues().push_back (pval);
	}
	vals.insert (make_pair(parmNames[i], pset));
      } else {
	ASSERT (uint(parmd.size()) == pos->second.getValues().size());
	for (int j=0; j<parmd.size(); ++j) {
	  setCoeff (pos->second.getValues()[j].rep(), parmd.getCoeff(j));
	}
      }
    }
    pdb.putValues (vals);
  }
}

void ParmWriter::setCoeff (LOFAR::ParmDB::ParmValueRep& pval, const MeqMatrix& coeff)
{
  ASSERT (int(pval.itsCoeff.size()) == coeff.nelements());
  const double* vals = coeff.doubleStorage();
  for (int i=0; i<coeff.nelements(); ++i) {
    pval.itsCoeff[i] = vals[i];
  }
}

} // namespace BBS
} // namespace LOFAR
