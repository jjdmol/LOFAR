//#  WH_FakeStation.h: Emulate a station
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

#ifndef STATIONCORRELATOR_WH_FAKESTATION_H
#define STATIONCORRELATOR_WH_FAKESTATION_H

#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>

namespace LOFAR
{
  class WH_FakeStation: public WorkHolder
  {
  public:

    explicit WH_FakeStation(const string& name, 
			    const ParameterSet ps,
			    TransportHolder& th);
    virtual ~WH_FakeStation();
    
    static WorkHolder* construct(const string& name, 
                                 const ParameterSet ps,
				 TransportHolder& th);
    virtual WH_FakeStation* make(const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

    //handle timer alarm
    static void timerSignal(int signal);    

  private:
    /// forbid copy constructor
    WH_FakeStation (const WH_FakeStation&);
    /// forbid assignment
    WH_FakeStation& operator= (const WH_FakeStation&);

    ParameterSet itsPS;
    TransportHolder& itsTH;
    EthernetFrame itsEthFrame;

    static int theirNoRunningWHs;
    static bool theirTimerSet;
  };

} // namespace LOFAR

#endif
