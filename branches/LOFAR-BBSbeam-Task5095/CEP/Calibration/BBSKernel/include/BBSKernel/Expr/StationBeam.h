//# StationBeam.h: Beam forming the signals from individual dipoles or
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

#ifndef LOFAR_BBSKERNEL_EXPR_StationBeam_H
#define LOFAR_BBSKERNEL_EXPR_StationBeam_H

// \file
// Beam forming the signals from individual dipoles or tiles into a single
// station beam.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StationBeam: public BasicTernaryExpr<Vector<3>, Vector<3>, Vector<3>, JonesMatrix>
{
public:
    typedef shared_ptr<StationBeam>       Ptr;
    typedef shared_ptr<const StationBeam> ConstPtr;

    StationBeam(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &station0,
        const Expr<Vector<3> >::ConstPtr &tile0,
        const StationResponse::Station::ConstPtr &station,
        bool conjugate = false);

    StationBeam(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &station0,
        const Expr<Vector<3> >::ConstPtr &tile0,
        const StationResponse::Station::ConstPtr &station,
        double refFrequency,
        bool conjugate = false);

protected:
    const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &station0,
        const Vector<3>::View &tile0) const;

private:
    //Expr<Vector<3> >::ConstPtr          itsDirection;
    //Expr<Vector<3> >::ConstPtr          itsReference;
    //vector<Expr<JonesMatrix>::ConstPtr> itsElementBeam;

    StationResponse::Station::ConstPtr  itsStation;
    bool                                itsUseChannelFreq;
    double                              itsRefFrequency;
    bool                                itsConjugateFlag;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: StationBeam                                      - //
// -------------------------------------------------------------------------- //

} //# namespace BBS
} //# namespace LOFAR

#endif
