//# GaussianCoherence.cc: Spatial coherence function of an elliptical
//#     gaussian source.
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
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
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

GaussianCoherence::GaussianCoherence(const GaussianSource *source,
        StatUVW *station1,
        StatUVW *station2)
    :   itsSource(source),
        itsStation1(station1),
        itsStation2(station2)
{
	addChild(itsSource->getI());
	addChild(itsSource->getQ());
	addChild(itsSource->getU());
	addChild(itsSource->getV());
	addChild(itsSource->getMajor());
	addChild(itsSource->getMinor());
	addChild(itsSource->getPhi());
}


GaussianCoherence::~GaussianCoherence()
{
}


JonesResult GaussianCoherence::getJResult(const Request &request)
{
    // Evaluate source parameters.
    // Note: The result of any parameter is either scalar or it is 2D and
    // conforms to the size of the request.
    Result ikBuf, qkBuf, ukBuf, vkBuf, majorBuf, minorBuf, phiBuf;
    const Result &ik =
        getChild(GaussianCoherence::IN_I).getResultSynced(request, ikBuf);
    const Result &qk =
        getChild(GaussianCoherence::IN_Q).getResultSynced(request, qkBuf);
    const Result &uk =
        getChild(GaussianCoherence::IN_U).getResultSynced(request, ukBuf);
    const Result &vk =
        getChild(GaussianCoherence::IN_V).getResultSynced(request, vkBuf);
    const Result &major =
        getChild(GaussianCoherence::IN_MAJOR).getResultSynced(request, majorBuf);
    const Result &minor =
        getChild(GaussianCoherence::IN_MINOR).getResultSynced(request, minorBuf);
    const Result &phi =
        getChild(GaussianCoherence::IN_PHI).getResultSynced(request, phiBuf);
    
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
    DBGASSERT(uStation1.getValue().nx() == 1 && uStation1.getValue().nelements() == request.ny());
    DBGASSERT(vStation1.getValue().nx() == 1 && vStation1.getValue().nelements() == request.ny());
    DBGASSERT(uStation2.getValue().nx() == 1 && uStation2.getValue().nelements() == request.ny());
    DBGASSERT(vStation2.getValue().nx() == 1 && vStation1.getValue().nelements() == request.ny());

    // Compute baseline uv-coordinates (1D in time).
    Matrix uBaseline = uStation2.getValue() - uStation1.getValue();
    Matrix vBaseline = vStation2.getValue() - vStation1.getValue();

    // Compute spatial coherence for a gaussian source.
    Matrix coherence = computeCoherence(request, uBaseline, vBaseline,
      major.getValue(), minor.getValue(), phi.getValue());

    // Allocate the result.
    JonesResult result(request.nspid());
    Result &resXX = result.result11();
    Result &resXY = result.result12();
    Result &resYX = result.result21();
    Result &resYY = result.result22();
    
    // Compute main result.
    Matrix iScaled = 0.5 * ik.getValue();
    Matrix qScaled = 0.5 * qk.getValue();
    Matrix uvScaled = 0.5 * tocomplex(uk.getValue(), vk.getValue());

    resXX.setValue((iScaled + qScaled) * coherence);
    resXY.setValue(uvScaled * coherence);
    resYX.setValue(conj(uvScaled) * coherence);
    resYY.setValue((iScaled - qScaled) * coherence);
    
    // Compute perturbed values.
    enum PVIteratorIndex
    {
        RESULT_I = 0,
        RESULT_Q = 0,
        RESULT_U = 0,
        RESULT_V = 0,
        RESULT_MAJOR = 0,
        RESULT_MINOR = 0,
        RESULT_PHI = 0,
        N_PVIteratorIndex
    };
    
    PValueIterator pvIter[N_PVIteratorIndex];
    pvIter[RESULT_I] = PValueIterator(ik);
    pvIter[RESULT_Q] = PValueIterator(qk);
    pvIter[RESULT_U] = PValueIterator(uk);
    pvIter[RESULT_V] = PValueIterator(vk);
    pvIter[RESULT_MAJOR] = PValueIterator(major);
    pvIter[RESULT_MINOR] = PValueIterator(minor);
    pvIter[RESULT_PHI] = PValueIterator(phi);

    Matrix iScaledPerturbed, qScaledPerturbed, uvScaledPerturbed,
        coherencePerturbed;

    bool atEnd = false;
    while(!atEnd)
    {
        atEnd = true;

        PValueKey currKey(static_cast<size_t>(-1), 0);
        for(size_t i = 0; i < N_PVIteratorIndex; ++i)
        {
            if(!pvIter[i].atEnd() && pvIter[i].key().parmId < currKey.parmId)
            {
                currKey = pvIter[i].key();
                atEnd = false;
            }
        }

        if(!atEnd)
        {
            bool evalIQ = evalUV = evalCoherence = false;

            iScaledPerturbed = iScaled;
            qScaledPerturbed = qScaled;
            uvScaledPerturbed = uvScaled;
            coherencePerturbed = coherence;

            if(pvIter[RESULT_I].isAt(currKey) || pvIter[RESULT_Q].isAt(currKey))
            {
                evalIQ = true;
                iScaledPerturbed = 0.5 * pvIter[RESULT_I].next(currKey);
                qScaledPerturbed = 0.5 * pvIter[RESULT_Q].next(currKey);
            }
            
            if(pvIter[RESULT_U].isAt(currKey) || pvIter[RESULT_V].isAt(currKey))
            {
                evalUV = true;
                uvScaledPerturbed =
                    0.5 * tocomplex(pvIter[RESULT_U].next(currKey),
                        pvIter[RESULT_V].next(currKey));
            }

            if(pvIter[RESULT_MAJOR].isAt(currKey)
              || pvIter[RESULT_MINOR].isAt(currKey)
              || pvIter[RESULT_PHI].isAt(currKey))
            {
                evalCoherence = true;
                coherencePerturbed = computeCoherence(request,
                    uBaseline, vBaseline,
                    pvIter[RESULT_MAJOR].next(currKey),
                    pvIter[RESULT_MINOR].next(currKey),
                    pvIter[RESULT_PHI].next(currKey));
            }              

            if(evalIQ || evalCoherence)
            {        
                resXX.setPerturbedValue(currKey,
                    (iScaledPerturbed + qScaledPerturbed) * coherencePerturbed);
                resYY.setPerturbedValue(currKey,
                    (iScaledPerturbed - qScaledPerturbed) * coherencePerturbed);
            }

            if(evalUV || evalCoherence)
            {
                resXY.setPerturbedValue(currKey, uvScaledPerturbed
                    * coherencePerturbed);
                resYX.setPerturbedValue(currKey, conj(uvScaledPerturbed)
                    * coherencePerturbed);
            }
        }
    }
    
    return result;
}


