//# StationBeamFormer.h: Beam forming the signals from individual dipoles or
//# tiles into a single station beam.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_EXPR_STATIONBEAMFORMER_H
#define LOFAR_BBSKERNEL_EXPR_STATIONBEAMFORMER_H

// \file
// Beam forming the signals from individual dipoles or tiles into a single
// station beam.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StationBeamFormer: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<StationBeamFormer>       Ptr;
    typedef shared_ptr<const StationBeamFormer> ConstPtr;

    template <typename T>
    StationBeamFormer(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &reference,
        const Station::ConstPtr &station, T first, T last,
        bool conjugate = false);

    template <typename T>
    StationBeamFormer(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &reference,
        const Station::ConstPtr &station, T first, T last,
        double refFrequency,
        bool conjugate = false);

    virtual ~StationBeamFormer();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

    const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &reference,
        const vector<JonesMatrix::View> &beam) const;

private:
    Expr<Vector<3> >::ConstPtr          itsDirection;
    Expr<Vector<3> >::ConstPtr          itsReference;
    vector<Expr<JonesMatrix>::ConstPtr> itsElementBeam;

    Station::ConstPtr                   itsStation;
    bool                                itsUseChannelFreq;
    double                              itsRefFrequency;
    bool                                itsConjugateFlag;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: StationBeamFormer                                      - //
// -------------------------------------------------------------------------- //
template <typename T>
StationBeamFormer::StationBeamFormer
    (const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &reference,
    const Station::ConstPtr &station, T first, T last, bool conjugate)
    :   itsDirection(direction),
        itsReference(reference),
        itsStation(station),
        itsUseChannelFreq(true),
        itsRefFrequency(0.0),
        itsConjugateFlag(conjugate)
{
    connect(itsDirection);
    connect(itsReference);

    ASSERT(distance(first, last) == itsStation->nField());
    for(; first != last; ++first)
    {
      connect(*first);
      itsElementBeam.push_back(*first);
    }
}

template <typename T>
StationBeamFormer::StationBeamFormer
    (const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &reference,
    const Station::ConstPtr &station, T first, T last, double refFrequency,
    bool conjugate)
    :   itsDirection(direction),
        itsReference(reference),
        itsStation(station),
        itsUseChannelFreq(false),
        itsRefFrequency(refFrequency),
        itsConjugateFlag(conjugate)
{
    connect(itsDirection);
    connect(itsReference);

    ASSERT(distance(first, last) == itsStation->nField());
    for(; first != last; ++first)
    {
      connect(*first);
      itsElementBeam.push_back(*first);
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
