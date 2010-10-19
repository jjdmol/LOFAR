//# PhaseShiftMS.cc: Phase delay due to baseline geometry with respect to
//#     source direction.
//#
//# Copyright (C) 2009
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

#include <BBSKernel/Expr/Mul.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

Shape Mul::shape(const ExprValueSet (&arguments)[Mul::N_Arguments])
    const
{
    DBGASSERT(arguments[LHS].shape() == Shape()
        && arguments[RHS].shape() == Shape(2, 2));
    return Shape(2, 2);
}

void Mul::evaluateImpl(const Request &request,
    const ExprValue (&arguments)[Mul::N_Arguments], ExprValue &result) const
{
    result.assign(0, 0, arguments[LHS]() * arguments[RHS](0, 0));
    result.assign(0, 1, arguments[LHS]() * arguments[RHS](0, 1));
    result.assign(1, 0, arguments[LHS]() * arguments[RHS](1, 0));
    result.assign(1, 1, arguments[LHS]() * arguments[RHS](1, 1));

//    LOG_DEBUG_STR("LHS: " << arguments[LHS]().getDComplex());
//    LOG_DEBUG_STR("RHS: " << arguments[RHS](0, 0).getDComplex() << arguments[RHS](0, 1).getDComplex() << arguments[RHS](1, 0).getDComplex() << arguments[RHS](1, 1).getDComplex());
}

} // namespace BBS
} // namespace LOFAR
