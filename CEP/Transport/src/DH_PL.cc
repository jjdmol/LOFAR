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


#include <Transport/DH_PL.h>		        // for class definition
#include <Transport/PO_DH_PL.h>
#include <Transport/TH_PL.h>                    // for class definition
#include <Common/Exception.h>
#include <sstream>				// for ostringstream

namespace LOFAR {


DH_PL::DH_PL (const string& name, const string& type, int version)
  : DataHolder (name, type, version),
    itsPODHPL  (0)
{}

DH_PL::DH_PL (const DH_PL& that)
  : DataHolder (that),
    itsPODHPL  (0)
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
} 

PL::PersistentObject& DH_PL::getPO() const
{
  return *itsPODHPL;
}

int DH_PL::queryDB (const string& queryString)
{
  TH_PL* ptr = dynamic_cast<TH_PL*>(getTransporter().getTransportHolder());
  ASSERT (ptr != 0);
  int result;
  result = ptr->queryDB (queryString, getTransporter().getReadTag());
  handleDataRead();
  return result;
}

void DH_PL::insertDB()
{
  writeExtra();
  TH_PL* ptr = dynamic_cast<TH_PL*>(getTransporter().getTransportHolder());
  ASSERT (ptr != 0);
  ptr->insertDB (getTransporter().getWriteTag());
}

void DH_PL::updateDB()
{
  writeExtra();
  TH_PL* ptr = dynamic_cast<TH_PL*>(getTransporter().getTransportHolder());
  ASSERT (ptr != 0);
  ptr->updateDB (getTransporter().getWriteTag());
}

} // end namespace

