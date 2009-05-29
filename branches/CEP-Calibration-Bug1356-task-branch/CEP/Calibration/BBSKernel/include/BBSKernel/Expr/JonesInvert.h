//# JonesInvert.h: The inverse of a Jones matrix expression.
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_BBS_EXPR_JONESINVERT_H
#define LOFAR_BBS_EXPR_JONESINVERT_H

// \file
// The inverse of a Jones matrix expression.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class JonesInvert : public Expr1<JonesMatrix, JonesMatrix>
{
public:
    typedef shared_ptr<JonesInvert> Ptr;
    typedef shared_ptr<JonesInvert> ConstPtr;

    JonesInvert(const Expr<JonesMatrix>::ConstPtr &expr)
        :   Expr1<JonesMatrix, JonesMatrix>(expr)
    {
    }

private:
    virtual const JonesMatrix::proxy evaluateImpl(const Request &request,
        const JonesMatrix::proxy &arg0) const
    {
        JonesMatrix::proxy result;

        Matrix invDet(1. / (arg0(0, 0) * arg0(1, 1) - arg0(0, 1) * arg0(1, 0)));
        result.assign(0, 0, arg0(1, 1) * invDet);
        result.assign(0, 1, arg0(0, 1) * -invDet);
        result.assign(1, 0, arg0(1, 0) * -invDet);
        result.assign(1, 1, arg0(0, 0) * invDet);

        return result;
    }
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
