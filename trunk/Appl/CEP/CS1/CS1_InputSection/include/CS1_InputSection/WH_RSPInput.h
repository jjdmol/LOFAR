//#  WH_RSPInput.h: Catch RSP ethernet frames and synchronize RSP inputs 
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

#ifndef LOFAR_CS1_INPUTSECTION_WH_RSPINPUT_H
#define LOFAR_CS1_INPUTSECTION_WH_RSPINPUT_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <boost/thread.hpp>
#include <CS1_Interface/RSPTimeStamp.h>
#include <APS/ParameterSet.h>

namespace LOFAR 
{
  class NSTimer;
  class TransportHolder;

  namespace CS1_InputSection 
  {

    // \addtogroup CS1_InputSection
    // @{

    //# Forward Declarations
    class BeamletBuffer;
    class InputThread;

    // This class is the workholder that receives data from the RSP boards
    // and distributes it per subband to several other input nodes
    class WH_RSPInput: public WorkHolder {
    public:
      explicit WH_RSPInput(const string& name, 
                           ACC::APS::ParameterSet& ps,
                           TransportHolder& th,
                           const bool isSyncMaster);
      virtual ~WH_RSPInput();
    
      static WorkHolder* construct(const string& name, 
                                   ACC::APS::ParameterSet& ps,
                                   TransportHolder& th,
				   const bool isSyncMaster);
	
      virtual WH_RSPInput* make(const string& name);
     
      virtual void startThread();
    
      virtual void preprocess();

      virtual void process();
      
      virtual void postprocess();
      
      // Show the work holder on stdout.
      virtual void dump() const;

    private:
      // Copying is not allowed
      WH_RSPInput (const WH_RSPInput& that);
      WH_RSPInput& operator= (const WH_RSPInput& that);
      
      //# Datamembers
      // writer thread
      InputThread* itsInputThreadObject;
      boost::thread* itsInputThread;

      TransportHolder& itsTH;
      
      // ACC parameters interface
      ACC::APS::ParameterSet &itsPS;
      
      // Sync Master or slave
      bool itsSyncMaster;

      // detect First process loop
      bool itsFirstProcessLoop;

      // synced stamp
      TimeStamp itsSyncedStamp;
     
      int itsNRSPOutputs;
      int itsNSubbands;
      int itsNSamplesPerSec;
      int itsNSamplesToCopy;
      int itsNSamplesPerSec;
      int itsStationID;
     
      BeamletBuffer* itsBBuffer;

      vector<NSTimer*> itsTimers;
      NSTimer* itsPrePostTimer;
      NSTimer* itsProcessTimer;
      NSTimer* itsDelayTimer;
      NSTimer* itsGetElemTimer;
      
    };
    
    // @}

  } // namespace CS1_InputSection
} // namespace LOFAR

#endif
