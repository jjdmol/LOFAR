//# ParmTableMySQL.h: Object to hold parameters in a database table.
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

#if !defined(MNS_PARMTABLEMYSQL_H)
#define MNS_PARMTABLEMYSQL_H

//# Includes
#include <lofar_config.h>
#include <MNS/ParmTable.h>
#include <MNS/ParmTableFiller.h>
#include <MNS/MeqParmHolder.h>
#include <MNS/MeqPolc.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <mysql/mysql.h>

template<class T> class Vector;

namespace LOFAR {

//# Forward Declarations
class MeqDomain;

class ParmTableMySQL : public ParmTableRep, public ParmTableFiller
{
public:
  // Create the ParmTable object.
  // The dbType argument gives the database type.
  // It can be postgres.
  ParmTableMySQL (const string& hostName, const string& userName,
		  const string& tableName);

  virtual ~ParmTableMySQL();

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

  virtual void putNewCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc);
  virtual void putNewDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc);

  // Get the names of all sources in the table.
  virtual vector<string> getSources();

  // Unlock the underlying table.
  virtual void unlock();

private:

  MYSQL itsDB;

  vector<MeqParmHolder> find (const string& parmName, 
		 const MeqDomain& domain);

  string itsTableName;
};

}

#endif
