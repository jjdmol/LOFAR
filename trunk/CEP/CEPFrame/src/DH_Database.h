//# DH_Database.h: Database based DataHolder definition
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


#ifndef CEPFRAME_DH_DATABASE_H
#define CEPFRAME_DH_DATABASE_H

#include <CEPFrame/DataHolder.h>

/// The DH_Database class is a base class for all DataHolders using a
/// (relational) database as a transport medium. In order to maintain
/// indepenance of database, the StoreInDatabase and RetrieveFromDatabase
/// methods are defined. A subclass of DH_Database, specific for a certain
/// database implementation (like Postgresql or MySQL) must override these
/// methods.

namespace LOFAR
{
  class DH_Database : public DataHolder
  {
  public:
    explicit DH_Database (const string& name, const string& type);
  
    /// A subclass of DH_Database must override the StoreIndatabase ()
    /// method and store the content of the arguments in the intended
    /// database. The StoreInDatabase () of DH_Database prints a warning
    /// message that this method should be overridden.
    virtual bool StoreInDatabase (int appId, int tag, char * buf, int size);

    /// A subclass of DH_Database must override the RetrieveIndatabase ()
    /// method and retrieve the content of the arguments from the
    /// database. The RetrieveInDatabase () of DH_Database prints a warning
    /// message that this method should be overridden.
    virtual bool RetrieveFromDatabase 
      (int appId, int tag, char * buf, int size);
  
  protected:
    class DataPacket:
      public DataHolder::DataPacket
    {
    public:
      DataPacket () {}
    };
  };

}
#endif
