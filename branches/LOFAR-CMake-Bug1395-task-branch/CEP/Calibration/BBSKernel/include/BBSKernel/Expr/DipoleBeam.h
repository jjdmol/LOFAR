//# DipoleBeam.h: Dipole voltage beam (analytic)
//#
//# Copyright (C) 2007
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

#ifndef EXPR_DIPOLEBEAM_H
#define EXPR_DIPOLEBEAM_H

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class DipoleBeam: public JonesExprRep
{
public:
    enum
    {
        IN_AZEL,
        N_InputPort
    } InputPort;

    DipoleBeam(Expr azel, double height = 1.706, double length = 1.38,
        double slant = casa::C::pi / 4.001, double orientation = 0.0);

    virtual JonesResult getJResult(const Request &request);

private:
    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, Matrix &out_E_theta, Matrix &out_E_phi, 
        double height, double length, double slant, double orientation);


    double      itsHeight, itsLength, itsSlant, itsOrientation;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
