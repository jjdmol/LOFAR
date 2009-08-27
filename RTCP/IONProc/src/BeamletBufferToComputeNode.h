//#  BeamletBufferToComputeNode.h: Catch RSP ethernet frames and synchronize RSP inputs 
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

#ifndef LOFAR_IONPROC_BEAMLET_BUFFER_TO_COMPUTE_NODE_H
#define LOFAR_IONPROC_BEAMLET_BUFFER_TO_COMPUTE_NODE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Interface/Parset.h>
#include <Interface/MultiDimArray.h>
#include <Interface/RSPTimeStamp.h>
#include <Stream/Stream.h>
#include <BeamletBuffer.h>
#include <WH_DelayCompensation.h>
#include <AMCBase/Direction.h>

#include <boost/multi_array.hpp>
#include <pthread.h>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class BeamletBufferToComputeNode {
  public:
    BeamletBufferToComputeNode(const std::vector<Stream *> &cnStreams, const std::vector<BeamletBuffer<SAMPLE_TYPE> *> &beamletBuffers, unsigned psetNumber);
    ~BeamletBufferToComputeNode();
  
    void			 preprocess(const Parset *ps);
    void			 process();
    void			 postprocess();
    
  private:
    static void			 limitFlagsLength(SparseSet<unsigned> &flags);

    void			 computeDelays();
    void			 startTransaction();
    void			 writeLogMessage() const;
    void			 toComputeNodes();
    void			 stopTransaction();

    void			 dumpRawData();
    Stream			 *itsRawDataStream;
    bool			 itsFileHeaderWritten;

    bool			 itsDelayCompensation;
    bool			 itsNeedDelays;
    bool			 itsIsRealTime;
    bool			 itsDumpRawData;
    std::vector<unsigned>	 itsSubbandToBeamMapping;
    std::vector<unsigned>	 itsSubbandToRSPboardMapping;
    std::vector<unsigned>	 itsSubbandToRSPslotMapping;

    const std::vector<Stream *>  &itsCNstreams;
    
    const Parset		 *itsPS;
    
    TimeStamp			 itsCurrentTimeStamp;
   
    Matrix<double>		 itsDelaysAtBegin;
    Matrix<double>		 itsDelaysAfterEnd;
    Matrix<AMC::Direction>	 itsBeamDirectionsAtBegin;
    Matrix<AMC::Direction>	 itsBeamDirectionsAfterEnd;
    unsigned			 itsNrOutputPsets;
    
    unsigned			 itsMaxNetworkDelay; // in samples
    unsigned                     itsNrSubbands;
    unsigned			 itsNrSubbandsPerPset;
    unsigned			 itsNrSamplesPerSec;
    unsigned			 itsNrHistorySamples;
    unsigned			 itsNrInputs;
    unsigned			 itsNrBeams;
    unsigned			 itsNrPencilBeams;

    unsigned			 itsCurrentComputeCore, itsNrCoresPerPset;
    unsigned			 itsPsetNumber;
   
    const std::vector<BeamletBuffer<SAMPLE_TYPE> *> &itsBeamletBuffers;
    WH_DelayCompensation	 *itsDelayComp;
    double			 itsSampleRate, itsSampleDuration;

    std::vector<TimeStamp>	 itsDelayedStamps;
    std::vector<signed int>	 itsSamplesDelay;
    boost::multi_array<SparseSet<unsigned>, 2> itsFlags;

    Matrix<float>		 itsFineDelaysAtBegin, itsFineDelaysAfterEnd;

    NSTimer			 itsDelayTimer;
};

} // namespace RTCP
} // namespace LOFAR

#endif
