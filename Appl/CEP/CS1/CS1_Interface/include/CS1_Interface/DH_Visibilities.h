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
#include <CS1_Interface/CS1_Parset.h>

#if defined HAVE_BOOST
#include <boost/multi_array.hpp>
#endif

namespace LOFAR {
namespace CS1 {

class DH_Visibilities: public DataHolder
{
  public:
    typedef fcomplex	   VisibilityType;
    typedef unsigned short NrValidSamplesType;

    explicit DH_Visibilities(const string& name,
			     const CS1_Parset *pSet);

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

#if defined HAVE_BOOST
    typedef boost::multi_array_ref<VisibilityType, 4>	  VisibilitiesType;
    typedef boost::multi_array_ref<NrValidSamplesType, 2> AllNrValidSamplesType;

    VisibilitiesType getVisibilities() const
    {
      static boost::detail::multi_array::extent_gen<4u> extents = boost::extents[itsNrBaselines][itsNrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS];
      return VisibilitiesType(itsVisibilities, extents);
    }

    AllNrValidSamplesType getNrValidSamples() const
    {
      static boost::detail::multi_array::extent_gen<2u> extents = boost::extents[itsNrBaselines][itsNrChannels];
      return AllNrValidSamplesType(itsNrValidSamples, extents);
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

    DH_Visibilities &operator += (const DH_Visibilities &);

  private:
    /// Forbid assignment.
    DH_Visibilities& operator= (const DH_Visibilities&);

    const CS1_Parset  *itsCS1PS;
    unsigned	       itsNrBaselines, itsNrChannels;

    VisibilityType     *itsVisibilities;
    NrValidSamplesType *itsNrValidSamples;

    void fillDataPointers();
};

} // namespace CS1
} // namespace LOFAR

#endif 
