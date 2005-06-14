//# DH_PL.cc: Database persistent DH_Database
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

#include <TransportPL/DH_PL.h>		        // for class definition
#include <TransportPL/PO_DH_PL.h>
#include <TransportPL/TH_PL.h>                  // for class definition
#include <Common/LofarLogger.h>                 // for ASSERT macro
#include <Transport/Connection.h>
#include <Common/Exception.h>
#include <sstream>				// for ostringstream

namespace LOFAR {


DH_PL::DH_PL (const string& name, const string& type, int version)
  : DataHolder (name, type, version),
    itsPODHPL  (0),
    itsPOInitialized (false)
{}

DH_PL::DH_PL (const DH_PL& that)
  : DataHolder (that),
    itsPODHPL  (0),
    itsPOInitialized (false)
{}

DH_PL::~DH_PL ()
{
  delete itsPODHPL;
}

DataHolder* DH_PL::clone() const
{
  return new DH_PL (*this);
}

void DH_PL::initPO (const string& tableName)
{
  itsPODHPL = new PO_DH_PL(*this);
  itsPODHPL->tableName (tableName);
  setPOInitialized();
} 

PL::PersistentObject& DH_PL::getPO() const
{
  return *itsPODHPL;
}

int DH_PL::queryDB (const string& queryString, Connection& conn)
{
  TH_PL* ptr = dynamic_cast<TH_PL*>(conn.getTransportHolder());
  ASSERT (ptr != 0);
  int result;
  result = ptr->queryDB (queryString, conn.getTag(), this);
  unpack();
  return result;
}

void DH_PL::insertDB(Connection& conn)
{
  pack();
  TH_PL* ptr = dynamic_cast<TH_PL*>(conn.getTransportHolder());
  ASSERT (ptr != 0);
  ptr->insertDB (conn.getTag(), this);
}

void DH_PL::updateDB(Connection& conn)
{
  pack();
  TH_PL* ptr = dynamic_cast<TH_PL*>(conn.getTransportHolder());
  ASSERT (ptr != 0);
  ptr->updateDB (conn.getTag(), this);
}

} // end namespace

