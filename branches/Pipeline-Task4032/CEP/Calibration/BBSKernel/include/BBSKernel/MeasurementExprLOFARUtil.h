//# MeasurementExprLOFARUtil.h: Utility functions to construct sub-expressions
//# for the LOFAR measurement expression.
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

#ifndef LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFARUTIL_H
#define LOFAR_BBSKERNEL_MEASUREMENTEXPRLOFARUTIL_H

// \file
// Utility functions to construct sub-expressions for the LOFAR measurement
// expression.

#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/IonosphereExpr.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Instrument.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSKernel
// @{

Expr<Vector<2> >::Ptr
makeDirectionExpr(const casa::MDirection &direction);

Expr<Vector<2> >::Ptr
makeAzElExpr(const casa::MPosition &position,
    const Expr<Vector<2> >::Ptr &direction);

Expr<Vector<3> >::Ptr
makeITRFExpr(const casa::MPosition &position,
    const Expr<Vector<2> >::Ptr &direction);

Expr<Vector<3> >::Ptr
makeLMNExpr(const casa::MDirection &reference,
    const Expr<Vector<2> >::Ptr &direction);

Expr<Vector<3> >::Ptr
makeStationUVWExpr(const casa::MPosition &arrayPosition,
    const casa::MPosition &stationPosition,
    const casa::MDirection &direction);

Expr<Vector<2> >::Ptr
makeStationShiftExpr(const Expr<Vector<3> >::Ptr &exprUVW,
    const Expr<Vector<3> >::Ptr &exprLMN);

// Direction independent effects.
Expr<JonesMatrix>::Ptr
makeBandpassExpr(Scope &scope,
    const Station::ConstPtr &station);

Expr<JonesMatrix>::Ptr
makeClockExpr(Scope &scope,
    const Station::ConstPtr &station);

Expr<JonesMatrix>::Ptr
makeGainExpr(Scope &scope,
    const Station::ConstPtr &station,
    bool phasors);

Expr<JonesMatrix>::Ptr
makeTECExpr(Scope &scope,
    const Station::ConstPtr &station);

Expr<JonesMatrix>::Ptr
makeCommonRotationExpr(Scope &scope,
    const Station::ConstPtr &station);

Expr<Scalar>::Ptr
makeCommonScalarPhaseExpr(Scope &scope,
    const Station::ConstPtr &station);

// Direction dependent effects.
Expr<JonesMatrix>::Ptr
makeDirectionalGainExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch,
    bool phasors);

Expr<JonesMatrix>::Ptr
makeElevationCutExpr(const Expr<Vector<2> >::Ptr &exprAzEl,
    const ElevationCutConfig &config);

Expr<JonesMatrix>::Ptr
makeBeamExpr(const Station::ConstPtr &station,
    double refFreq,
    const Expr<Vector<3> >::Ptr &exprITRF,
    const Expr<Vector<3> >::Ptr &exprRefDelayITRF,
    const Expr<Vector<3> >::Ptr &exprRefTileITRF,
    const BeamConfig &config);

Expr<JonesMatrix>::Ptr
makeDirectionalTECExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch);

Expr<JonesMatrix>::Ptr
makeFaradayRotationExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch);

Expr<JonesMatrix>::Ptr
makeRotationExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch);

Expr<Scalar>::Ptr
makeScalarPhaseExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch);

Expr<JonesMatrix>::Ptr
makeIonosphereExpr(const Station::ConstPtr &station,
    const casa::MPosition &refPosition,
    const Expr<Vector<2> >::Ptr &exprAzEl,
    const IonosphereExpr::Ptr &exprIonosphere);

// Right multiply \p lhs by \p rhs. Return \p rhs if \p lhs is uninitialized.
Expr<JonesMatrix>::Ptr
compose(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &rhs);

// Right multiply \p lhs by \p rhs. Return \p rhs as a diagonal Jones matrix
// if \p lhs is uninitialized.
Expr<JonesMatrix>::Ptr
compose(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<Scalar>::Ptr &rhs);

// Construct \p lhs * \p coherence * (\p rhs)^H. Return \p coherence if
// either \p lhs or \p rhs are uninitialized.
Expr<JonesMatrix>::Ptr
apply(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &coherence,
    const Expr<JonesMatrix>::Ptr &rhs);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
