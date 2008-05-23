//#  InputSection.h: Catch RSP ethernet frames and synchronize RSP inputs 
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

#ifndef LOFAR_CS1_INPUTSECTION_INPUTSECTION_H
#define LOFAR_CS1_INPUTSECTION_INPUTSECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/ION_to_CN.h>
#include <Transport/TransportHolder.h>
#include <BeamletBuffer.h>
#include <WH_DelayCompensation.h>
#include <InputThread.h>

#include <boost/multi_array.hpp>
#include <pthread.h>


namespace LOFAR {
namespace CS1 {

// This class is the workholder that receives data from the RSP boards
// and distributes it per subband to the Blue Gene/L
class InputSection {
  public:
    InputSection(const std::vector<TransportHolder *> &);
    ~InputSection();
  
    void preprocess(const CS1_Parset *ps);
    void process();
    void postprocess();
    
  private:
    void limitFlagsLength(SparseSet<unsigned> &flags);

    bool itsDelayCompensation, itsIsSynchronous;
    std::vector<int32>  itsBeamlet2beams;
    std::vector<uint32> itsSubband2Index;

    ION_to_CN itsIONtoCNdata;

    // writer thread
    InputThread *itsInputThread;

    TransportHolder *itsInputTH;
    const std::vector<TransportHolder *> &itsClientTHs;
    int32    itsPartitionIndex;
    unsigned itsStationNr;
    
    const CS1_Parset *itsCS1PS;
    
    // synced stamp
    TimeStamp itsSyncedStamp;
   
    double   itsSampleDuration;
    std::vector<double> itsDelaysAtBegin;
    std::vector<double> itsDelaysAfterEnd;
    std::vector<double> itsNrCalcDelaysForEachTimeNrDirections;
    //std::vector<double> itsNrCalcIntTimes;
    unsigned            itsNrCalcDelays;
    unsigned            itsCounter;
    
    unsigned itsMaxNetworkDelay;
    unsigned itsNSubbandsPerPset;
    unsigned itsNSamplesPerSec;
    unsigned itsNHistorySamples;

    unsigned itsCurrentComputeCore, itsNrCoresPerPset;
    unsigned itsPsetNumber;
   
    BeamletBuffer        *itsBBuffer;
    WH_DelayCompensation *itsDelayComp;
    double	          itsSampleRate;
    
    NSTimer itsDelayTimer;
    
    void startThread();
};
    
} // namespace CS1
} // namespace LOFAR

#endif
