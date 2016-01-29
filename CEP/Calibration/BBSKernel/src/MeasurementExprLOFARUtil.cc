//# MeasurementExprLOFARUtil.cc: Utility functions to construct sub-expressions
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

#include <lofar_config.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/Delay.h>
#include <BBSKernel/Expr/ElevationCut.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/FaradayRotation.h>
#include <BBSKernel/Expr/Rotation.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/MatrixMul3.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/StationResponse.h>
#include <BBSKernel/Expr/StationShift.h>
#include <BBSKernel/Expr/StationUVW.h>
#include <BBSKernel/Expr/TECU2Phase.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

Expr<Vector<2> >::Ptr
makeDirectionExpr(const casa::MDirection &direction)
{
    casa::MDirection dirJ2000(casa::MDirection::Convert(direction,
        casa::MDirection::J2000)());
    casa::Quantum<casa::Vector<casa::Double> > angles = dirJ2000.getAngle();

    Literal::Ptr ra(new Literal(angles.getBaseValue()(0)));
    Literal::Ptr dec(new Literal(angles.getBaseValue()(1)));

    AsExpr<Vector<2> >::Ptr dirExpr(new AsExpr<Vector<2> >());
    dirExpr->connect(0, ra);
    dirExpr->connect(1, dec);
    return dirExpr;
}

Expr<Vector<2> >::Ptr
makeAzElExpr(const casa::MPosition &position,
    const Expr<Vector<2> >::Ptr &direction)
{
    return Expr<Vector<2> >::Ptr(new AzEl(position, direction));
}

Expr<Vector<3> >::Ptr
makeITRFExpr(const casa::MPosition &position,
    const Expr<Vector<2> >::Ptr &direction)
{
    return Expr<Vector<3> >::Ptr(new ITRFDirection(position, direction));
}

Expr<Vector<3> >::Ptr
makeLMNExpr(const casa::MDirection &reference,
    const Expr<Vector<2> >::Ptr &direction)
{
    return Expr<Vector<3> >::Ptr(new LMN(reference, direction));
}

Expr<Vector<3> >::Ptr
makeStationUVWExpr(const casa::MPosition &arrayPosition,
    const casa::MPosition &stationPosition,
    const casa::MDirection &direction)
{
    return Expr<Vector<3> >::Ptr(new StationUVW(arrayPosition, stationPosition,
        direction));
}

Expr<Vector<2> >::Ptr
makeStationShiftExpr(const Expr<Vector<3> >::Ptr &exprUVW,
    const Expr<Vector<3> >::Ptr &exprLMN)
{
    return Expr<Vector<2> >::Ptr(new StationShift(exprUVW, exprLMN));
}

Expr<JonesMatrix>::Ptr
makeBandpassExpr(Scope &scope,
    const Station::ConstPtr &station)
{
    const string &suffix = station->name();

    Expr<Scalar>::Ptr B00 = scope(INSTRUMENT, "Bandpass:0:0:" + suffix);
    Expr<Scalar>::Ptr B11 = scope(INSTRUMENT, "Bandpass:1:1:" + suffix);

    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(B00, B11));
}

Expr<JonesMatrix>::Ptr
makeClockExpr(Scope &scope, const Station::ConstPtr &station,
    const ClockConfig &config)
{
    if (config.splitClock())
    {
        ExprParm::Ptr delay0 = scope(INSTRUMENT, "Clock:0:" + station->name());
        ExprParm::Ptr delay1 = scope(INSTRUMENT, "Clock:1:" + station->name());

        Expr<Scalar>::Ptr shift0 = Expr<Scalar>::Ptr(new Delay(delay0));
        Expr<Scalar>::Ptr shift1 = Expr<Scalar>::Ptr(new Delay(delay1));

        return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift0, shift1));
    }
    else
    {
        ExprParm::Ptr delay = scope(INSTRUMENT, "Clock:" + station->name());
        Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new Delay(delay));
        return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
    }
}

