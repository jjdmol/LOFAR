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

#ifndef LOFAR_IONPROC_INPUTSECTION_H
#define LOFAR_IONPROC_INPUTSECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Interface/Parset.h>
#include <Interface/MultiDimArray.h>
#include <Interface/RSPTimeStamp.h>
#include <Stream/Stream.h>
#include <BeamletBuffer.h>
#include <WH_DelayCompensation.h>
#include <InputThread.h>
#include <LogThread.h>
#include <AMCBase/Direction.h>

#include <boost/multi_array.hpp>
#include <pthread.h>

#undef DUMP_RAW_DATA

namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class InputSection {
  public:
    InputSection(const std::vector<Stream *> &clientStreams, unsigned psetNumber);
    ~InputSection();
  
    void			 preprocess(const Parset *ps);
    void			 process();
    void			 postprocess();
    
  private:
    void			 startThreads();
    static void			 limitFlagsLength(SparseSet<unsigned> &flags);

    void			 computeDelays();
    void			 startTransaction();
    void			 writeLogMessage() const;
    void			 toComputeNodes();
    void			 stopTransaction();

#if defined DUMP_RAW_DATA
    void			 dumpRawData();
#endif

    bool			 itsDelayCompensation;
    bool			 itsNeedDelays;
    bool			 itsIsRealTime;
    std::vector<unsigned>	 itsSubbandToBeamMapping;
    std::vector<unsigned>	 itsSubbandToRSPboardMapping;
    std::vector<unsigned>	 itsSubbandToRSPslotMapping;

    std::vector<InputThread<SAMPLE_TYPE> *> itsInputThreads;

    std::vector<Stream *>	 itsInputStreams;
    const std::vector<Stream *>  &itsClientStreams;
    
    const Parset		 *itsPS;
    
    TimeStamp			 itsSyncedStamp;
   
    Matrix<double>		 itsDelaysAtBegin;
    Matrix<double>		 itsDelaysAfterEnd;
    Matrix<AMC::Direction>	 itsBeamDirectionsAtBegin;
    Matrix<AMC::Direction>	 itsBeamDirectionsAfterEnd;
    unsigned			 itsNrPsets;
    
    unsigned			 itsMaxNetworkDelay; // in samples
    unsigned                     itsNSubbands;
    unsigned			 itsNSubbandsPerPset;
    unsigned			 itsNSamplesPerSec;
    unsigned			 itsNHistorySamples;
    unsigned			 itsNrInputs;
    unsigned			 itsNrBeams;
    unsigned			 itsNrPencilBeams;

    unsigned			 itsCurrentComputeCore, itsNrCoresPerPset;
    unsigned			 itsPsetNumber;
   
    std::vector<BeamletBuffer<SAMPLE_TYPE> *> itsBBuffers;
    WH_DelayCompensation	 *itsDelayComp;
    double			 itsSampleRate, itsSampleDuration;

    std::vector<TimeStamp>	 itsDelayedStamps;
    std::vector<signed int>	 itsSamplesDelay;
    boost::multi_array<SparseSet<unsigned>, 2> itsFlags;

    Matrix<float>		 itsFineDelaysAtBegin, itsFineDelaysAfterEnd;

    
    LogThread			 *itsLogThread;
    NSTimer			 itsDelayTimer;
};

} // namespace RTCP
} // namespace LOFAR

#endif
