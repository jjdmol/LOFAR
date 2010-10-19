//# JonesMul.h: Calculate c * A, where c is an Expr and A is a JonesExpr.
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

#ifndef EXPR_JONESMUL_H
#define EXPR_JONESMUL_H

// \file
// Calculate c * A, where c is an Expr and A is a JonesExpr.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/JonesResult.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Calculate c * A, where c is an Expr and A is a JonesExpr.
class JonesMul: public JonesExprRep
{
public:
    JonesMul(const Expr &left, const JonesExpr &right);
    ~JonesMul();

    // Get the result of the expression for the given domain.
    JonesResult getJResult(const Request &request);

private:

    Expr        itsLeft;
    JonesExpr   itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
