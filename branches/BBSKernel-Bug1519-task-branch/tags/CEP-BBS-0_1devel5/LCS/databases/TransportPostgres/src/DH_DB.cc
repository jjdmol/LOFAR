//# DH_DB.cc: Abstract base class for DataHolders storable in a database
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <TransportPostgres/DH_DB.h>	        // for class definition
#include <Common/LofarLogger.h>                 // for ASSERT macro
#include <Transport/Connection.h>
#include <TransportPostgres/TH_DB.h>
//#include <Common/Exception.h>
//#include <sstream>				// for ostringstream

namespace LOFAR {


DH_DB::DH_DB (const string& name, const string& type, int version)
  : DataHolder (name, type, version)
{}

DH_DB::DH_DB (const DH_DB& that)
  : DataHolder (that)
{}

DH_DB::~DH_DB ()
{
}

int DH_DB::queryDB (const string& query, Connection& conn)
{
  TH_DB* thPtr = dynamic_cast<TH_DB*>(conn.getTransportHolder());
  ASSERT (thPtr != 0);
  int result;
  result = thPtr->queryDB (query, this);
  unpack();
  return result;
}

void DH_DB::insertDB(Connection& conn)
{
  pack();
  TH_DB* thPtr = dynamic_cast<TH_DB*>(conn.getTransportHolder());
  ASSERT (thPtr != 0);
  string sqlStat=createInsertStatement(thPtr);
  thPtr->executeSQL (sqlStat);
}

void DH_DB::updateDB(Connection& conn)
{
  pack();
  TH_DB* thPtr = dynamic_cast<TH_DB*>(conn.getTransportHolder());
  ASSERT (thPtr != 0);
  string sqlStat=createUpdateStatement(thPtr);
  thPtr->executeSQL (sqlStat);
}

string DH_DB::createInsertStatement(TH_DB*)
{
  LOG_WARN("getInsertStatement() not implemented by this DataHolder. Returning empty string");
  return "";
}

string DH_DB::createUpdateStatement(TH_DB*)
{
  LOG_WARN("getUpdateStatement() not implemented by this DataHolder. Returning empty string");
  return "";
}


} // end namespace

