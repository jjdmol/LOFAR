//#  WH_SubbandWriter.h: Write subband(s) in an AIPS++ Measurement Set
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

// \file
// Write subband(s) in an AIPS++ Measurement Set

#include <Blob/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/DH_Visibilities.h>
#ifdef USE_MAC_PI
# include <GCF/PALlight/CEPPropertySet.h>
# include <GCF/GCF_PVDynArr.h>
#endif
#include <Common/Timer.h>
#include <Common/lofar_vector.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR
{
  namespace CS1
  {
    //# Forward declaration
    class MSWriter;

    class WH_SubbandWriter: public WorkHolder
    {
    public:

      WH_SubbandWriter(const string& name,  const vector<uint>& subbandIDs,
                             CS1_Parset *pset);

      virtual ~WH_SubbandWriter();
    
      static WorkHolder* construct(const string& name,  
                                   const vector<uint>& subbandIDs,
				         CS1_Parset *pset);

      virtual WH_SubbandWriter* make(const string& name);

      void preprocess();

      virtual void process();

      void postprocess();
    private:
      /// forbid copy constructor
      WH_SubbandWriter (const WH_SubbandWriter&);
      /// forbid assignment
      WH_SubbandWriter& operator= (const WH_SubbandWriter&);

      // clear the integration buffers
      void clearAllSums();
      
      CS1_Parset *itsCS1PS;
      const vector<uint> itsSubbandIDs;     ///< IDs of the subband(s)
      uint  itsNStations;
      uint  itsNBaselines;
      uint  itsNChannels;
      uint  itsNBeams;
      uint  itsNPolSquared;
      uint  itsNVisibilities;

      vector <MSWriter *> itsWriters;

      uint itsNrSubbandsPerCell; ///< Number of subbands per BG/L cell
      uint itsNrSubbandsPerStorage;
      uint itsNrInputChannels;
      uint itsNrSubbandsPerMS;

      vector<uint> itsCurrentInputs;
      vector<uint> itsBandIDs;   ///< MS IDs of the frequency bands
      vector<uint> itsFieldIDs;  ///< MS IDs of the field, i.e. the beam.
      uint itsTimeCounter;       ///< Counts the time
      uint itsTimesToIntegrate;  ///< Number of timeSteps to integrate
      bool *itsFlagsBuffers;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];
      float *itsWeightsBuffers;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS];
      DH_Visibilities::VisibilityType *itsVisibilities;//[NR_SUBBANDS][NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];

      float itsWeightFactor;

      NSTimer itsWriteTimer;

#ifdef USE_MAC_PI
      bool itsWriteToMAC;
      GCF::CEPPMLlight::CEPPropertySet* itsPropertySet;
      GCF::Common::GCFPValueArray itsVArray; 
#endif
    };

  } // namespace CS1

} // namespace LOFAR

#endif
