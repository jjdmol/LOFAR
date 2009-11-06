//#  KVLDefines.h:
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef KVLUTILS_H
#define KVLUTILS_H

#include <GCF/TM/GCF_Event.h>
#include <GCF/GCF_PVBlob.h>

namespace LOFAR 
{
 namespace GCF 
 {  
  namespace LogSys 
  {

/**
*/

class Value : public TM::GCFTransportable
{
  public:
    Value() : _pValue(0), _unpacked(false) {};

    virtual ~Value();

    unsigned int pack(char* buffer);
    unsigned int unpack(char* buffer);
    unsigned int getSize();

    const Common::GCFPValue* _pValue;

  private:
    bool _unpacked;
};

class EventCollection : public TM::GCFTransportable
{
  public:
    EventCollection() {};

    virtual ~EventCollection() {};

    unsigned int pack(char* buffer);
    unsigned int unpack(char* buffer);
    unsigned int getSize();
    
    Common::GCFPVBlob buf;
};

inline unsigned int EventCollection::pack(char* buffer)
{
  return buf.pack(buffer);
}

inline unsigned int EventCollection::getSize()
{
  return buf.getSize();
}
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR

#endif
