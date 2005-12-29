//# TH_DB.h: Abstract TransportHolder base class for database TransportHolders
//#
//# Copyright (C) 2000, 2002
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

#ifndef TRANSPORTPOSTGRES_TH_DB_H
#define TRANSPORTPOSTGRES_TH_DB_H

#include <Transport/TransportHolder.h>
#include <Common/LofarTypes.h>                  // for int64


namespace LOFAR
{

class TH_DB: public TransportHolder
{
public:
  explicit TH_DB ();
  virtual ~TH_DB ();

  // Special functions to deal with database records in a special way.
  // Execute query and store result in dh
  virtual int queryDB (const string& queryString, DataHolder* dh) = 0;  
  // Execute SQL statement and store first field of first record in pResult. 
  // Return 0 if no result is found.
  virtual int queryDB (const string& queryString, 
		       char* pResult, int maxResultSize) = 0;  
  // Execute SQL statement
  virtual void executeSQL (const string& sqlStatement) = 0;  
  // Convert DataHolder blob to string to be used in SQL statement.
  virtual void addDBBlob(DataHolder* dh, ostringstream& str) = 0;

private:
  // Methods to manage the connection to the dbms
   // <group> 
  //  void connectDatabase();
  //  void disconnectDatabase();
  // </group> 

 
  int64  itsWriteSeqNo;
  int64  itsReadSeqNo;

  bool itsInitCalled; // Flag to indicate if the init method has been called
                      // This is used in the counting of initialized instances.
};

} // end namespace

#endif
