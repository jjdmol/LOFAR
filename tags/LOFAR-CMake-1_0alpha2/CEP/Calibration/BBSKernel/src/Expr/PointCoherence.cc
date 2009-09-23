//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
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
//include <Common/Timer.h>

#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

PointCoherence::PointCoherence(const PointSource::ConstPointer &source)
{
    addChild(source->getI());
    addChild(source->getQ());
    addChild(source->getU());
    addChild(source->getV());
    addChild(source->getSpectralIndex());
}


JonesResult PointCoherence::getJResult(const Request &request)
{
    // Allocate the result.
    JonesResult result;
    result.init();

    Result& resXX = result.result11();
    Result& resXY = result.result12();
    Result& resYX = result.result21();
    Result& resYY = result.result22();

    // Calculate the source fluxes.
    Result ikBuf, qkBuf, ukBuf, vkBuf, siBuf;
    const Result& ik = getChild(0).getResultSynced(request, ikBuf);
    const Result& qk = getChild(1).getResultSynced(request, qkBuf);
    const Result& uk = getChild(2).getResultSynced(request, ukBuf);
    const Result& vk = getChild(3).getResultSynced(request, vkBuf);
    const Result& si = getChild(4).getResultSynced(request, siBuf);

    // Compute main value.
    Matrix uvk_2 = tocomplex(uk.getValue(), vk.getValue()) * 0.5;
    resXX.setValue(si.getValue() * (ik.getValue() + qk.getValue()) * 0.5);
    resXY.setValue(uvk_2);
    resYX.setValue(conj(uvk_2));
    resYY.setValue(si.getValue() * (ik.getValue() - qk.getValue()) * 0.5);

    // Compute perturbed values.
    enum PValues
    { PV_I, PV_Q, PV_U, PV_V, PV_SI, N_PValues };

    const Result *pvSet[N_PValues] = {&ik, &qk, &uk, &vk, &si};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        if(pvIter.hasPValue(PV_I) || pvIter.hasPValue(PV_Q)
            || pvIter.hasPValue(PV_SI))
        {
          const Matrix &pvI = pvIter.value(PV_I);
          const Matrix &pvQ = pvIter.value(PV_Q);
          const Matrix &pvSI = pvIter.value(PV_SI);
          resXX.setPerturbedValue(pvIter.key(), pvSI * (pvI + pvQ) * 0.5);
          resYY.setPerturbedValue(pvIter.key(), pvSI * (pvI - pvQ) * 0.5);
        }

        if(pvIter.hasPValue(PV_U) || pvIter.hasPValue(PV_V))
        {
            const Matrix &pvU = pvIter.value(PV_U);
            const Matrix &pvV = pvIter.value(PV_V);
            Matrix uvkp_2 = tocomplex(pvU, pvV) * 0.5;
            resXY.setPerturbedValue(pvIter.key(), uvkp_2);
            resYX.setPerturbedValue(pvIter.key(), conj(uvkp_2));
        }

        pvIter.next();
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
