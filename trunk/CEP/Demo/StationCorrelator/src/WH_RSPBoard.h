//#  WH_RSPBoard.h: Emulate an RSP board
//#
//#  Copyright (C) 2002-2005
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

#ifndef STATIONCORRELATOR_WH_RSPBOARD_H
#define STATIONCORRELATOR_WH_RSPBOARD_H


#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#include <SyncStamp.h>

namespace LOFAR
{
  class WH_RSPBoard: public WorkHolder
  {
  public:

    explicit WH_RSPBoard(const string& name, 
			 const KeyValueMap kvm);
    virtual ~WH_RSPBoard();
    
    static WorkHolder* construct(const string& name, 
                                 const KeyValueMap kvm);
    virtual WH_RSPBoard* make(const string& name);

    virtual void process();

  private:
    /// forbid copy constructor
    WH_RSPBoard (const WH_RSPBoard&);
    /// forbid assignment
    WH_RSPBoard& operator= (const WH_RSPBoard&);

    KeyValueMap itsKVM;

    SyncStamp itsStamp;
  };

} // namespace LOFAR

#endif
