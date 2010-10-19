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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRPARM_H
#define LOFAR_BBSKERNEL_EXPR_EXPRPARM_H

// \file
// Parameter that can be used in an expression.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/ParmProxy.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ExprParm: public NullaryExpr<Scalar>
{
public:
    typedef shared_ptr<ExprParm>        Ptr;
    typedef shared_ptr<const ExprParm>  ConstPtr;

    ExprParm(const ParmProxy::ConstPtr &parm);

    void setPValueFlag();
    bool getPValueFlag() const;
    void clearPValueFlag();

protected:
    virtual const Scalar evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    ParmProxy::ConstPtr itsParm;
    bool                itsPValueFlag;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
