//#  WH_InputSection.h: Catch RSP ethernet frames and synchronize RSP inputs 
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

#ifndef LOFAR_CS1_INPUTSECTION_WH_INPUTSECTION_H
#define LOFAR_CS1_INPUTSECTION_WH_INPUTSECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/ION_to_CN.h>
#include <BeamletBuffer.h>
#include <InputThread.h>
#include <Common/Timer.h>

#include <boost/multi_array.hpp>
#include <pthread.h>


namespace LOFAR {
namespace CS1 {

// This class is the workholder that receives data from the RSP boards
// and distributes it per subband to the Blue Gene/L
class WH_InputSection: public WorkHolder {
  public:
    explicit WH_InputSection(const string &name, 
			     unsigned stationNumber,
			     CS1_Parset *ps,
			     TransportHolder *inputTH);
    virtual ~WH_InputSection();
  
    virtual WH_InputSection *make(const string &name);
   
    virtual void preprocess();
    virtual void process();
    virtual void postprocess();
      
  private:
    // Copying is not allowed
    WH_InputSection (const WH_InputSection &that);
    WH_InputSection& operator= (const WH_InputSection &that);

    void limitFlagsLength(SparseSet<unsigned> &flags);

    bool itsDelayCompensation;

    ION_to_CN itsIONtoCNdata;

    // writer thread
    InputThread *itsInputThread;

    TransportHolder *itsInputTH;
    unsigned itsStationNr;
    string   itsStationName;
    
    CS1_Parset *itsCS1PS;
    
    // synced stamp
    TimeStamp itsSyncedStamp;
   
    unsigned itsNSubbandsPerPset;
    unsigned itsNSamplesPerSec;
    unsigned itsNHistorySamples;

    unsigned itsCurrentComputeCore, itsNrCoresPerPset;
    unsigned itsPsetNumber;
   
    BeamletBuffer *itsBBuffer;
    
    NSTimer itsPrePostTimer, itsProcessTimer, itsGetElemTimer;
    
    void startThread();
};
    
} // namespace CS1
} // namespace LOFAR

#endif
