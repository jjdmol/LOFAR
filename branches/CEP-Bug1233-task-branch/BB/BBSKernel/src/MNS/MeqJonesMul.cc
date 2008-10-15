//# JonesMul.h: Multiply each component of a JonesExpr with a single
//#     Expr.
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
#include <BBSKernel/MNS/PValueIterator.h>

namespace LOFAR
{
namespace BBS
{

JonesMul::JonesMul(const JonesExpr &left, const Expr &right)
    :    itsLeft(left),
         itsRight(right)
{
    addChild(left);
    addChild(right);
}


JonesMul::~JonesMul()
{
}


JonesResult JonesMul::getJResult(const Request &request)
{
    // Create the result object.
    JonesResult result;
    result.init();
    
    Result& result11 = result.result11();
    Result& result12 = result.result12();
    Result& result21 = result.result21();
    Result& result22 = result.result22();
    
    // Evaluate the children.
    JonesResult leftResult;
    Result rightResult;

    const JonesResult &left = itsLeft.getResultSynced(request, leftResult);
    const Result &right = itsRight.getResultSynced(request, rightResult);
        
    const Result &l11 = left.getResult11();
    const Result &l12 = left.getResult12();
    const Result &l21 = left.getResult21();
    const Result &l22 = left.getResult22();
    
    const Matrix &ml11 = l11.getValue();
    const Matrix &ml12 = l12.getValue();
    const Matrix &ml21 = l21.getValue();
    const Matrix &ml22 = l22.getValue();

    const Matrix &mr = right.getValue();

    // Compute result.
    result11.setValue(ml11 * mr);
    result12.setValue(ml12 * mr);
    result21.setValue(ml21 * mr);
    result22.setValue(ml22 * mr);

    // Compute perturbed values.
    enum PValues
    { PV_LEFT11, PV_LEFT12, PV_LEFT21, PV_LEFT22, PV_RIGHT, N_PValues };

    const Result *pvSet[N_PValues] = {&l11, &l12, &l21, &l22, &right};    
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &ml11 = pvIter.value(PV_LEFT11);
        const Matrix &ml12 = pvIter.value(PV_LEFT12);
        const Matrix &ml21 = pvIter.value(PV_LEFT21);
        const Matrix &ml22 = pvIter.value(PV_LEFT22);
        const Matrix &mr = pvIter.value(PV_RIGHT);
            
        if(pvIter.hasPValue(PV_RIGHT))
        {
            result11.setPerturbedValue(pvIter.key(), ml11 * mr);
            result12.setPerturbedValue(pvIter.key(), ml12 * mr);
            result21.setPerturbedValue(pvIter.key(), ml21 * mr);
            result22.setPerturbedValue(pvIter.key(), ml22 * mr);
        }
        else
        {
            if(pvIter.hasPValue(PV_LEFT11))
            {
                result11.setPerturbedValue(pvIter.key(), ml11 * mr);
            }
            
            if(pvIter.hasPValue(PV_LEFT12))
            {
                result12.setPerturbedValue(pvIter.key(), ml12 * mr);
            }

            if(pvIter.hasPValue(PV_LEFT21))
            {
                result21.setPerturbedValue(pvIter.key(), ml21 * mr);
            }

            if(pvIter.hasPValue(PV_LEFT22))
            {
                result21.setPerturbedValue(pvIter.key(), ml22 * mr);
            }
        }
        
        pvIter.next();
    }

    return result;
}


#ifdef EXPR_GRAPH
virtual std::string JonesMul::getLabel()
{
    return std::string("JonesMul\\n"
        "Multiply JonesExpr with a single Expr");
}
#endif


} // namespace BBS
} // namespace LOFAR
