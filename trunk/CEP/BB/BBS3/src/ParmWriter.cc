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
ParmWriter::ParmWriter()
{}

ParmWriter::~ParmWriter()
{}

void ParmWriter::write(vector<ParmData>& pData, double fStart, double fEnd,
		       double tStart, double tEnd)
{
  MeqDomain domain(fStart, fEnd, tStart, tEnd);

  // Store all parameters in their own ParmTable

  MeqParmGroup pgroup;
  for (uint i=0; i<pData.size(); ++i) {
    cout << "Writing parm " << pData[i].getName() << " into "
	 << pData[i].getTableName() << ' ' << pData[i].getDBName()
	 << " (" << pData[i].getDBType()
	 << ") values=" << pData[i].getValues() << endl;
    ParmTable ptab(pData[i].getDBType(), pData[i].getTableName(),
		   pData[i].getDBName(), "");
    MeqStoredParmPolc parm(pData[i].getName(), &pgroup, &ptab);
    parm.readPolcs (domain);
    parm.update (pData[i].getValues());
    parm.save();

//     streamsize prec = cout.precision();
//     cout.precision(10);
//     cout << "****Contr write: " << parm.getCoeffValues().getDouble()
// 	 << " for parameter " << pData[i].getName() << endl;
//     cout.precision (prec);

    ptab.unlock();
  }
}

} // namespace LOFAR
