//# JonesCMul3.h: Calculate A * B * C^H (the conjugate transpose of C).
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

#ifndef EXPR_JONESCMUL3_H
#define EXPR_JONESCMUL3_H

// \file
// Calculate A * B * C^H (the conjugate transpose of C).

//# Includes
#include <BBSKernel/Expr/JonesExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Calculate A * B * C^H (the conjugate transpose of C).
class JonesCMul3: public JonesExprRep
{
public:
    JonesCMul3(const JonesExpr &left, const JonesExpr &mid,
        const JonesExpr &right);
    ~JonesCMul3();

    // Get the result of the expression for the given domain.
    JonesResult getJResult (const Request&);

private:

    JonesExpr itsLeft, itsMid, itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR


#endif
