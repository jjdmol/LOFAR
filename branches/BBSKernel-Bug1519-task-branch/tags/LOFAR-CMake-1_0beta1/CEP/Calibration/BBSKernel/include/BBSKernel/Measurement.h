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

#include <BBSKernel/Instrument.h>
#include <BBSKernel/VisData.h>
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

    virtual VisDimensions
        getDimensions(const VisSelection &selection) const = 0;

    virtual VisData::Ptr read(const VisSelection &selection,
        const string &column = "DATA", bool readUVW = true) const = 0;

    virtual void write(const VisSelection &selection,
        VisData::Ptr buffer, const string &column = "CORRECTED_DATA",
        bool writeFlags = true) = 0;

    double getReferenceFreq() const;
    const casa::MDirection &getPhaseCenter() const;

    const Instrument &getInstrument() const;
    const VisDimensions &getDimensions() const;

protected:
    double                  itsReferenceFreq;
    casa::MDirection        itsPhaseCenter;
    Instrument              itsInstrument;
    VisDimensions           itsDimensions;
};

// @}

// -------------------------------------------------------------------------- //
// - Measurement implementation                                             - //
// -------------------------------------------------------------------------- //

inline Measurement::~Measurement()
{
}

inline const Instrument &Measurement::getInstrument() const
{
    return itsInstrument;
}

inline const casa::MDirection &Measurement::getPhaseCenter() const
{
    return itsPhaseCenter;
}

inline double Measurement::getReferenceFreq() const
{
    return itsReferenceFreq;
}

inline const VisDimensions &Measurement::getDimensions() const
{
    return itsDimensions;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