Matrix GaussianCoherence::computeCoherence(const Request &request,
    const Matrix &uBaseline, const Matrix &vBaseline,
    const Matrix &major, const Matrix &minor, const Matrix &phi)
{
    // Compute dot product of rotated, scaled uv-vector with itself (1D in time).
    Matrix cosPhi(cos(phi));
    Matrix sinPhi(sin(phi));
    Matrix uvTransformedDotProduct =
        sqr(major * (uBaseline * cosPhi - vBaseline * sinPhi))
        + sqr(minor * (uBaseline * sinPhi + vBaseline * cosPhi));

    // Allocate the result matrix (2D).
    Matrix result;
    double *valuep = result.setDoubleFormat(request.nx(), request.ny());

    // Compute unnormalized spatial coherence (2D).
    double cSqr = casa::C::c * casa::C::c;
    for(int t = 0; t < request.ny(); ++t)
    {
        double freq = request.domain().startX() + request.stepX() / 2.0;
        const double uv = casa::C::_2pi * uvTransformedDotProduct.getDouble(0, t);

        for(int f = 0; f < request.nx(); ++f)
        {
            *(valuep++) = exp(-(freq * freq * uv) / cSqr);
            freq += request.stepX();
        }
    }

    return result;
}


#ifdef EXPR_GRAPH
std::string GaussianCoherence::getLabel()
{
    return std::string("GaussianCoherence\\nSpatial coherence function of"
        " an elliptical gaussian source\\n" + itsSource->getName() + " ("
        + itsSource->getGroupName() + ")");
}
#endif

} // namespace BBS
} // namespace LOFAR
