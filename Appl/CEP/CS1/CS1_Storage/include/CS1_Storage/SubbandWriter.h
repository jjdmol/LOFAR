//#  SubbandWriter.h: Write subband(s) in an AIPS++ Measurement Set
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

#ifndef CS1_STORAGE_SUBBANDWRITER_H
#define CS1_STORAGE_SUBBANDWRITER_H

// \file
// Write subband(s) in an AIPS++ Measurement Set

#include <Blob/KeyValueMap.h>
#include <CS1_Interface/CS1_Config.h>
#ifdef USE_MAC_PI
# include <GCF/PALlight/CEPPropertySet.h>
# include <GCF/GCF_PVDynArr.h>
#endif
#include <Common/Timer.h>
#include <Common/lofar_vector.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/CorrelatedData.h>
#include <CS1_Interface/Queue.h>
#include <Stream/Stream.h>

#include <pthread.h>


namespace LOFAR {
namespace CS1 {

class MSWriter;

class SubbandWriter
{
  public:
			    SubbandWriter(const CS1_Parset *, unsigned rank);
			    ~SubbandWriter();

    void		    preprocess();
    void		    process();
    void		    postprocess();

  private:
    void		    clearAllSums();
    void		    createInputStreams();
    void		    writeLogMessage();
    bool		    processSubband(unsigned sb);

    void		    createInputThread();
    void		    stopInputThread();
    static void		    *inputThreadStub(void *);
    void		    inputThread();

  
    const CS1_Parset	    *itsCS1PS;
    unsigned		    itsRank;

    std::vector<Stream *>   itsInputStreams;

    static const unsigned   nrInputBuffers = 10;
    Queue<CorrelatedData *> itsFreeQueue, itsReceiveQueue;
    std::vector<Arena *>    itsArenas;
    pthread_t		    itsInputThread;

    unsigned		    itsNStations;
    unsigned		    itsNBaselines;
    unsigned		    itsNChannels;
    unsigned		    itsNBeams;
    unsigned		    itsNPolSquared;
    unsigned		    itsNVisibilities;

    vector<MSWriter *>	    itsWriters;

    unsigned		    itsNrSubbandsPerPset;
    unsigned		    itsNrSubbandsPerStorage;

    vector<unsigned>	    itsBandIDs;
    unsigned		    itsTimeCounter;
    unsigned		    itsTimesToIntegrate;
    bool		    *itsFlagsBuffers;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];
    float		    *itsWeightsBuffers;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS];
    fcomplex		    *itsVisibilities;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];

    float		    itsWeightFactor;

    NSTimer		    itsWriteTimer;

#ifdef USE_MAC_PI
    bool itsWriteToMAC;
    GCF::CEPPMLlight::CEPPropertySet* itsPropertySet;
    GCF::Common::GCFPValueArray itsVArray; 
#endif
};

} // namespace CS1
} // namespace LOFAR

#endif