Expr<JonesMatrix>::Ptr
makeGainExpr(Scope &scope,
    const Station::ConstPtr &station,
    const GainConfig &config)
{
    Expr<Scalar>::Ptr J00, J01, J10, J11;

    string suffix0 = string(config.phasors() ? "Ampl"  : "Real") + ":"
        + station->name();
    string suffix1 = string(config.phasors() ? "Phase"  : "Imag") + ":"
        + station->name();

    ExprParm::Ptr J00_elem0 = scope(INSTRUMENT, "Gain:0:0:" + suffix0);
    ExprParm::Ptr J00_elem1 = scope(INSTRUMENT, "Gain:0:0:" + suffix1);
    ExprParm::Ptr J01_elem0 = scope(INSTRUMENT, "Gain:0:1:" + suffix0);
    ExprParm::Ptr J01_elem1 = scope(INSTRUMENT, "Gain:0:1:" + suffix1);
    ExprParm::Ptr J10_elem0 = scope(INSTRUMENT, "Gain:1:0:" + suffix0);
    ExprParm::Ptr J10_elem1 = scope(INSTRUMENT, "Gain:1:0:" + suffix1);
    ExprParm::Ptr J11_elem0 = scope(INSTRUMENT, "Gain:1:1:" + suffix0);
    ExprParm::Ptr J11_elem1 = scope(INSTRUMENT, "Gain:1:1:" + suffix1);

    if(config.phasors())
    {
        J00.reset(new AsPolar(J00_elem0, J00_elem1));
        J01.reset(new AsPolar(J01_elem0, J01_elem1));
        J10.reset(new AsPolar(J10_elem0, J10_elem1));
        J11.reset(new AsPolar(J11_elem0, J11_elem1));
    }
    else
    {
        J00.reset(new AsComplex(J00_elem0, J00_elem1));
        J01.reset(new AsComplex(J01_elem0, J01_elem1));
        J10.reset(new AsComplex(J10_elem0, J10_elem1));
        J11.reset(new AsComplex(J11_elem0, J11_elem1));
    }

    return Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10, J11));
}

Expr<JonesMatrix>::Ptr
makeTECExpr(Scope &scope,
    const Station::ConstPtr &station)
{
    ExprParm::Ptr tec = scope(INSTRUMENT, "TEC:" + station->name());

    Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new TECU2Phase(tec));
    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
}

Expr<JonesMatrix>::Ptr
makeCommonRotationExpr(Scope &scope,
    const Station::ConstPtr &station)
{
    ExprParm::Ptr chi = scope(INSTRUMENT, "CommonRotationAngle:"
        + station->name());

    return Expr<JonesMatrix>::Ptr(new Rotation(chi));
}

Expr<Scalar>::Ptr
makeCommonScalarPhaseExpr(Scope &scope,
    const Station::ConstPtr &station)
{
    ExprParm::Ptr phi = scope(INSTRUMENT, "CommonScalarPhase:"
        + station->name());

    return Expr<Scalar>::Ptr(new AsPhasor(phi));
}

Expr<JonesMatrix>::Ptr
makeDirectionalGainExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch,
    const DirectionalGainConfig &config)
{
    Expr<Scalar>::Ptr J00, J01, J10, J11;

    string suffix0 = string(config.phasors() ? "Ampl"  : "Real") + ":"
        + station->name() + ":" + patch;
    string suffix1 = string(config.phasors() ? "Phase"  : "Imag") + ":"
        + station->name() + ":" + patch;

    ExprParm::Ptr J00_elem0 = scope(INSTRUMENT, "DirectionalGain:0:0:"
        + suffix0);
    ExprParm::Ptr J00_elem1 = scope(INSTRUMENT, "DirectionalGain:0:0:"
        + suffix1);
    ExprParm::Ptr J01_elem0 = scope(INSTRUMENT, "DirectionalGain:0:1:"
        + suffix0);
    ExprParm::Ptr J01_elem1 = scope(INSTRUMENT, "DirectionalGain:0:1:"
        + suffix1);
    ExprParm::Ptr J10_elem0 = scope(INSTRUMENT, "DirectionalGain:1:0:"
        + suffix0);
    ExprParm::Ptr J10_elem1 = scope(INSTRUMENT, "DirectionalGain:1:0:"
        + suffix1);
    ExprParm::Ptr J11_elem0 = scope(INSTRUMENT, "DirectionalGain:1:1:"
        + suffix0);
    ExprParm::Ptr J11_elem1 = scope(INSTRUMENT, "DirectionalGain:1:1:"
        + suffix1);

    if(config.phasors())
    {
        J00.reset(new AsPolar(J00_elem0, J00_elem1));
        J01.reset(new AsPolar(J01_elem0, J01_elem1));
        J10.reset(new AsPolar(J10_elem0, J10_elem1));
        J11.reset(new AsPolar(J11_elem0, J11_elem1));
    }
    else
    {
        J00.reset(new AsComplex(J00_elem0, J00_elem1));
        J01.reset(new AsComplex(J01_elem0, J01_elem1));
        J10.reset(new AsComplex(J10_elem0, J10_elem1));
        J11.reset(new AsComplex(J11_elem0, J11_elem1));
    }

    return Expr<JonesMatrix>::Ptr(new AsExpr<JonesMatrix>(J00, J01, J10, J11));
}

