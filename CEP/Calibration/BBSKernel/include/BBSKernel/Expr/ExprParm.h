//# ExprParm.h: Parameter that can be used in an expression.
//#
//# Copyright (C) 2008
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

#ifndef EXPR_EXPRPARM_H
#define EXPR_EXPRPARM_H

// \file
// Parameter that can be used in an expression.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/ParmProxy.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class ExprParm: public ExprRep
{
public:
    ExprParm(const ParmProxy::ConstPointer &parm);
    ~ExprParm();
    
    void setPValueFlag();
    bool getPValueFlag() const
    { return itsPValueFlag; }
    void clearPValueFlag();
    
    // Compute a result for the given request.
    Result getResult(const Request &request);

private:
    ExprParm(const ExprParm &other);
    ExprParm &operator=(const ExprParm &other);

    ParmProxy::ConstPointer itsParm;
    bool                    itsPValueFlag;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
