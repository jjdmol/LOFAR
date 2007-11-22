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

#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/SparseSet.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <CS1_Interface/CS1_Parset.h>

#if defined HAVE_BOOST
#include <boost/multi_array.hpp>
#endif

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
			const CS1_Parset *pSet); 

    DH_Subband(const DH_Subband &);

    virtual ~DH_Subband();

    DataHolder *clone() const;

    virtual void init();

    SampleType &getSample(unsigned station, unsigned time, unsigned pol)
    {
      return itsSamples[NR_POLARIZATIONS * (itsNrInputSamples * station + time) + pol];
    }

    size_t nrSamples() const
    {
      return itsNrStations * itsNrInputSamples * NR_POLARIZATIONS;
    }

    size_t nrInputSamples() const
    {
      return itsNrInputSamples;
    }

    DelayIntervalType &getDelay(unsigned station)
    {
      return itsDelays[station];
    }

    SparseSet<unsigned> &getFlags(unsigned station)
    {
      return itsFlags[station];
    }

    const SparseSet<unsigned> &getFlags(unsigned station) const
    {
      return itsFlags[station];
    }

    size_t nrDelays() const
    {
      return itsNrStations;
    }

#if defined HAVE_BOOST
    typedef boost::multi_array_ref<SampleType, 3>	   Samples3Dtype;
    typedef boost::multi_array_ref<SampleType, 4>	   Samples4Dtype;
    typedef boost::multi_array_ref<DelayIntervalType, 1>   DelaysType;
    typedef boost::multi_array_ref<SparseSet<unsigned>, 1> FlagsType;

    Samples3Dtype getSamples3D() const
    {
      static boost::detail::multi_array::extent_gen<3u> extents = boost::extents[itsNrStations][itsNrInputSamples][NR_POLARIZATIONS];
      return Samples3Dtype(itsSamples, extents);
    }

    Samples4Dtype getSamples4D() const
    {
      static boost::detail::multi_array::extent_gen<4u> extents = boost::extents[itsNrStations][itsNrInputSamples / NR_SUBBAND_CHANNELS][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS];
      return Samples4Dtype(itsSamples, extents);
    }

    DelaysType getDelays() const
    {
      static boost::detail::multi_array::extent_gen<1u> extents = boost::extents[itsNrStations];
      return DelaysType(itsDelays, extents);
    }

    FlagsType getFlags()
    {
      static boost::detail::multi_array::extent_gen<1u> extents = boost::extents[itsNrStations];
      return FlagsType(itsFlags, extents);
    }
#endif

    void		   swapBytes();
    void		   getExtraData(), fillExtraData();

  private:
    /// Forbid assignment.
    DH_Subband &operator = (const DH_Subband &);

    const CS1_Parset       *itsCS1PS;
    unsigned		   itsNrStations, itsNrInputSamples;

    SampleType		   *itsSamples;
    SparseSet<unsigned>	   *itsFlags;
    DelayIntervalType	   *itsDelays;

    void fillDataPointers();
};

} // namespace CS1
} // namespace LOFAR

#endif 
