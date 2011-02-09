//# Measurement.h: Interface class for measurement I/O.
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

#ifndef LOFAR_BBSKERNEL_MEASUREMENT_H
#define LOFAR_BBSKERNEL_MEASUREMENT_H

#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/VisDimensions.h>
#include <BBSKernel/VisSelection.h>

#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Measurement
{
public:
    typedef shared_ptr<Measurement>         Ptr;
    typedef shared_ptr<const Measurement>   ConstPtr;

    virtual ~Measurement();

    virtual VisDimensions dimensions(const VisSelection &selection) const = 0;

    virtual VisBuffer::Ptr read(const VisSelection &selection = VisSelection(),
        const string &column = "DATA") const = 0;

    virtual void write(VisBuffer::Ptr buffer,
        const VisSelection &selection = VisSelection(),
        const string &column = "CORRECTED_DATA",
        bool writeFlags = true, flag_t flagMask = ~flag_t(0)) = 0;

    // Apply the given filter to the baselines contained in the Measurement and
    // return the result as a (boolean) baseline mask.
    virtual BaselineMask asMask(const string &filter) const = 0;

    // add MODEL_DATA and/or CORRECTED_DATA according to itsClearcalColFlag
    //virtual void addClearcalColumns();
    
    double getReferenceFreq() const;
    const casa::MDirection &getPhaseReference() const;

    const Instrument &instrument() const;
    const VisDimensions &dimensions() const;

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

protected:
    double                  itsReferenceFreq;
    casa::MDirection        itsPhaseReference;
    Instrument              itsInstrument;
    VisDimensions           itsDims;
};

// @}

// -------------------------------------------------------------------------- //
// - Measurement implementation                                             - //
// -------------------------------------------------------------------------- //

inline Measurement::~Measurement()
{
}

inline const Instrument &Measurement::instrument() const
{
    return itsInstrument;
}

inline const casa::MDirection &Measurement::getPhaseReference() const
{
    return itsPhaseReference;
}

inline double Measurement::getReferenceFreq() const
{
    return itsReferenceFreq;
}

inline size_t Measurement::nFreq() const
{
    return itsDims[FREQ]->size();
}

inline size_t Measurement::nTime() const
{
    return itsDims[TIME]->size();
}

inline size_t Measurement::nBaselines() const
{
    return itsDims.nBaselines();
}

inline size_t Measurement::nCorrelations() const
{
    return itsDims.nCorrelations();
}

inline Box Measurement::domain() const
{
    return itsDims.grid().getBoundingBox();
}

inline const Grid &Measurement::grid() const
{
    return itsDims.grid();
}

inline const BaselineSeq &Measurement::baselines() const
{
    return itsDims.baselines();
}

inline const CorrelationSeq &Measurement::correlations() const
{
    return itsDims.correlations();
}

inline const VisDimensions &Measurement::dimensions() const
{
    return itsDims;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
