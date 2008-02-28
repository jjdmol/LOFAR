//# MeqJonesMul.h: Multiply each component of a MeqJonesExpr with a single
//#     MeqExpr.
//#
//# Copyright (C) 2007
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

#include <BBSKernel/MNS/MeqJonesMul.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

MeqJonesMul::MeqJonesMul(const MeqJonesExpr &left, const MeqExpr &right)
    :    itsLeft(left),
         itsRight(right)
{
    addChild(left);
    addChild(right);
}


MeqJonesMul::~MeqJonesMul()
{
}


MeqJonesResult MeqJonesMul::getJResult(const MeqRequest &request)
{
    // Create the result object.
    MeqJonesResult result(request.nspid());
    MeqResult& result11 = result.result11();
    MeqResult& result12 = result.result12();
    MeqResult& result21 = result.result21();
    MeqResult& result22 = result.result22();
    
    // Evaluate the children.
    MeqJonesResult leftResult;
    MeqResult rightResult;

    const MeqJonesResult &left = itsLeft.getResultSynced(request, leftResult);
    const MeqResult &right = itsRight.getResultSynced(request, rightResult);
        
    const MeqResult &l11 = left.getResult11();
    const MeqResult &l12 = left.getResult12();
    const MeqResult &l21 = left.getResult21();
    const MeqResult &l22 = left.getResult22();
    
    const MeqMatrix &ml11 = l11.getValue();
    const MeqMatrix &ml12 = l12.getValue();
    const MeqMatrix &ml21 = l21.getValue();
    const MeqMatrix &ml22 = l22.getValue();

    const MeqMatrix &mr = right.getValue();

    // Compute result.
    result11.setValue(ml11 * mr);
    result12.setValue(ml12 * mr);
    result21.setValue(ml21 * mr);
    result22.setValue(ml22 * mr);

    // Compute perturbed values.
    for (int spinx=0; spinx<request.nspid(); spinx++)
    {
        if(right.isDefined(spinx))
        {
            const MeqMatrix &mr = right.getPerturbedValue(spinx);
            const MeqMatrix &ml11 = l11.getPerturbedValue(spinx);
            const MeqMatrix &ml12 = l12.getPerturbedValue(spinx);
            const MeqMatrix &ml21 = l21.getPerturbedValue(spinx);
            const MeqMatrix &ml22 = l22.getPerturbedValue(spinx);

            const MeqParmFunklet *perturbedParm = right.getPerturbedParm(spinx);
            result11.setPerturbedParm(spinx, perturbedParm);
            result11.setPerturbedValue(spinx, ml11 * mr);
            result12.setPerturbedParm(spinx, perturbedParm);
            result12.setPerturbedValue(spinx, ml12 * mr);
            result21.setPerturbedParm(spinx, perturbedParm);
            result21.setPerturbedValue(spinx, ml21 * mr);
            result22.setPerturbedParm(spinx, perturbedParm);
            result22.setPerturbedValue(spinx, ml22 * mr);
        }
        else
        {        
            if(l11.isDefined(spinx))
            {
                const MeqMatrix &mr = right.getPerturbedValue(spinx);
                const MeqMatrix &ml11 = l11.getPerturbedValue(spinx);

                result11.setPerturbedParm(spinx, l11.getPerturbedParm(spinx));
                result11.setPerturbedValue(spinx, ml11 * mr);
            }
            
            if(l12.isDefined(spinx))
            {
                const MeqMatrix &mr = right.getPerturbedValue(spinx);
                const MeqMatrix &ml12 = l12.getPerturbedValue(spinx);

                result12.setPerturbedParm(spinx, l12.getPerturbedParm(spinx));
                result12.setPerturbedValue(spinx, ml12 * mr);
            }

            if(l21.isDefined(spinx))
            {
                const MeqMatrix &mr = right.getPerturbedValue(spinx);
                const MeqMatrix &ml21 = l21.getPerturbedValue(spinx);

                result21.setPerturbedParm(spinx, l21.getPerturbedParm(spinx));
                result21.setPerturbedValue(spinx, ml21 * mr);
            }

            if(l22.isDefined(spinx))
            {
                const MeqMatrix &mr = right.getPerturbedValue(spinx);
                const MeqMatrix &ml22 = l22.getPerturbedValue(spinx);

                result22.setPerturbedParm(spinx, l22.getPerturbedParm(spinx));
                result22.setPerturbedValue(spinx, ml22 * mr);
            }
        }
    }

    return result;
}


#ifdef EXPR_GRAPH
virtual std::string MeqJonesMul::getLabel()
{
    return std::string("MeqJonesMul\\n"
        "Multiply MeqJonesExpr with a single MeqExpr");
}
#endif


} // namespace BBS
} // namespace LOFAR
