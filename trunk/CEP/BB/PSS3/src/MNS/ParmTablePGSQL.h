//# ParmTablePGSQL.h: Object to hold parameters in a postgres database table.
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

#if !defined(MNS_PARMTABLEPGSQL_H)
#define MNS_PARMTABLEPGSQL_H

//# Includes
#include <MNS/ParmTable.h>
#include <MNS/MeqParmHolder.h>
#include <MNS/MeqPolc.h>
#include <Common/lofar_vector.h>

#include <lofar_config.h>

#include <libpq-fe.h>

typedef vector<MeqParmHolder> VMParm;

//# Forward Declarations
class MeqDomain;
template<class T> class Vector;

using namespace std;

class ParmTablePGSQL : public ParmTableRep
{
public:
  // Create the ParmTable object.
  // The dbType argument gives the database type.
  // It can be postgres.
  ParmTablePGSQL (const string& hostName, const string& userName, const string& tableName);

  virtual ~ParmTablePGSQL();

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

  PGconn* itsDBConnection;

  VMParm find (const string& parmName, 
		 const MeqDomain& domain);

  string itsTableName;



  // functions to make reading from the database easier:
  inline MeqMatrix getMeqMatrix (PGresult* queryResult, int row, int column);
  inline double getDouble (PGresult* queryResult, int row, int column);
  inline bool getBool (PGresult* queryResult, int row, int column);
  inline string getString (PGresult* queryResult, int row, int column);
  inline int getInt (PGresult* queryResult, int row, int column);
  inline MeqDomain getDomain (PGresult* queryResult, int row, int column);

  // create the queries for update and insert
  inline string getUpdateQuery(MeqParmHolder MPH);
  inline string getInsertQuery(MeqParmHolder MPH);
  inline string getDefInsertQuery(MeqParmHolder MPH);

  // the first function returns the columns in the query 
  // the second function analyses the result and puts it in an object
  inline string getPolcNoDomainColumns();
  inline MeqPolc readPolcNoDomainQRes (PGresult* queryResult, int row, int column);

  inline string getDomainColumns();
  inline MeqDomain readDomainQRes (PGresult* queryResult, int row, int column);
  
  inline string getMeqParmNoPolcColumns();
  inline MeqParmHolder readMeqParmNoPolcQRes (PGresult* queryResult, int row, int column);

  // function to convert a MeqMatrix to a char*
  inline string MeqMat2string(const MeqMatrix &MM);
  
  // functions to remove special characters
  inline string encode(char* b, int length);
  inline int decode(char* c, char* b, int length);
};

#endif
