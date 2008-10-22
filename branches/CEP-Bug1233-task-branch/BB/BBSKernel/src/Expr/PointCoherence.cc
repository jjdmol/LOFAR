//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
    :   itsSource(source)
{
    addChild (itsSource->getI());
    addChild (itsSource->getQ());
    addChild (itsSource->getU());
    addChild (itsSource->getV());
}


PointCoherence::~PointCoherence()
{
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
    Result ikBuf, qkBuf, ukBuf, vkBuf;
    const Result& ik = getChild(0).getResultSynced(request, ikBuf);
    const Result& qk = getChild(1).getResultSynced(request, qkBuf);
    const Result& uk = getChild(2).getResultSynced(request, ukBuf);
    const Result& vk = getChild(3).getResultSynced(request, vkBuf);
    
    // Compute main value.
    Matrix uvk_2 = 0.5 * tocomplex(uk.getValue(), vk.getValue());
    resXX.setValue(0.5 * (ik.getValue() + qk.getValue()));
    resXY.setValue(uvk_2);
    resYX.setValue(conj(uvk_2));
    resYY.setValue(0.5 * (ik.getValue() - qk.getValue()));
    
    // Compute perturbed values.
    enum PValues
    {   PV_I, PV_Q, PV_U, PV_V, N_PValues };
    
    const Result *pvSet[N_PValues] = {&ik, &qk, &uk, &vk};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        if(pvIter.hasPValue(PV_I) || pvIter.hasPValue(PV_Q))
        {
          const Matrix &pvI = pvIter.value(PV_I);
          const Matrix &pvQ = pvIter.value(PV_Q);
          resXX.setPerturbedValue(pvIter.key(), 0.5 * (pvI + pvQ)); 
          resYY.setPerturbedValue(pvIter.key(), 0.5 * (pvI - pvQ)); 
        }

        if(pvIter.hasPValue(PV_U) || pvIter.hasPValue(PV_V))
        {
            const Matrix &pvU = pvIter.value(PV_U);
            const Matrix &pvV = pvIter.value(PV_V);
            Matrix uvkp_2 = 0.5 * tocomplex(pvU, pvV);
            resXY.setPerturbedValue(pvIter.key(), uvkp_2);
            resYX.setPerturbedValue(pvIter.key(), conj(uvkp_2));
        }

        pvIter.next();
    }

    return result;
}

#ifdef EXPR_GRAPH
string PointCoherence::getLabel()
{
    return string("PointCoherence\\nSpatial coherence function of a point"
        " source\\n" + itsSource->getName());
}
#endif

} // namespace BBS
} // namespace LOFAR
