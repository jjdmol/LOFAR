//# MeqGaussianCoherency.cc: Spatial coherence function of an elliptical
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
#include <BBSKernel/MNS/MeqGaussianCoherency.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
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

MeqGaussianCoherency::MeqGaussianCoherency(const MeqGaussianSource *source,
        MeqStatUVW *station1,
        MeqStatUVW *station2)
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


MeqGaussianCoherency::~MeqGaussianCoherency()
{
}


MeqJonesResult MeqGaussianCoherency::getJResult(const MeqRequest &request)
{
    // Evaluate source parameters.
    // Note: The result of any parameter is either scalar or it is 2D and
    // conforms to the size of the request.
    MeqResult ikBuf, qkBuf, ukBuf, vkBuf, majorBuf, minorBuf, phiBuf;
    const MeqResult &ik =
        getChild(MeqGaussianCoherency::IN_I).getResultSynced(request, ikBuf);
    const MeqResult &qk =
        getChild(MeqGaussianCoherency::IN_Q).getResultSynced(request, qkBuf);
    const MeqResult &uk =
        getChild(MeqGaussianCoherency::IN_U).getResultSynced(request, ukBuf);
    const MeqResult &vk =
        getChild(MeqGaussianCoherency::IN_V).getResultSynced(request, vkBuf);
    const MeqResult &major =
        getChild(MeqGaussianCoherency::IN_MAJOR).getResultSynced(request, majorBuf);
    const MeqResult &minor =
        getChild(MeqGaussianCoherency::IN_MINOR).getResultSynced(request, minorBuf);
    const MeqResult &phi =
        getChild(MeqGaussianCoherency::IN_PHI).getResultSynced(request, phiBuf);
    
    // Assume major, minor, phi are scalar.
    DBGASSERT(!major.getValue().isArray());
    DBGASSERT(!minor.getValue().isArray());
    DBGASSERT(!phi.getValue().isArray());

    // Note: The result of a MeqStatUVW node is always 1D in time.
    const MeqResult &uStation1 = itsStation1->getU(request);
    const MeqResult &vStation1 = itsStation1->getV(request);
    const MeqResult &uStation2 = itsStation2->getU(request);
    const MeqResult &vStation2 = itsStation2->getV(request);

    // Check pre-conditions.
    DBGASSERT(uStation1.getValue().nx() == 1 && uStation1.getValue().nelements() == request.ny());
    DBGASSERT(vStation1.getValue().nx() == 1 && vStation1.getValue().nelements() == request.ny());
    DBGASSERT(uStation2.getValue().nx() == 1 && uStation2.getValue().nelements() == request.ny());
    DBGASSERT(vStation2.getValue().nx() == 1 && vStation1.getValue().nelements() == request.ny());

    // Compute baseline uv-coordinates (1D in time).
    MeqMatrix uBaseline = uStation2.getValue() - uStation1.getValue();
    MeqMatrix vBaseline = vStation2.getValue() - vStation1.getValue();

    // Compute scaled Stokes vector.
    MeqMatrix uvScaled = tocomplex(uk.getValue(), vk.getValue()) * 0.5;
    MeqMatrix iScaled = ik.getValue() * 0.5;
    MeqMatrix qScaled = qk.getValue() * 0.5;

    // Compute spatial coherence for a gaussian source.
    MeqMatrix coherence = computeCoherence(request, uBaseline, vBaseline, major.getValue(), minor.getValue(), phi.getValue());

    // Allocate the result.
    MeqJonesResult result(request.nspid());
    MeqResult &resXX = result.result11();
    MeqResult &resXY = result.result12();
    MeqResult &resYX = result.result21();
    MeqResult &resYY = result.result22();
    
    resXX.setValue((iScaled + qScaled) * coherence);
    resXY.setValue(uvScaled * coherence);
    resYX.setValue(conj(uvScaled) * coherence);
    resYY.setValue((iScaled - qScaled) * coherence);
    
    // Evaluate (if needed) for the perturbed parameter values.
    const MeqParmFunklet* perturbedParm = 0;
    MeqMatrix coherencePerturbed;
    MeqMatrix iScaledPerturbed, qScaledPerturbed, uvScaledPerturbed;

    for(int spinx = 0; spinx < request.nspid(); ++spinx)
    {
        bool evalCoherence = false;
        bool evalIQ = false;
        bool evalUV = false;
        
        coherencePerturbed = coherence;
        iScaledPerturbed = iScaled;
        qScaledPerturbed = qScaled;
        uvScaledPerturbed = uvScaled;

        if(major.isDefined(spinx))
        {
            evalCoherence = true;
            perturbedParm = major.getPerturbedParm (spinx);
        }
        else if(minor.isDefined(spinx))
        {
            evalCoherence = true;
            perturbedParm = minor.getPerturbedParm (spinx);
        }
        else if(phi.isDefined(spinx))
        {
            evalCoherence = true;
            perturbedParm = phi.getPerturbedParm (spinx);
        }
        
        if(ik.isDefined(spinx))
        {
            evalIQ = true;
            perturbedParm = ik.getPerturbedParm (spinx);
        }
        else if(qk.isDefined(spinx))
        {
            evalIQ = true;
            perturbedParm = qk.getPerturbedParm (spinx);
        }
    
        if(uk.isDefined(spinx))
        {
            evalUV = true;
            perturbedParm = uk.getPerturbedParm (spinx);
        }
        else if(vk.isDefined(spinx))
        {
            evalUV = true;
            perturbedParm = vk.getPerturbedParm (spinx);
        }
        
        if(evalCoherence)
        {
            coherencePerturbed = computeCoherence(request, uBaseline, vBaseline,
                major.getPerturbedValue(spinx),
                minor.getPerturbedValue(spinx),
                phi.getPerturbedValue(spinx));
        }

        if(evalIQ)
        {
            iScaledPerturbed = ik.getPerturbedValue(spinx) * 0.5;
            qScaledPerturbed = qk.getPerturbedValue(spinx) * 0.5;
        }

        if(evalUV)
        {
            uvScaledPerturbed = tocomplex(uk.getPerturbedValue(spinx),
                vk.getPerturbedValue(spinx)) * 0.5;
        }

        if(evalCoherence || evalIQ)
        {        
            resXX.setPerturbedParm(spinx, perturbedParm);
            resXX.setPerturbedValue(spinx,
                (iScaledPerturbed + qScaledPerturbed) * coherencePerturbed);
                
            resYY.setPerturbedParm(spinx, perturbedParm);
            resYY.setPerturbedValue(spinx,
                (iScaledPerturbed - qScaledPerturbed) * coherencePerturbed);
        }

        if(evalCoherence || evalUV)
        {
            resXY.setPerturbedParm(spinx, perturbedParm);
            resXY.setPerturbedValue(spinx, uvScaledPerturbed * coherencePerturbed);
                
            resYX.setPerturbedParm(spinx, perturbedParm);
            resYX.setPerturbedValue(spinx, conj(uvScaledPerturbed) * coherencePerturbed);
        }
    }

    return result;
}


MeqMatrix MeqGaussianCoherency::computeCoherence(const MeqRequest &request,
    const MeqMatrix &uBaseline, const MeqMatrix &vBaseline,
    const MeqMatrix &major, const MeqMatrix &minor, const MeqMatrix &phi)
{
    // Compute dot product of rotated, scaled uv-vector with itself (1D in time).
    MeqMatrix cosPhi(cos(phi));
    MeqMatrix sinPhi(sin(phi));
    MeqMatrix uvTransformedDotProduct =
        sqr(major * (uBaseline * cosPhi - vBaseline * sinPhi))
        + sqr(minor * (uBaseline * sinPhi + vBaseline * cosPhi));

    // Allocate the result matrix (2D).
    MeqMatrix result;
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
std::string MeqGaussianCoherency::getLabel()
{
    return std::string("MeqGaussianCoherency\\nSpatial coherence function of"
        " an elliptical gaussian source\\n" + itsSource->getName() + " ("
        + itsSource->getGroupName() + ")");
}
#endif

} // namespace BBS
} // namespace LOFAR
