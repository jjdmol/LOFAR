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

//# Includes
#include <PSS3/MNS/ParmTable.h>
#include <PSS3/MNS/MeqPolc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Containers/RecordField.h>


namespace LOFAR {

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
				    int sourceNr, int station,
				    const MeqDomain& domain);

  // Get the initial polynomial coefficients for the given parameter.
  virtual MeqPolc getInitCoeff (const string& parmName,
				int sourceNr, int station);

  // Put the polynomial coefficient for the given parameter and domain.
  virtual void putCoeff (const string& parmName,
			 int sourceNr, int station,
			 const MeqPolc& polc);

  // Put the default coefficients
  virtual void putDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc);

  // Insert new coefficients
  void putNewCoeff( const string& parmName, 
		    int srcnr,
		    int statnr,
		    const MeqPolc& polc);

  // Insert new default coefficients
  void putNewDefCoeff( const string& parmName, 
		       int srcnr,
		       int statnr,
		       const MeqPolc& polc);

  // Get the names of all sources in the table.
  virtual vector<string> getSources();

  // Unlock the underlying table.
  virtual void unlock();

  // Connect to the database
  virtual void connect();
  // Create the database or table
  static void createTable(const string& userName, const string& tableName);
  // clear database or table
  virtual void clearTable();

private:
  // Find the table subset containing the parameter values for the
  // requested domain.
  Table find (const string& parmName, 
	      int sourceNr, int station,
	      const MeqDomain& domain);

  Table                  itsTable;
  ColumnsIndex           itsIndex;
  RecordFieldPtr<Int>    itsIndexSrcnr;
  RecordFieldPtr<Int>    itsIndexStatnr;
  RecordFieldPtr<String> itsIndexName;
  Table                  itsInitTable;
  ColumnsIndex*          itsInitIndex;
  RecordFieldPtr<Int>    itsInitIndexSrcnr;
  RecordFieldPtr<Int>    itsInitIndexStatnr;
  RecordFieldPtr<String> itsInitIndexName;

  string itsTableName;
};

}

#endif
