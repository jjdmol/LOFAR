//# YatawattaDipole.h: Dipole voltage beam using Sarod Yatawatta's analytical
//# model.
//#
//# Copyright (C) 2008
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

#ifndef EXPR_YATAWATTADIPOLE_H
#define EXPR_YATAWATTADIPOLE_H

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/ExternalFunction.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class YatawattaDipole: public JonesExprRep
{
public:
    YatawattaDipole(const string &moduleTheta, const string &modulePhi,
        const Expr &azel, const Expr &orientation, double scaleFactor);

    virtual JonesResult getJResult(const Request &request);

private:
    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, const Matrix &in_orientation,
        Matrix &out_E11, Matrix &out_E12,
        Matrix &out_E21, Matrix &out_E22);


    ExternalFunction    itsThetaFunction, itsPhiFunction;
    double              itsScaleFactor;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
