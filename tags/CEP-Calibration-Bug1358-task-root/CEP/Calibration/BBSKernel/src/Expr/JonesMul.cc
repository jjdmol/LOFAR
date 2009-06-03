//# JonesMul.h: Calculate c * A, where c is an Expr and A is a JonesExpr.
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

#include <BBSKernel/Expr/JonesMul.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

namespace LOFAR
{
namespace BBS
{

JonesMul::JonesMul(const Expr &left, const JonesExpr &right)
    :    itsLeft(left),
         itsRight(right)
{
    addChild(itsLeft);
    addChild(itsRight);
}

JonesMul::~JonesMul()
{
}

JonesResult JonesMul::getJResult(const Request &request)
{
    // Create the result object.
    JonesResult result;
    result.init();

    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();

    // Evaluate the children.
    Result tmpLeft;
    JonesResult tmpRight;
    const Result &left = itsLeft.getResultSynced(request, tmpLeft);
    const JonesResult &right = itsRight.getResultSynced(request, tmpRight);

    const Result &r11 = right.getResult11();
    const Result &r12 = right.getResult12();
    const Result &r21 = right.getResult21();
    const Result &r22 = right.getResult22();

    const Matrix &mr11 = r11.getValue();
    const Matrix &mr12 = r12.getValue();
    const Matrix &mr21 = r21.getValue();
    const Matrix &mr22 = r22.getValue();

    const Matrix &ml = left.getValue();

    // Compute result.
    result11.setValue(ml * mr11);
    result12.setValue(ml * mr12);
    result21.setValue(ml * mr21);
    result22.setValue(ml * mr22);

    // Compute perturbed values.
    enum PValues
    { PV_LEFT, PV_RIGHT11, PV_RIGHT12, PV_RIGHT21, PV_RIGHT22, N_PValues };

    const Result *pvSet[N_PValues] = {&left, &r11, &r12, &r21, &r22};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &ml = pvIter.value(PV_LEFT);
        const Matrix &mr11 = pvIter.value(PV_RIGHT11);
        const Matrix &mr12 = pvIter.value(PV_RIGHT12);
        const Matrix &mr21 = pvIter.value(PV_RIGHT21);
        const Matrix &mr22 = pvIter.value(PV_RIGHT22);

        if(pvIter.hasPValue(PV_LEFT))
        {
            result11.setPerturbedValue(pvIter.key(), ml * mr11);
            result12.setPerturbedValue(pvIter.key(), ml * mr12);
            result21.setPerturbedValue(pvIter.key(), ml * mr21);
            result22.setPerturbedValue(pvIter.key(), ml * mr22);
        }
        else
        {
            if(pvIter.hasPValue(PV_RIGHT11))
            {
                result11.setPerturbedValue(pvIter.key(), ml * mr11);
            }

            if(pvIter.hasPValue(PV_RIGHT12))
            {
                result12.setPerturbedValue(pvIter.key(), ml * mr12);
            }

            if(pvIter.hasPValue(PV_RIGHT21))
            {
                result21.setPerturbedValue(pvIter.key(), ml * mr21);
            }

            if(pvIter.hasPValue(PV_RIGHT22))
            {
                result22.setPerturbedValue(pvIter.key(), ml * mr22);
            }
        }

        pvIter.next();
    }

    return result;
}

#ifdef EXPR_GRAPH
virtual std::string JonesMul::getLabel()
{
    return std::string("JonesMul\\nMultiply an Expr with a JonesExpr");
}
#endif

} // namespace BBS
} // namespace LOFAR
