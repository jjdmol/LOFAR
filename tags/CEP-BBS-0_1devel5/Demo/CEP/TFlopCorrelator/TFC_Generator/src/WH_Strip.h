//#  WH_Strip.h: Take a signal from a DH and send only the bare data over a TH
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

#ifndef TFC_GENERATOR_WH_STRIP_H
#define TFC_GENERATOR_WH_STRIP_H

#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <TFC_Generator/DH_RSP.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  class WH_Strip: public WorkHolder
  {
  public:

    explicit WH_Strip(const string& name, 
		      TransportHolder& th,
		      const ParameterSet ps);
    virtual ~WH_Strip();
    
    static WorkHolder* construct(const string& name, 
				 TransportHolder& th,
                                 const ParameterSet ps);
    virtual WH_Strip* make(const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

    //handle timer alarm
    static void timerSignal(int signal);    

  private:
    /// forbid copy constructor
    WH_Strip (const WH_Strip&);
    /// forbid assignment
    WH_Strip& operator= (const WH_Strip&);

    ParameterSet itsPS;
    double itsFrequency;
    TimeStamp itsStamp;

    static int theirNoRunningWHs;
    static int theirNoAlarms;
    static bool theirTimerSet;

    TransportHolder& itsTH;
  };

} // namespace LOFAR

#endif
