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

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class JonesInvert: public BasicUnaryExpr<JonesMatrix, JonesMatrix>
{
public:
    typedef shared_ptr<JonesInvert>         Ptr;
    typedef shared_ptr<const JonesInvert>   ConstPtr;

    JonesInvert(const Expr<JonesMatrix>::ConstPtr &expr);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const JonesMatrix::View &arg0) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
