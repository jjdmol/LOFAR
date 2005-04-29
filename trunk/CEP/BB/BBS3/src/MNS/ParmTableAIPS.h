//# ParmTableAIPS.h: Object to hold parameters in an AIPS++ table.
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

#if !defined(MNS_PARMTABLEAIPS_H)
#define MNS_PARMTABLEAIPS_H

// \file MNS/ParmTableAIPS.h
// Object to hold parameters in an AIPS++ table.

//# Includes
#include <BBS3/MNS/ParmTable.h>
#include <BBS3/MNS/MeqPolc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Containers/RecordField.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

class ParmTableAIPS : public ParmTableRep
{
public:
  // Create the ParmTableAIPS object.
  // The extension .MEP is added to the table name.
  explicit ParmTableAIPS (const string& userName, const string& tableName);

  virtual ~ParmTableAIPS();

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  virtual vector<MeqPolc> getPolcs (const string& parmName,
				    const MeqDomain& domain);

  // Get the initial polynomial coefficients for the given parameter.
  virtual MeqPolc getInitCoeff (const string& parmName);

  // Put the polynomial coefficient for the given parameter and domain.
  // If written in a new row, the polc ID is set to the rownr.
  virtual void putCoeff (const string& parmName, MeqPolc& polc);

  // Insert new coefficients.
  // The polc ID is set to the rownr.
  virtual void putNewCoeff (const string& parmName, MeqPolc& polc);

  // Put the default coefficients
  virtual void putDefCoeff (const string& parmName, MeqPolc& polc);

  // Insert new default coefficients
  virtual void putNewDefCoeff (const string& parmName, MeqPolc& polc);

  // Get the names of all sources in the table.
  virtual vector<string> getSources();

  // Unlock the underlying table.
  virtual void unlock();

  // Connect to the database.
  virtual void connect();
  // Create the database or table.
  static void createTable (const string& userName, const string& tableName);
  // Clear database or table.
  virtual void clearTable();

private:
  // Put a polc. Check if it is in a new row or not.
  void putCoeffCheck (const string& parmName, MeqPolc& polc);

  // Find the table subset containing the parameter values for the
  // requested domain.
  casa::Table find (const string& parmName, 
		    const MeqDomain& domain);

  casa::Table                  itsTable;
  casa::ColumnsIndex           itsIndex;
  casa::RecordFieldPtr<casa::String> itsIndexName;
  casa::Table                  itsInitTable;
  casa::ColumnsIndex*          itsInitIndex;
  casa::RecordFieldPtr<casa::String> itsInitIndexName;

  string itsTableName;

};

// @}

}

#endif
