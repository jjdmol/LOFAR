//# DH_Database.h: Definition Database DataHolder
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

namespace LOFAR
{

class DH_Database : public DataHolder
{
public:
  explicit DH_Database (const string& name, const string& type)
    : DataHolder (name, type) { rdSeqNo = wrSeqNo = 0L; }

  virtual bool StoreInDatabase (int appId, int tag, char * buf, int size);
  virtual bool RetrieveFromDatabase (int appId, int tag, char * buf, int size);

protected:
  class DataPacket:
    public DataHolder::DataPacket
  {
  public:
    DataPacket () {}
  };

private:
  unsigned long rdSeqNo, wrSeqNo;

};

}
#endif
