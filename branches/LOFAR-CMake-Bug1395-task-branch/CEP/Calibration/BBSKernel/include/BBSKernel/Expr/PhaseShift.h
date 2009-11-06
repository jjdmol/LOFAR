//# PhaseShift.h: Phase delay due to baseline geometry with respect to source
//# direction.
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

#ifndef EXPR_PHASESHIFT_H
#define EXPR_PHASESHIFT_H

// \file
// Phase delay due to baseline geometry with respect to source direction.

//# Includes
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class PhaseShift: public ExprRep
{
public:
    PhaseShift (const Expr& left, const Expr& right);
    ~PhaseShift();

    // Calculate the results for the given domain.
    virtual Result getResult(const Request &request);

private:

    Expr itsLeft;
    Expr itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
