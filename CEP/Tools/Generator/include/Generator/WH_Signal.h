//#  WH_Signal.h: Create a signal
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

#ifndef LOFAR_GENERATOR_WH_SIGNAL_H
#define LOFAR_GENERATOR_WH_SIGNAL_H

// \file
// Create a signal

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <Generator/DH_RSP.h>
#include <Generator/Signals.h>

namespace LOFAR 
{
  namespace Generator 
  {

    // \addtogroup Generator
    // @{

    using ACC::APS::ParameterSet;

    class WH_Signal: public WorkHolder
    {
    public:
      
      explicit WH_Signal(const string& name, 
			 const int noOutputs,
			 const ParameterSet ps);
      virtual ~WH_Signal();
    
      static WorkHolder* construct(const string& name, 
				   const int noOutputs,
				   const ParameterSet ps);
      virtual WH_Signal* make(const string& name);

      virtual void preprocess();
      virtual void process();
      virtual void postprocess();

      //handle timer alarm
      static void timerSignal(int signal);    

    private:
      /// forbid copy constructor
      WH_Signal (const WH_Signal&);
      /// forbid assignment
      WH_Signal& operator= (const WH_Signal&);
      
      ParameterSet itsPS;
      double itsFrequency;
      TimeStamp itsStamp;
      Signal* itsSignal;
      
      static int theirNoRunningWHs;
      static int theirNoAlarms;
      static bool theirTimerSet;
    };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
