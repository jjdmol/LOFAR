//# JonesCMul2.cc: Calculate A * B^H (the conjugate transpose of B).
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

#include <BBSKernel/Expr/JonesCMul2.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

JonesCMul2::JonesCMul2(const JonesExpr &left, const JonesExpr &right)
    :   itsLeft(left),
        itsRight(right)
{
    addChild(itsLeft);
    addChild(itsRight);
}

JonesCMul2::~JonesCMul2()
{
}

JonesResult JonesCMul2::getJResult(const Request &request)
{
    // Create the result object.
    JonesResult result;
    result.init();

    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();

    // Evaluate the children.
    JonesResult tmpLeft, tmpRight;
    const JonesResult &left  = itsLeft.getResultSynced(request, tmpLeft);
    const JonesResult &right = itsRight.getResultSynced(request, tmpRight);

    const Result &l11 = left.getResult11();
    const Result &l12 = left.getResult12();
    const Result &l21 = left.getResult21();
    const Result &l22 = left.getResult22();
    const Result &r11 = right.getResult11();
    const Result &r12 = right.getResult12();
    const Result &r21 = right.getResult21();
    const Result &r22 = right.getResult22();

    const Matrix &ml11 = l11.getValue();
    const Matrix &ml12 = l12.getValue();
    const Matrix &ml21 = l21.getValue();
    const Matrix &ml22 = l22.getValue();
    const Matrix &mr11 = r11.getValue();
    const Matrix &mr12 = r12.getValue();
    const Matrix &mr21 = r21.getValue();
    const Matrix &mr22 = r22.getValue();

    // Compute the main value.
    result11.setValue(ml11 * conj(mr11) + ml12 * conj(mr12));
    result12.setValue(ml11 * conj(mr21) + ml12 * conj(mr22));
    result21.setValue(ml21 * conj(mr11) + ml22 * conj(mr12));
    result22.setValue(ml21 * conj(mr21) + ml22 * conj(mr22));

    // Compute perturbed values.
    enum PValues
    {
        PV_LEFT11, PV_LEFT12, PV_LEFT21, PV_LEFT22,
        PV_RIGHT11, PV_RIGHT12, PV_RIGHT21, PV_RIGHT22,
        N_PValues
    };
    
    const Result *pvSet[N_PValues] = {&l11, &l12, &l21, &l22, &r11, &r12, &r21,
        &r22};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {  
        bool eval11, eval12, eval21, eval22;
        eval11 = eval12 = eval21 = eval22 = false;

        if(pvIter.hasPValue(PV_LEFT11) || pvIter.hasPValue(PV_LEFT12))
        {
            eval11 = eval12 = true;
        }
        
        if(pvIter.hasPValue(PV_LEFT21) || pvIter.hasPValue(PV_LEFT22))
        {
            eval21 = eval22 = true;
        }
        
        if(pvIter.hasPValue(PV_RIGHT11) || pvIter.hasPValue(PV_RIGHT12))
        {
            eval11 = eval21 = true;
        }
        
        if(pvIter.hasPValue(PV_RIGHT21) || pvIter.hasPValue(PV_RIGHT22))
        {
            eval12 = eval22 = true;
        }
         
        const Matrix &ml11 = pvIter.value(PV_LEFT11);
        const Matrix &ml12 = pvIter.value(PV_LEFT12);
        const Matrix &ml21 = pvIter.value(PV_LEFT21);
        const Matrix &ml22 = pvIter.value(PV_LEFT22);
        const Matrix &mr11 = pvIter.value(PV_RIGHT11);
        const Matrix &mr12 = pvIter.value(PV_RIGHT12);
        const Matrix &mr21 = pvIter.value(PV_RIGHT21);
        const Matrix &mr22 = pvIter.value(PV_RIGHT22);

        if(eval11)
        {
            result11.setPerturbedValue(pvIter.key(), ml11 * conj(mr11)
                + ml12 * conj(mr12));
        }
        
        if(eval12)
        {
            result12.setPerturbedValue(pvIter.key(), ml11 * conj(mr21)
                + ml12 * conj(mr22));
        }

        if(eval21)
        {
            result21.setPerturbedValue(pvIter.key(), ml21 * conj(mr11)
                + ml22 * conj(mr12));
        }
        
        if(eval22)
        {
            result22.setPerturbedValue(pvIter.key(), ml21 * conj(mr21)
                + ml22 * conj(mr22));
        }

        pvIter.next();
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
