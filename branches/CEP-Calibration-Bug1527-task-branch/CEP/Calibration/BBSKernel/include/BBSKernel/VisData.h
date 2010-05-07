//# VisData.h: A buffer of visibility data and associated information (e.g.
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

#ifndef LOFAR_BBSKERNEL_VISDATA_H
#define LOFAR_BBSKERNEL_VISDATA_H

#include <Common/lofar_smartptr.h>

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

class VisData
{
public:
    typedef shared_ptr<VisData>         Ptr;
    typedef shared_ptr<const VisData>   ConstPtr;

    enum TimeFlag
    {
        AVAILABLE           = 0,
        UNAVAILABLE         = 1<<1,
        FLAGGED_IN_INPUT    = 1<<2,
        N_TimeFlag
    };

    VisData();
    VisData(const VisDimensions &dims);

    // Access to the dimensions of this buffer.
    const VisDimensions &dimensions() const;

    void setPhaseCenter(const casa::MDirection &center);
    casa::MDirection getPhaseCenter() const;

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
    //@}

    // Data.
    boost::multi_array<double, 3>       uvw;
    boost::multi_array<tslot_flag_t, 2> tslot_flag;
    boost::multi_array<flag_t, 4>       vis_flag;
    boost::multi_array<sample_t, 4>     vis_data;

private:
    casa::MDirection    itsPhaseCenter;
    double              itsReferenceFreq;

    // Shape of the buffer along all four dimensions (frequency, time, baseline,
    // correlation).
    VisDimensions       itsDims;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: VisData                                                - //
// -------------------------------------------------------------------------- //

inline casa::MDirection VisData::getPhaseCenter() const
{
    return itsPhaseCenter;
}

inline double VisData::getReferenceFreq() const
{
    return itsReferenceFreq;
}

inline size_t VisData::nFreq() const
{
    return itsDims[FREQ]->size();
}

inline size_t VisData::nTime() const
{
    return itsDims[TIME]->size();
}

inline size_t VisData::nBaselines() const
{
    return itsDims.nBaselines();
}

inline size_t VisData::nCorrelations() const
{
    return itsDims.nCorrelations();
}

inline Box VisData::domain() const
{
    return itsDims.grid().getBoundingBox();
}

inline const Grid &VisData::grid() const
{
    return itsDims.grid();
}

inline const BaselineSeq &VisData::baselines() const
{
    return itsDims.baselines();
}

inline const CorrelationSeq &VisData::correlations() const
{
    return itsDims.correlations();
}

inline const VisDimensions &VisData::dimensions() const
{
    return itsDims;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
