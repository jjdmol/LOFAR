//# JonesMul2.h: Calculate A * B.
//#
//# Copyright (C) 2002
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

#ifndef EXPR_JONESMUL2_H
#define EXPR_JONESMUL2_H

// \file
// Calculate A * B.

#include <BBSKernel/Expr/JonesExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Calculate A * B.
class JonesMul2: public JonesExprRep
{
public:
    JonesMul2(const JonesExpr &left, const JonesExpr &right);
    ~JonesMul2();

    // Get the result of the expression for the given domain.
    JonesResult getJResult(const Request &request);

private:
    JonesExpr   itsLeft, itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
