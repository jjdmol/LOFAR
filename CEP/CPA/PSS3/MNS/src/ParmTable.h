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

#if !defined(MNS_PARMTABLE_H)
#define MNS_PARMTABLE_H

//# Includes
#include <aips/Tables/Table.h>
#include <aips/Tables/ColumnsIndex.h>
#include <aips/Containers/RecordField.h>
#include <MNS/MeqPolc.h>
#include <MNS/MeqSourceList.h>
#include <Common/lofar_vector.h>

//# Forward Declarations
class MeqDomain;
template<class T> class Vector;


class ParmTable
{
public:
  explicit ParmTable (const string& tableName);

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  vector<MeqPolc> getPolcs (const string& parmName,
			    int sourceNr, int station,
			    const MeqDomain& domain);

  // Get the initial polynomial coefficients for the given parameter.
  MeqPolc getInitCoeff (const string& parmName,
			int sourceNr, int station);

  // Put the polynomial coefficient for the given parameter and domain.
  void putCoeff (const string& parmName,
		 int sourceNr, int station,
		 const MeqPolc& polc);

  // Return point sources for the given source numbers.
  // An empty sourceNr vector means all sources.
  MeqSourceList getPointSources (const Vector<int>& sourceNrs);

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
};


#endif
