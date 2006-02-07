//#  WH_SubbandWriter.h: Writes one subband in a AIPS++ Measurement Set
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

#ifndef CS1_STORAGE_WH_SUBBANDWRITER_H
#define CS1_STORAGE_WH_SUBBANDWRITER_H

// (Optionally) Writes one subband in a AIPS++ Measurement Set

#include <Blob/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/CS1_Config.h>
#ifdef USE_MAC_PI
#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/GCF_PVDynArr.h>
#endif
#include <Common/Timer.h>

namespace LOFAR
{

class MSWriter;

class WH_SubbandWriter: public WorkHolder
  {
  public:

    explicit WH_SubbandWriter(const string& name,  int subbandID,
			      const ACC::APS::ParameterSet& pset);
    virtual ~WH_SubbandWriter();
    
    static WorkHolder* construct(const string& name,  int subbandID,
                                 const ACC::APS::ParameterSet& pset);
    virtual WH_SubbandWriter* make(const string& name);

    void preprocess();

    virtual void process();

    void postprocess();
  private:
    /// forbid copy constructor
    WH_SubbandWriter (const WH_SubbandWriter&);
    /// forbid assignment
    WH_SubbandWriter& operator= (const WH_SubbandWriter&);

    int  itsSubbandID; // ID of this subband
    const ACC::APS::ParameterSet itsPS;
    int  itsNStations;
    int  itsNBaselines;
    int  itsNInputsPerSubband;
    int  itsNChannels;
    int  itsNPolSquared;

    MSWriter* itsWriter;

    int itsBandId;       // MS ID of frequency band
    int itsFieldId;
    int itsTimeCounter;  // Counts the time
    bool itsFlagsBuffer[NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];
    float itsWeightsBuffer[NR_BASELINES][NR_SUBBAND_CHANNELS];

    float itsWeightFactor;

    NSTimer itsWriteTimer;

#ifdef USE_MAC_PI
    bool itsWriteToMAC;
    GCF::CEPPMLlight::CEPPropertySet* itsPropertySet;
    GCF::Common::GCFPValueArray itsVArray; 
#endif
  };
} // namespace LOFAR

#endif
