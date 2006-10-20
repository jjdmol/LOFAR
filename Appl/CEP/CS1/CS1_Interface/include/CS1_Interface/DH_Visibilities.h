//# DH_Visibilities.h: Visibilities DataHolder
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

#ifndef LOFAR_CS1_INTERFACE_DH_VISIBILITIES_H
#define LOFAR_CS1_INTERFACE_DH_VISIBILITIES_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <CS1_Interface/CS1_Config.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{
  namespace CS1
  {

    class DH_Visibilities: public DataHolder
    {
    public:
      typedef fcomplex	 VisibilityType;
      typedef unsigned short NrValidSamplesType;

      // Constructor with centerFreq being the center frequency of the subband
      explicit DH_Visibilities(const string& name,
                               const ACC::APS::ParameterSet &pSet);

      DH_Visibilities(const DH_Visibilities&);

      virtual ~DH_Visibilities();

      DataHolder* clone() const;

      /// Allocate the buffers.
      virtual void init();

      static int baseline(int station1, int station2)
      {
        DBGASSERT(station1 <= station2);
        return station2 * (station2 + 1) / 2 + station1;
      }

#if 0
      fcomplex (*getChannels(int station1, int station2)) [NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]
      {
        return &(*itsVisibilities)[baseline(station1, station2)];
      }
#endif

#if defined BGL_PROCESSING
      typedef VisibilityType AllVisibilitiesType[NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];
      typedef NrValidSamplesType AllNrValidSamplesType[NR_BASELINES][NR_SUBBAND_CHANNELS];

      AllVisibilitiesType* getVisibilities()
      {
        return (AllVisibilitiesType *) itsVisibilities;
      }

      const AllVisibilitiesType* getVisibilities() const
      {
        return (const AllVisibilitiesType *) itsVisibilities;
      }

      AllNrValidSamplesType *getNrValidSamples()
      {
        return (AllNrValidSamplesType *) itsNrValidSamples;
      }

      const AllNrValidSamplesType *getNrValidSamples() const
      {
        return (const AllNrValidSamplesType *) itsNrValidSamples;
      }
#endif

      VisibilityType &getVisibility(unsigned baseline, unsigned channel, unsigned pol1, unsigned pol2)
      {
        return itsVisibilities[NR_POLARIZATIONS * (NR_POLARIZATIONS * (itsNrChannels * baseline + channel) + pol1) + pol2];
      }

      NrValidSamplesType &getNrValidSamples(unsigned baseline, unsigned channel)
      {
        return itsNrValidSamples[itsNrChannels * baseline + channel];
      }

      const size_t getNrVisibilities() const
      {
        return itsNrBaselines * itsNrChannels * NR_POLARIZATIONS * NR_POLARIZATIONS;
      }

    private:
      /// Forbid assignment.
      DH_Visibilities& operator= (const DH_Visibilities&);

      unsigned	     itsNrBaselines, itsNrChannels;

      VisibilityType     *itsVisibilities;
      NrValidSamplesType *itsNrValidSamples;

      void fillDataPointers();
    };

  } // namespace CS1

} // namespace LOFAR

#endif 
