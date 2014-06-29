//#  -*- mode: c++ -*-
//#
//#  DataEvent.h: raw data event
//#
//#  Copyright (C) 2002-2004
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

#ifndef DATAEVENT_H_
#define DATAEVENT_H_

#include "MEPData.h"

class DataEvent : public GCFEvent
{
  public:
  DataEvent(GCFEvent& e) : GCFEvent(e) { unpack(); }

  DataEvent() : GCFEvent(0) {}

  virtual ~DataEvent()
    {
      if (_unpackDone)
      {
      }
    }
    
  MEPData payload;

  void *pack(unsigned int& packsize)
    {
      unsigned int requiredSize = payload.getSize();

      resizeBuf(requiredSize);
      unsigned int offset = 0;
  
      offset += payload.pack(_buffer + offset);
          
      packsize = offset;
      return _buffer;
    }

  private:
  DataEvent(DataEvent&);
  DataEvent& operator= (const DataEvent&);
    
  void unpack()
    {
      if (length > 0)
      {
	unsigned int offset = sizeof(GCFEvent);
	char* data = (char*) _base;
    
	offset += payload.unpack(data + offset);
    
      }
    }
};

#endif
