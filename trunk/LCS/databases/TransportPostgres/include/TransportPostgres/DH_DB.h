//# DH_DB.h: Abstract base class for DataHolders storable in a database
//#
//# Copyright (C) 2004
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

#ifndef TRANSPORTPOSTGRES_DH_DB_H
#define TRANSPORTPOSTGRES_DH_DB_H

#include <Transport/DataHolder.h>		// for super-class definition
#include <Common/LofarTypes.h>			// for int64


namespace LOFAR {

class Connection;
class TH_DB;

// If a programmer wants it to make possible that a DataHolder can be stored
// in a database, the particular DH class should be derived from DH_DB.

class DH_DB : public DataHolder
{
public:
  // Create the DH_DB object. 
  explicit DH_DB (const string& name="DH_DBbasic", 
		  const string& type="DH_DB",
		  int version=0);

  virtual ~DH_DB ();

  // Special functions to deal with database records in a special way.
   // Otherwise they throw an exception.
  // <group>

  // Read from the database with the given query string.
  // It returns the number of matching records. Only the first matching one
  // is really retrieved.
  int queryDB (const string& query, Connection& conn);
  // Insert the current record in the database.
  void insertDB(Connection& conn);
  // Update the current record in the database.
  void updateDB(Connection& conn);
  // <group>

protected:
  // Methods to obtain the specific queries to insert/update this DataHolder 
  virtual string createInsertStatement(TH_DB* th);
  virtual string createUpdateStatement(TH_DB* th);

  // Copy constructor.
  DH_DB (const DH_DB&);

private:
  // Forbid assignment.
  DH_DB& operator= (const DH_DB&);

};

}// namespace LOFAR

#endif
