//# ExprParm.h: Parameter that can be used in an expression.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRPARM_H
#define LOFAR_BBSKERNEL_EXPR_EXPRPARM_H

// \file
// Parameter that can be used in an expression.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <BBSKernel/ParmProxy.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ExprParm: public Expr<Scalar>
{
public:
    typedef shared_ptr<ExprParm>        Ptr;
    typedef shared_ptr<const ExprParm>  ConstPtr;

    ExprParm(const ParmProxy::ConstPtr &parm);

    void setPValueFlag();
    bool getPValueFlag() const
    { return itsPValueFlag; }
    void clearPValueFlag();

protected:
    virtual unsigned int nArguments() const
    {
        return 0;
    }

    virtual ExprBase::ConstPtr argument(unsigned int) const
    {
        ASSERT(false);
    }

private:
    virtual void updateSolvables(set<PValueKey> &solvables) const;

    virtual const Scalar evaluateExpr(const Request &request, Cache &cache)
        const;

    ParmProxy::ConstPtr itsParm;
    bool                itsPValueFlag;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
