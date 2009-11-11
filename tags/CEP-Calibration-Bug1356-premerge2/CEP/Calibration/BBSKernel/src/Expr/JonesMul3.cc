//# JonesMul3.cc: Calculate A * B * C.
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

#include <BBSKernel/Expr/JonesMul3.h>
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

JonesMul3::JonesMul3(const JonesExpr &left, const JonesExpr &mid,
    const JonesExpr &right)
    :   itsLeft(left),
        itsMid(mid),
        itsRight(right)
{
    addChild (itsLeft);
    addChild (itsMid);
    addChild (itsRight);
}

JonesMul3::~JonesMul3()
{
}

JonesResult JonesMul3::getJResult(const Request &request)
{
    // Create the result object.
    JonesResult result;
    result.init();

    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();

    // Evaluate the children.
    JonesResult tmpLeft, tmpMid, tmpRight;
    const JonesResult &left = itsLeft.getResultSynced(request, tmpLeft);
    const JonesResult &mid = itsMid.getResultSynced(request, tmpMid);
    const JonesResult &right = itsRight.getResultSynced(request, tmpRight);
    
    const Result &l11 = left.getResult11();
    const Result &l12 = left.getResult12();
    const Result &l21 = left.getResult21();
    const Result &l22 = left.getResult22();
    const Result &m11 = mid.getResult11();
    const Result &m12 = mid.getResult12();
    const Result &m21 = mid.getResult21();
    const Result &m22 = mid.getResult22();
    const Result &r11 = right.getResult11();
    const Result &r12 = right.getResult12();
    const Result &r21 = right.getResult21();
    const Result &r22 = right.getResult22();
    
    const Matrix &ml11 = l11.getValue();
    const Matrix &ml12 = l12.getValue();
    const Matrix &ml21 = l21.getValue();
    const Matrix &ml22 = l22.getValue();
    const Matrix &mm11 = m11.getValue();
    const Matrix &mm12 = m12.getValue();
    const Matrix &mm21 = m21.getValue();
    const Matrix &mm22 = m22.getValue();
    const Matrix &mr11 = r11.getValue();
    const Matrix &mr12 = r12.getValue();
    const Matrix &mr21 = r21.getValue();
    const Matrix &mr22 = r22.getValue();
    
    // Compute main value.
    Matrix t11(ml11 * mm11 + ml12 * mm21);
    Matrix t12(ml11 * mm12 + ml12 * mm22);
    Matrix t21(ml21 * mm11 + ml22 * mm21);
    Matrix t22(ml21 * mm12 + ml22 * mm22);
    
    result11.setValue(t11 * mr11 + t12 * mr21);
    result12.setValue(t11 * mr12 + t12 * mr22);
    result21.setValue(t21 * mr11 + t22 * mr21);
    result22.setValue(t21 * mr12 + t22 * mr22);

    // Compute perturbed values.
    enum PValues
    {
        PV_LEFT11, PV_LEFT12, PV_LEFT21, PV_LEFT22,
        PV_MID11, PV_MID12, PV_MID21, PV_MID22,
        PV_RIGHT11, PV_RIGHT12, PV_RIGHT21, PV_RIGHT22,
        N_PValues
    };

    const Result *pvSet[N_PValues] = {&l11, &l12, &l21, &l22, &m11, &m12, &m21,
        &m22, &r11, &r12, &r21, &r22};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {  
        bool eval11, eval12, eval21, eval22;
        eval11 = eval12 = eval21 = eval22 = false;

        if(pvIter.hasPValue(PV_MID11) || pvIter.hasPValue(PV_MID12)
            || pvIter.hasPValue(PV_MID21) || pvIter.hasPValue(PV_MID22))
        {
            eval11 = eval12 = eval21 = eval22 = true;
        }
        else
        {
            if(pvIter.hasPValue(PV_LEFT11) || pvIter.hasPValue(PV_LEFT12))
            {
                eval11 = eval12 = true;
            }
            
            if(pvIter.hasPValue(PV_LEFT21) || pvIter.hasPValue(PV_LEFT22))
            {
                eval21 = eval22 = true;
            }
            
            if(pvIter.hasPValue(PV_RIGHT11) || pvIter.hasPValue(PV_RIGHT21))
            {
                eval11 = eval21 = true;
            }
            
            if(pvIter.hasPValue(PV_RIGHT12) || pvIter.hasPValue(PV_RIGHT22))
            {
                eval12 = eval22 = true;
            }
        }
         
        const Matrix &ml11 = pvIter.value(PV_LEFT11);
        const Matrix &ml12 = pvIter.value(PV_LEFT12);
        const Matrix &ml21 = pvIter.value(PV_LEFT21);
        const Matrix &ml22 = pvIter.value(PV_LEFT22);
        const Matrix &mm11 = pvIter.value(PV_MID11);
        const Matrix &mm12 = pvIter.value(PV_MID12);
        const Matrix &mm21 = pvIter.value(PV_MID21);
        const Matrix &mm22 = pvIter.value(PV_MID22);
        const Matrix &mr11 = pvIter.value(PV_RIGHT11);
        const Matrix &mr12 = pvIter.value(PV_RIGHT12);
        const Matrix &mr21 = pvIter.value(PV_RIGHT21);
        const Matrix &mr22 = pvIter.value(PV_RIGHT22);

        if(eval11 || eval12)
        {
            Matrix t11(ml11 * mm11 + ml12 * mm21);
            Matrix t12(ml11 * mm12 + ml12 * mm22);
          
            if(eval11)
            {
                result11.setPerturbedValue(pvIter.key(), t11 * mr11
                    + t12 * mr21);
            }
        
            if(eval12)
            {
                result12.setPerturbedValue(pvIter.key(), t11 * mr12
                    + t12 * mr22);
            }
        }

        if(eval21 || eval22)
        {
            Matrix t21(ml21 * mm11 + ml22 * mm21);
            Matrix t22(ml21 * mm12 + ml22 * mm22);

            if(eval21)
            {
                result21.setPerturbedValue(pvIter.key(), t21 * mr11
                    + t22 * mr21);
            }
        
            if(eval22)
            {
                result22.setPerturbedValue(pvIter.key(), t21 * mr12
                    + t22 * mr22);
            }
        }

        pvIter.next();
    } // while(!atEnd)
    
    return result;
}

} // namespace BBS
} // namespace LOFAR
