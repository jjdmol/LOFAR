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

Mul::Mul()
    :   ExprStatic<Mul::N_Inputs>()
{
}    


ValueSet::ConstPtr Mul::evaluateImpl(const Request &request,
    const ValueSet::ConstPtr (&inputs)[Mul::N_Inputs]) const
{    
    ValueSet::ConstPtr lhs = inputs[LHS];
    ValueSet::ConstPtr rhs = inputs[RHS];
    
    // TODO: generalize this.
    ASSERT(lhs->rank() == 0 && lhs->size() == 1);
    ASSERT(rhs->rank() == 2 && rhs->shape(0) == 2 && rhs->shape(1) == 2);

    ValueSet::Ptr result(new ValueSet(2, 2));
    result->assign(0, 0, lhs->value() * rhs->value(0, 0));
    result->assign(1, 0, lhs->value() * rhs->value(1, 0));
    result->assign(0, 1, lhs->value() * rhs->value(0, 1));
    result->assign(1, 1, lhs->value() * rhs->value(1, 1));

//    cout << "LHS: " << lhs->value()(0, 0) << endl;
//    cout << "RHS: " << rhs->value(0, 0)(0, 0) << endl;

    return result;
}

} // namespace BBS
} // namespace LOFAR
