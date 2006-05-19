//# DH_Subband.h: DataHolder for subband samples and flags
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

#ifndef LOFAR_CS1_INTERFACE_DH_SUBBAND_H
#define LOFAR_CS1_INTERFACE_DH_SUBBAND_H

#define SPARSE_FLAGS

#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/RectMatrix.h>
#if defined SPARSE_FLAGS
#include <CS1_Interface/SparseSet.h>
#else
#include <CS1_Interface/bitset.h>
#endif
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <APS/ParameterSet.h>

namespace LOFAR {
namespace CS1 {

class DH_Subband: public DataHolder
{
  public:
    // samples are ALWAYS stored in little endian format !

    typedef INPUT_SAMPLE_TYPE SampleType;

    // Fine-grained delays
    typedef struct {
      float delayAtBegin, delayAfterEnd;
    } DelayIntervalType;

    explicit DH_Subband(const string &name,
			const ACC::APS::ParameterSet &pSet); 

    DH_Subband(const DH_Subband &);

    virtual ~DH_Subband();

    DataHolder *clone() const;

    virtual void init();

    RectMatrix<SampleType> &getSamplesMatrix() const
    {
      return *itsSamplesMatrix;
    }

    SampleType &getSample(unsigned station, unsigned time, unsigned pol)
    {
      return itsSamples[NR_POLARIZATIONS * (itsNrInputSamples * station + time) + pol];
    }

    size_t nrSamples() const
    {
      return itsNrStations * itsNrInputSamples * NR_POLARIZATIONS;
    }

    DelayIntervalType &getDelay(unsigned station)
    {
      return itsDelays[station];
    }

#if defined SPARSE_FLAGS
    SparseSet &getFlags(unsigned station)
    {
      return itsFlags[station];
    }

    const SparseSet &getFlags(unsigned station) const
    {
      return itsFlags[station];
    }
#else
    size_t nrFlags() const
    {
      return itsNrStations * ((itsNrInputSamples + 31) & ~31);
    }
#endif

    size_t nrDelays() const
    {
      return itsNrStations;
    }

#if defined BGL_PROCESSING
    // Samples
    typedef SampleType AllSamplesType[NR_STATIONS][NR_INPUT_SAMPLES][NR_POLARIZATIONS];

    // Fine-grained delays
    typedef DelayIntervalType AllDelaysType[NR_STATIONS];

    AllSamplesType *getSamples()
    {
      return (AllSamplesType *) itsSamples;
    }

    const AllSamplesType *getSamples() const
    {
      return (const AllSamplesType *) itsSamples;
    }

    AllDelaysType *getDelays()
    {
      return (AllDelaysType *) itsDelays;
    }

    const AllDelaysType *getDelays() const
    {
      return (const AllDelaysType *) itsDelays;
    }
#endif

    void swapBytes();

#if defined SPARSE_FLAGS
    void		   getExtraData(), fillExtraData();
#endif

  private:
    /// Forbid assignment.
    DH_Subband &operator = (const DH_Subband &);

    unsigned		   itsNrStations, itsNrInputSamples;

    SampleType		   *itsSamples;
    // RectMatrix cannot be used for bitsets, thus not for flags
    RectMatrix<SampleType> *itsSamplesMatrix;
#if defined SPARSE_FLAGS
    SparseSet		   *itsFlags;
#else
    uint32		   *itsFlags;
#endif
    DelayIntervalType	   *itsDelays;

    void fillDataPointers();
};

} // namespace CS1
} // namespace LOFAR

#endif 
