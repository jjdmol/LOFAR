//# MeqPointCoherency.h: Spatial coherence function of a point source.
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
//#include <Common/Timer.h>

#include <BBSKernel/MNS/MeqPointCoherency.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqPointCoherency::MeqPointCoherency(const MeqPointSource *source)
    :   itsSource(source)
{
  addChild (itsSource->getI());
  addChild (itsSource->getQ());
  addChild (itsSource->getU());
  addChild (itsSource->getV());
}


MeqPointCoherency::~MeqPointCoherency()
{
}


MeqJonesResult MeqPointCoherency::getJResult(const MeqRequest &request)
{
    //static NSTimer timer("MeqPointCoherency::getResult", true);
    //timer.start();

    // Allocate the result.
    MeqJonesResult result(request.nspid());
    MeqResult& resXX = result.result11();
    MeqResult& resXY = result.result12();
    MeqResult& resYX = result.result21();
    MeqResult& resYY = result.result22();
    
    // Calculate the source fluxes.
    MeqResult ikBuf, qkBuf, ukBuf, vkBuf;
    const MeqResult& ik = itsSource->getI().getResultSynced (request, ikBuf);
    const MeqResult& qk = itsSource->getQ().getResultSynced (request, qkBuf);
    const MeqResult& uk = itsSource->getU().getResultSynced (request, ukBuf);
    const MeqResult& vk = itsSource->getV().getResultSynced (request, vkBuf);
    
    // Calculate the XX values, etc.
    MeqMatrix uvk_2 = tocomplex(uk.getValue(), vk.getValue()) * 0.5;
    MeqMatrix ik_2 = ik.getValue() * 0.5;
    MeqMatrix qk_2 = qk.getValue() * 0.5;

    resXX.setValue (ik_2 + qk_2);
    resXY.setValue (uvk_2);
    resYX.setValue (conj(uvk_2));
    resYY.setValue (ik_2 - qk_2);
    
    // Evaluate (if needed) for the perturbed parameter values.
    const MeqParmFunklet* perturbedParm;
    for(int spinx=0; spinx<request.nspid(); spinx++)
    {
        bool eval1 = false;
        bool eval2 = false;
        
        if(ik.isDefined(spinx))
        {
            eval1 = true;
            perturbedParm = ik.getPerturbedParm (spinx);
        }
        else if(qk.isDefined(spinx))
        {
            eval1 = true;
            perturbedParm = qk.getPerturbedParm (spinx);
        }
    
        if(uk.isDefined(spinx))
        {
            eval2 = true;
            perturbedParm = uk.getPerturbedParm (spinx);
        }
        else if(vk.isDefined(spinx))
        {
            eval2 = true;
            perturbedParm = vk.getPerturbedParm (spinx);
        }
        
        if(eval1)
        {
            const MeqMatrix& ikp_2 = ik.getPerturbedValue(spinx) * 0.5;
            const MeqMatrix& qkp_2 = qk.getPerturbedValue(spinx) * 0.5;
            
            resXX.setPerturbedParm (spinx, perturbedParm);
            resXX.setPerturbedValue(spinx, (ikp_2 + qkp_2));
                
            resYY.setPerturbedParm (spinx, perturbedParm);
            resYY.setPerturbedValue(spinx, (ikp_2 - qkp_2));
        }

        if(eval2)
        {
            MeqMatrix uvkp_2 = tocomplex(uk.getPerturbedValue(spinx),
                vk.getPerturbedValue(spinx)) * 0.5;

            resXY.setPerturbedParm(spinx, perturbedParm);
            resXY.setPerturbedValue(spinx, uvkp_2);
                
            resYX.setPerturbedParm (spinx, perturbedParm);
            resYX.setPerturbedValue(spinx, conj(uvkp_2));
        }
    }

    //timer.stop();
    return result;
}

#ifdef EXPR_GRAPH
std::string MeqPointCoherency::getLabel()
{
    return std::string("MeqPointCoherency\\nSpatial coherence function of a"
        " point source\\n" + itsSource->getName() + " ("
        + itsSource->getGroupName() + ")");
}
#endif

} // namespace BBS
} // namespace LOFAR
