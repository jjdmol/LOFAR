//# GaussianCoherence.cc: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/lofar_complex.h>
#include <Common/lofar_math.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{
using LOFAR::exp;
using LOFAR::conj;

GaussianCoherence::GaussianCoherence(const GaussianSource::ConstPointer &source,
    const StatUVW::ConstPointer &station1,
    const StatUVW::ConstPointer &station2)
    :   itsStation1(station1),
        itsStation2(station2)
{
	addChild(source->getI());
	addChild(source->getQ());
	addChild(source->getU());
	addChild(source->getV());
	addChild(source->getMajor());
	addChild(source->getMinor());
	addChild(source->getPhi());
}

JonesResult GaussianCoherence::getJResult(const Request &request)
{
    enum ChildExprIndex
    {
        IN_I, IN_Q, IN_U, IN_V, IN_MAJOR, IN_MINOR, IN_PHI
    };

    // Evaluate source parameters.
    // Note: The result of any parameter is either scalar or it is 2D and
    // conforms to the size of the request.
    Result ikBuf, qkBuf, ukBuf, vkBuf, siBuf, majorBuf, minorBuf, phiBuf;
    const Result &ik = getChild(IN_I).getResultSynced(request, ikBuf);
    const Result &qk = getChild(IN_Q).getResultSynced(request, qkBuf);
    const Result &uk = getChild(IN_U).getResultSynced(request, ukBuf);
    const Result &vk = getChild(IN_V).getResultSynced(request, vkBuf);
    const Result &major = getChild(IN_MAJOR).getResultSynced(request, majorBuf);
    const Result &minor = getChild(IN_MINOR).getResultSynced(request, minorBuf);
    const Result &phi = getChild(IN_PHI).getResultSynced(request, phiBuf);

    // Assume major, minor, phi are scalar.
    DBGASSERT(!major.getValue().isArray());
    DBGASSERT(!minor.getValue().isArray());
    DBGASSERT(!phi.getValue().isArray());

    // Note: The result of a StatUVW node is always 1D in time.
    const Result &uStation1 = itsStation1->getU(request);
    const Result &vStation1 = itsStation1->getV(request);
    const Result &uStation2 = itsStation2->getU(request);
    const Result &vStation2 = itsStation2->getV(request);

    // Check pre-conditions.
    DBGASSERT(uStation1.getValue().nx() == 1
        && static_cast<uint>(uStation1.getValue().nelements())
            == request.getTimeslotCount());
    DBGASSERT(vStation1.getValue().nx() == 1
        && static_cast<uint>(vStation1.getValue().nelements())
            == request.getTimeslotCount());
    DBGASSERT(uStation2.getValue().nx() == 1
        && static_cast<uint>(uStation2.getValue().nelements())
            == request.getTimeslotCount());
    DBGASSERT(vStation2.getValue().nx() == 1
        && static_cast<uint>(vStation1.getValue().nelements())
            == request.getTimeslotCount());

    // Allocate the result.
    JonesResult result;
    result.init();

    Result &resXX = result.result11();
    Result &resXY = result.result12();
    Result &resYX = result.result21();
    Result &resYY = result.result22();

    // Compute baseline uv-coordinates (1D in time).
    Matrix uBaseline = uStation2.getValue() - uStation1.getValue();
    Matrix vBaseline = vStation2.getValue() - vStation1.getValue();

    // Compute spatial coherence for a gaussian source.
    Matrix coherence_2 = computeCoherence(request, uBaseline, vBaseline,
        major.getValue(), minor.getValue(), phi.getValue()) * 0.5;

    // Compute main result.
    Matrix uv = tocomplex(uk.getValue(), vk.getValue());

    resXX.setValue(coherence_2 * (ik.getValue() + qk.getValue()));
    resXY.setValue(coherence_2 * uv);
    resYX.setValue(coherence_2 * conj(uv));
    resYY.setValue(coherence_2 * (ik.getValue() - qk.getValue()));

    // Compute the perturbed values.
    enum PValues
    {
        PV_I, PV_Q, PV_U, PV_V, PV_MAJOR, PV_MINOR, PV_PHI, N_PValues
    };

    const Result *pvSet[N_PValues] = {&ik, &qk, &uk, &vk, &major, &minor, &phi};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        bool evalIQ = pvIter.hasPValue(PV_I) || pvIter.hasPValue(PV_Q);
        bool evalUV = pvIter.hasPValue(PV_U) || pvIter.hasPValue(PV_V);
        bool evalCoh = pvIter.hasPValue(PV_MAJOR) || pvIter.hasPValue(PV_MINOR)
            || pvIter.hasPValue(PV_PHI);

        Matrix pvUV(uv);
        Matrix pvCoh_2(coherence_2);

        if(evalUV)
        {
            pvUV = tocomplex(pvIter.value(PV_U), pvIter.value(PV_V));
        }

        if(evalCoh)
        {
            pvCoh_2 = computeCoherence(request, uBaseline, vBaseline,
                pvIter.value(PV_MAJOR), pvIter.value(PV_MINOR),
                pvIter.value(PV_PHI)) * 0.5;
        }

        if(evalIQ || evalCoh)
        {
            const Matrix &pvI = pvIter.value(PV_I);
            const Matrix &pvQ = pvIter.value(PV_Q);

            resXX.setPerturbedValue(pvIter.key(), pvCoh_2 * (pvI + pvQ));
            resYY.setPerturbedValue(pvIter.key(), pvCoh_2 * (pvI - pvQ));
        }

        if(evalUV || evalCoh)
        {
            resXY.setPerturbedValue(pvIter.key(), pvCoh_2 * pvUV);
            resYX.setPerturbedValue(pvIter.key(), pvCoh_2 * conj(pvUV));
        }

        pvIter.next();
    }

    return result;
}

Matrix GaussianCoherence::computeCoherence(const Request &request,
    const Matrix &uBaseline, const Matrix &vBaseline,
    const Matrix &major, const Matrix &minor, const Matrix &phi)
{
    // Compute dot product of a rotated, scaled uv-vector with itself (1D in
    // time) and pre-multiply with 2.0 * PI^2 / C^2.
    Matrix cosPhi(cos(phi));
    Matrix sinPhi(sin(phi));
    Matrix uvTransformed = (2.0 * casa::C::pi * casa::C::pi)
        / (casa::C::c * casa::C::c)
        * (sqr(major * (uBaseline * cosPhi - vBaseline * sinPhi))
        + sqr(minor * (uBaseline * sinPhi + vBaseline * cosPhi)));

    // Allocate the result matrix (2D).
    const uint nChannels = request.getChannelCount();
    const uint nTimeslots = request.getTimeslotCount();

    Matrix result;
    double *valuep = result.setDoubleFormat(nChannels, nTimeslots);

    // Compute non-normalized spatial coherence (2D).
    Axis::ShPtr freqAxis(request.getGrid()[FREQ]);
    for(uint t = 0; t < nTimeslots; ++t)
    {
        const double uv = uvTransformed.getDouble(0, t);
        for(uint f = 0; f < nChannels; ++f)
        {
            const double freq = freqAxis->center(f);
            *(valuep++) = exp(-(freq * freq * uv));
        }
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
