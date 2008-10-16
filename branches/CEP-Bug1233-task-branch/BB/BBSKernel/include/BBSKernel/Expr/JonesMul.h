//# JonesMul.h: Multiply each component of a JonesExpr with a single
//#     Expr.
//#
//# Copyright (C) 2007
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

#ifndef EXPR_JONESMUL_H
#define EXPR_JONESMUL_H

// \file Multiply each component of a JonesExpr with a single Expr.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/JonesResult.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup Expr
// @{


// Multiply each component of a JonesExpr with a single Expr.

class JonesMul: public JonesExprRep
{
public:
    JonesMul(const JonesExpr &left, const Expr &right);
    ~JonesMul();

    // Get the result of the expression for the given domain.
    JonesResult getJResult(const Request &request);

private:
#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    JonesExpr	itsLeft;
    Expr       itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
