//# PhaseShift.h: Phase delay due to baseline geometry with respect to source
//#     direction.
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

// \ingroup BBSKernel
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
    #ifdef EXPR_GRAPH
    virtual std::string getLabel();
    #endif

    Expr itsLeft;
    Expr itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
