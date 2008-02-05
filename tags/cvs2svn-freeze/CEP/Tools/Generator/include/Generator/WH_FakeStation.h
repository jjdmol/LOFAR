//#  WH_FakeStation.h: Emulate a station
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_GENERATOR_WH_FAKESTATION_H
#define LOFAR_GENERATOR_WH_FAKESTATION_H

// \file
// Emulate a station

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <Generator/RSPEthFrame.h>
#include <Generator/Flaggers.h>

namespace LOFAR 
{
  namespace Generator 
  {

    // \addtogroup Generator
    // @{

    using ACC::APS::ParameterSet;

    class WH_FakeStation: public WorkHolder
    {
    public:

      explicit WH_FakeStation(const string& name, 
			      const ParameterSet ps,
			      const int StationID,
			      const int delay,
			      TransportHolder* th);
      virtual ~WH_FakeStation();
    
      static WorkHolder* construct(const string& name, 
				   const ParameterSet ps,
				   const int StationID,
				   const int delay,
				   TransportHolder* th);
      virtual WH_FakeStation* make(const string& name);

      virtual void preprocess();
      virtual void process();

    private:
      /// forbid copy constructor
      WH_FakeStation (const WH_FakeStation&);
      /// forbid assignment
      WH_FakeStation& operator= (const WH_FakeStation&);
      
      ParameterSet itsPS;
      int itsStationId;
      int itsDelay;
      TransportHolder* itsTH;
      Flagger* itsFlagger;
    };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
