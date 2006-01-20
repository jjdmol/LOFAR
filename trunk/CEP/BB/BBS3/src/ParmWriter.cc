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
#include <BBS3/ParmWriter.h>
#include <BBS3/ParmData.h>
#include <MNS/MeqPolc.h>
#include <MNS/MeqDomain.h>
#include <MNS/ParmTable.h>
#include <Common/LofarLogger.h>

#include <BBS3/MNS/MeqStoredParmPolc.h>

using namespace std;

namespace LOFAR
{
ParmWriter::ParmWriter(const int DBMasterPort)
  : itsDBMasterPort(DBMasterPort)
{}

ParmWriter::~ParmWriter()
{}

void ParmWriter::write(vector<ParmData>& pData, double fStart, double fEnd,
		       double tStart, double tEnd)
{
  MeqDomain domain(fStart, fEnd, tStart, tEnd);

  // Store all parameters in their own ParmTable
  // Use one ParmTable at the time to avoid reopening the same table.
  vector<char> done(pData.size(), 0); 
  MeqParmGroup pgroup;
  uint nrDone = 0;
  string lastName;
  ParmDB::ParmDB* dbptr = 0;
  while (nrDone < pData.size()) {
    for (uint i=0; i<pData.size(); ++i) {
      if (done[i] == 0) {
	if (dbptr == 0) {
	  lastName = pData[i].getParmDBMeta().getTableName();
	  dbptr = new ParmDB::ParmDB(pData[i].getParmDBMeta());
	}
	if (pData[i].getParmDBMeta().getTableName() == lastName) {
//     cout << "Writing parm " << pData[i].getName() 
//  	 << " values=" << pData[i].getValues() << endl;
	  MeqStoredParmPolc parm(pData[i].getName(), &pgroup, dbptr);
	  parm.readPolcs (domain);
	  parm.update (pData[i].getValues());
	  parm.save();
	  done[i] = 1;
	  nrDone++;
//     streamsize prec = cout.precision();
//     cout.precision(10);
//     cout << "****Contr write: " << parm.getCoeffValues().getDouble()
// 	 << " for parameter " << pData[i].getName() << endl;
//     cout.precision (prec);
	}
      }
    }
    delete dbptr;
    dbptr = 0;
  }
}

} // namespace LOFAR
