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

//# Forward Declarations
class MeqDomain;
class MeqMatrix;


class ParmTable
{
public:
  ParmTable (const string& tableName);

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  MeqMatrix getValues (bool& matchDomain,
		       const string& parmName, const MeqDomain& domain);

  // Put the parameter values for the given parameter and domain.
  void putValues (const string& parmName,
		  const MeqDomain& domain,
		  const MeqMatrix& values);

  // Get the initial parameter values for the given parameter.
  MeqMatrix getInitValues (const string& parmName);

private:
  // Find the table subset containing the parameter values for the
  // requested domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  Table find (bool& matchDomain,
	      const string& parmName, const MeqDomain& domain);

  Table                  itsTable;
  ColumnsIndex           itsNameIndex;
  RecordFieldPtr<String> itsIndexField;
  Table                  itsInitTable;
  ColumnsIndex*          itsInitNameIndex;
  RecordFieldPtr<String> itsInitIndexField;
};


#endif
