//# VisBuffer.h: A buffer of visibility data and associated information (e.g.
//# flags, UVW coordinates).
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSKERNEL_VISBUFFER_H
#define LOFAR_BBSKERNEL_VISBUFFER_H

// \file
// A buffer of visibility data and associated information (e.g. flags, UVW
// coordinates).

#include <Common/lofar_smartptr.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/Types.h>
#include <BBSKernel/VisDimensions.h>
#include <measures/Measures/MDirection.h>
#include <boost/multi_array.hpp>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class VisBuffer
{
public:
    typedef shared_ptr<VisBuffer>       Ptr;
    typedef shared_ptr<const VisBuffer> ConstPtr;

    VisBuffer(const VisDimensions &dims);
    VisBuffer(const VisDimensions &dims, const Instrument::ConstPtr &instrument,
        const casa::MDirection &phaseRef, double refFreq);

    // Access to the dimensions of this buffer.
    const VisDimensions &dimensions() const;

    void setInstrument(const Instrument::ConstPtr &instrument);
    Instrument::ConstPtr instrument() const;
    size_t nStations() const;

    void setPhaseReference(const casa::MDirection &reference);
    const casa::MDirection &getPhaseReference() const;

    void setReferenceFreq(double freq);
    double getReferenceFreq() const;

    // Convenience functions that delegate to VisDimensions (refer to the
    // documentation of VisDimensions for their documentation).
    // @{
    size_t nFreq() const;
    size_t nTime() const;
    size_t nBaselines() const;
    size_t nCorrelations() const;

    Box domain() const;
    const Grid &grid() const;
    const BaselineSeq &baselines() const;
    const CorrelationSeq &correlations() const;
    // @}

    size_t nSamples() const;

    bool hasUVW() const;

    // Computes station UVW coordinates in meters for the center of each time
    // interval in the buffer. Requires a valid instrument and phase reference
    // (see setInstrument() and setPhaseReference()).
    void computeUVW();

    // \name Common operations on the flags.
    // @{
    void flagsAndWithMask(flag_t mask);
    void flagsOrWithMask(flag_t mask);
    void flagsSet(flag_t value);
    void flagsNot();
    // @}

    // Station UVW coordinates in meters.
    boost::multi_array<double, 3>   uvw;
    // Flags per visibility.
    boost::multi_array<flag_t, 4>   flags;
    // Visibilities.
    boost::multi_array<dcomplex, 4> samples;
//    // Weights.
//    boost::multi_array<double, 4>   weights;
    // Covariance.
    boost::multi_array<double, 5>   covariance;

private:
    // Information about the instrument (e.g. station positions).
    Instrument::ConstPtr    itsInstrument;
    // Phase reference direction (J2000).
    casa::MDirection        itsPhaseReference;
    // Reference frequency (Hz).
    double                  itsReferenceFreq;

    // Shape of the buffer along all four dimensions (frequency, time, baseline,
    // correlation).
    VisDimensions           itsDims;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: VisBuffer                                              - //
// -------------------------------------------------------------------------- //

inline const VisDimensions &VisBuffer::dimensions() const
{
    return itsDims;
}

inline void VisBuffer::setInstrument(const Instrument::ConstPtr &instrument)
{
    itsInstrument = instrument;
}

inline Instrument::ConstPtr VisBuffer::instrument() const
{
    return itsInstrument;
}

inline size_t VisBuffer::nStations() const
{
    return itsInstrument->nStations();
}

inline const casa::MDirection &VisBuffer::getPhaseReference() const
{
    return itsPhaseReference;
}

inline void VisBuffer::setReferenceFreq(double freq)
{
    itsReferenceFreq = freq;
}

inline double VisBuffer::getReferenceFreq() const
{
    return itsReferenceFreq;
}

inline size_t VisBuffer::nFreq() const
{
    return itsDims[FREQ]->size();
}

inline size_t VisBuffer::nTime() const
{
    return itsDims[TIME]->size();
}

inline size_t VisBuffer::nBaselines() const
{
    return itsDims.nBaselines();
}

inline size_t VisBuffer::nCorrelations() const
{
    return itsDims.nCorrelations();
}

inline Box VisBuffer::domain() const
{
    return itsDims.grid().getBoundingBox();
}

inline const Grid &VisBuffer::grid() const
{
    return itsDims.grid();
}

inline const BaselineSeq &VisBuffer::baselines() const
{
    return itsDims.baselines();
}

inline const CorrelationSeq &VisBuffer::correlations() const
{
    return itsDims.correlations();
}

inline bool VisBuffer::hasUVW() const
{
    return uvw.size() > 0;
}

inline size_t VisBuffer::nSamples() const
{
    return nBaselines() * nTime() * nFreq() * nCorrelations();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
