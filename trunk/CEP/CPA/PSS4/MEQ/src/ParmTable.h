//# ParmTable.h: Object to hold parameters in a table.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef MEQ_PARMTABLE_H
#define MEQ_PARMTABLE_H

//# Includes
#include <aips/Tables/Table.h>
#include <aips/Tables/ColumnsIndex.h>
#include <aips/Containers/RecordField.h>
#include <MEQ/Polc.h>
//#include <MEQ/MeqSourceList.h>
#include <Common/lofar_vector.h>
#include <map>


namespace Meq {

//# Forward Declarations
class Domain;


class ParmTable
{
public:
  explicit ParmTable (const string& tableName);

  ~ParmTable();

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  vector<Polc> getPolcs (const string& parmName, const Domain& domain);

  // Get the initial polynomial coefficients for the given parameter.
  Polc getInitCoeff (const string& parmName);

  // Put the polynomial coefficient for the given parameter and domain.
  void putCoeff (const string& parmName, const Polc& polc);

  // Return point sources for the given source numbers.
  // An empty sourceNr vector means all sources.
  // In the 2nd version the pointers to the created Parm objects
  // are added to the vector of objects to be deleted.
  //  MeqSourceList getPointSources (const Vector<int>& sourceNrs);
  //  MeqSourceList getPointSources (const Vector<int>& sourceNrs,
  //				 vector<MeqExpr*>& exprDel);

  // Open the table if not opened yet. If opened, it is added to the map.
  static ParmTable* openTable (const String& tableName);
  // Close all tables in the map.
  static void closeTables();

  // Get the name of the ParmTable.
  const string& name() const
    { return itsTable.tableName(); }

  // Create a new table.
  static void createTable (const String& tableName);

private:
  // Find the table subset containing the parameter values for the
  // requested domain.
  Table find (const string& parmName, const Domain& domain);

  Table                  itsTable;
  ColumnsIndex           itsIndex;
  RecordFieldPtr<String> itsIndexName;
  Table                  itsInitTable;
  ColumnsIndex*          itsInitIndex;
  RecordFieldPtr<String> itsInitIndexName;

  static std::map<string, ParmTable*> theirTables;
};


} // namespace Meq

#endif
