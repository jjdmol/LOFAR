//# CondNumberFlagger.cc: Flag the result of an Expr<JonesMatrix> by
//# thresholding on the condition number of the Jones matrices.
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
#include <BBSKernel/Expr/ConditionNumber.h>

namespace LOFAR
{
namespace BBS
{

ConditionNumber::ConditionNumber(const Expr<JonesMatrix>::ConstPtr &arg0)
    :   BasicUnaryExpr<JonesMatrix, Scalar>(arg0)
{
}

const Scalar::View ConditionNumber::evaluateImpl(const Request &request,
    const JonesMatrix::View &arg0) const
{
    Matrix norm00 = abs(arg0(0, 0));
    Matrix norm11 = abs(arg0(1, 1));

    Scalar::View result;
    result.assign(max(norm00, norm11) / max(min(norm00, norm11), 1e-9));
    LOG_DEBUG_STR("Condition#: " << result());
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