Expr<JonesMatrix>::Ptr
makeElevationCutExpr(const Expr<Vector<2> >::Ptr &exprAzEl,
    const ElevationCutConfig &config)
{
    return Expr<JonesMatrix>::Ptr(new ElevationCut(exprAzEl,
        config.threshold()));
}

Expr<JonesMatrix>::Ptr
makeBeamExpr(const Station::ConstPtr &station,
    double refFreq,
    const Expr<Vector<3> >::Ptr &exprITRF,
    const Expr<Vector<3> >::Ptr &exprRefDelayITRF,
    const Expr<Vector<3> >::Ptr &exprRefTileITRF,
    const BeamConfig &config)
{
    StationLOFAR::ConstPtr stationLOFAR =
        dynamic_pointer_cast<const StationLOFAR>(station);

    // Check if the beam model can be computed for this station.
    if(!stationLOFAR)
    {
        THROW(BBSKernelException, "Station " << station->name() << " is not a"
            " LOFAR station or the additional information needed to compute the"
            " station beam is missing.");
    }

    StationResponse::Ptr beam(new StationResponse(exprITRF, exprRefDelayITRF,
        exprRefTileITRF, stationLOFAR->station()));

    beam->useArrayFactor(config.mode() != BeamConfig::ELEMENT);
    beam->useElementResponse(config.mode() != BeamConfig::ARRAY_FACTOR);

    if(config.useChannelFreq())
    {
        beam->useChannelFreq();
    }
    else
    {
        beam->useReferenceFreq(refFreq);
    }

    return beam;
}

Expr<JonesMatrix>::Ptr
makeDirectionalTECExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr tec = scope(INSTRUMENT, "DirectionalTEC:" + station->name()
        + ":" + patch);

    Expr<Scalar>::Ptr shift = Expr<Scalar>::Ptr(new TECU2Phase(tec));
    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(shift, shift));
}

Expr<JonesMatrix>::Ptr
makeFaradayRotationExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr rm = scope(INSTRUMENT, "RotationMeasure:" + station->name()
        + ":" + patch);

    return Expr<JonesMatrix>::Ptr(new FaradayRotation(rm));
}

Expr<JonesMatrix>::Ptr
makeRotationExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr chi = scope(INSTRUMENT, "RotationAngle:" + station->name()
        + ":" + patch);

    return Expr<JonesMatrix>::Ptr(new Rotation(chi));
}

Expr<Scalar>::Ptr
makeScalarPhaseExpr(Scope &scope,
    const Station::ConstPtr &station,
    const string &patch)
{
    ExprParm::Ptr phi = scope(INSTRUMENT, "ScalarPhase:" + station->name()
        + ":" + patch);

    return Expr<Scalar>::Ptr(new AsPhasor(phi));
}

Expr<JonesMatrix>::Ptr
makeIonosphereExpr(const Station::ConstPtr &station,
    const casa::MPosition &refPosition,
    const Expr<Vector<3> >::Ptr &exprDirection,
    const IonosphereExpr::Ptr &exprIonosphere)
{
    return exprIonosphere->construct(refPosition, station->position(),
        exprDirection);
}

Expr<JonesMatrix>::Ptr
compose(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &rhs)
{
    if(lhs)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul2(lhs, rhs));
    }

    return rhs;
}

Expr<JonesMatrix>::Ptr
compose(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<Scalar>::Ptr &rhs)
{
    if(lhs)
    {
        return Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(rhs, lhs));
    }

    return Expr<JonesMatrix>::Ptr(new AsDiagonalMatrix(rhs, rhs));
}

Expr<JonesMatrix>::Ptr
apply(const Expr<JonesMatrix>::Ptr &lhs,
    const Expr<JonesMatrix>::Ptr &coherence,
    const Expr<JonesMatrix>::Ptr &rhs)
{
    if(lhs && rhs)
    {
        return Expr<JonesMatrix>::Ptr(new MatrixMul3(lhs, coherence, rhs));
    }

    return coherence;
}

} //# namespace BBS
} //# namespace LOFAR
