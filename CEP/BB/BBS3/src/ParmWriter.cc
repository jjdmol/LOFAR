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

using namespace std;

namespace LOFAR
{
ParmWriter::ParmWriter()
{}

ParmWriter::~ParmWriter()
{
  ParmTableMap::iterator iter;
  for (iter=itsTables.begin(); iter!= itsTables.end(); iter++)
  {
    delete iter->second;
  }
  itsTables.clear();
}

void ParmWriter::useTable(const string& dbType, const string& tableName,
			  const string& dbName, const string& pwd,
			  const string& hostName)
{
  ParmTableMap::iterator iter;
  iter = itsTables.find(tableName);
  if (iter != itsTables.end())
  {
    LOG_WARN_STR("A table with name " << tableName << " and type "
		 << dbType << " is already in use by ParmWriter.");
    return;
  }
  // Create new ParmTable object and add to table map.
  itsTables[tableName] = new ParmTable(dbType, tableName, dbName, pwd, hostName);

}

void ParmWriter::write(vector<ParmData>& pData, double fStart, double fEnd,
		       double tStart, double tEnd)
{
  vector<ParmData>::iterator parmIter;
  ParmTableMap::iterator tableIter;
  MeqDomain domain(fStart, fEnd, tStart, tEnd);

  // Store all parameters in their own ParmTable
  for (parmIter = pData.begin(); parmIter != pData.end(); parmIter++)
  {
    // Look up table 
    tableIter = itsTables.find(parmIter->getTableName());
    DBGASSERTSTR(tableIter != itsTables.end(), "No table " 
		 << parmIter->getTableName() << " in use by ParmWriter.");
    DBGASSERTSTR(tableIter->second->getDBType() == parmIter->getDBType(),
		 "No table " << parmIter->getTableName() << " of type " 
		 << parmIter->getDBType() << " in use by ParmWriter. ");

    MeqPolc polc;
    polc.setCoeff(parmIter->getValues());
    polc.setDomain(domain);
    polc.setSimCoeff(parmIter->getValues());
    polc.setPertSimCoeff(parmIter->getValues());
    // Store
    tableIter->second->putCoeff(parmIter->getName(), polc);

//     streamsize prec = cout.precision();
//     cout.precision(10);
//     cout << "****Contr write: " << polc.getCoeff().getDouble()
// 	 << " for parameter " << parmIter->getName() << endl;
//     cout.precision (prec);

    tableIter->second->unlock();
  }
}

} // namespace LOFAR
