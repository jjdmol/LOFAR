//# MergeFlags.h: Merge flags of the right hand side with the left hand side.
//# The data is not merged. Only the data of the left hand side is passed
//# through.
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

#ifndef LOFAR_BBSKERNEL_EXPR_MERGEFLAGS_H
#define LOFAR_BBSKERNEL_EXPR_MERGEFLAGS_H

// \file
// Merge flags of the right hand side with the left hand side. The data is not
// merged. Only the data of the left hand side is passed through.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

template <typename T_ARG0, typename T_ARG1>
class MergeFlags: public BinaryExpr<T_ARG0, T_ARG1, T_ARG0>
{
public:
    typedef shared_ptr<MergeFlags>          Ptr;
    typedef shared_ptr<const MergeFlags>    ConstPtr;

    using BinaryExpr<T_ARG0, T_ARG1, T_ARG0>::argument0;
    using BinaryExpr<T_ARG0, T_ARG1, T_ARG0>::argument1;

    MergeFlags(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1)
        :   BinaryExpr<T_ARG0, T_ARG1, T_ARG0>(arg0, arg1)
    {
    }

    virtual const T_ARG0 evaluateExpr(const Request &request,
        Cache &cache) const
    {
        // Allocate result.
        T_ARG0 result;

        // Evaluate arguments.
        const T_ARG0 arg0 = argument0()->evaluate(request, cache);
        const T_ARG1 arg1 = argument1()->evaluate(request, cache);

        if(arg1.hasFlags())
        {
            // Pass through value.
            for(unsigned int i = 0; i < arg0.size(); ++i)
            {
                result.setValueSet(i, arg0.getValueSet(i));
            }

            // Merge flags.
            result.setFlags(arg0.hasFlags() ? arg0.flags() | arg1.flags()
                : arg1.flags());
        }
        else
        {
            result = arg0;
        }

        return result;
    }
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
