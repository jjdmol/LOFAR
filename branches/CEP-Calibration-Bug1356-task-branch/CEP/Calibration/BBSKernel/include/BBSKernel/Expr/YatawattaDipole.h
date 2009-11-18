//# YatawattaDipole.h: Dipole voltage beam using Sarod Yatawatta's analytical
//# model.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_EXPR_YATAWATTADIPOLE_H
#define LOFAR_BBSKERNEL_EXPR_YATAWATTADIPOLE_H

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Expr/ExternalFunction.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class YatawattaDipole: public BasicBinaryExpr<Vector<2>, Scalar, JonesMatrix>
{
public:
    YatawattaDipole(const string &moduleTheta, const string &modulePhi,
        double scaleFactor, const Expr<Vector<2> >::ConstPtr &azel,
        const Expr<Scalar>::ConstPtr &orientation);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const Vector<2>::View &azel, const Scalar::View &orientation) const;

private:
    ExternalFunction    itsThetaFunction, itsPhiFunction;
    double              itsScaleFactor;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
