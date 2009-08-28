//# BasicExpr.h: Base classes for unary, binary, and ternary expressions that
//# transparently handle the computation of perturbed values.
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

#ifndef LOFAR_BBSKERNEL_EXPR_BASICEXPR_H
#define LOFAR_BBSKERNEL_EXPR_BASICEXPR_H

// \file
// Base classes for unary, binary, and ternary expressions that transparently
// handle the computation of perturbed values.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

template <typename T_ARG0, typename T_EXPR_VALUE>
class BasicUnaryExpr: public UnaryExpr<T_ARG0, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicUnaryExpr>          Ptr;
    typedef shared_ptr<const BasicUnaryExpr>    ConstPtr;

    using UnaryExpr<T_ARG0, T_EXPR_VALUE>::argument0;

    BasicUnaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0);

protected:
    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const;

    virtual const typename T_EXPR_VALUE::View evaluateImpl
        (const Request &request, const typename T_ARG0::View &arg0) const = 0;
};

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
class BasicBinaryExpr: public BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicBinaryExpr>         Ptr;
    typedef shared_ptr<const BasicBinaryExpr>   ConstPtr;

    using BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::argument0;
    using BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::argument1;

    BasicBinaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1);

protected:
    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const;

    virtual const typename T_EXPR_VALUE::View evaluateImpl
        (const Request &request, const typename T_ARG0::View &arg0,
        const typename T_ARG1::View &arg1) const = 0;
};

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
class BasicTernaryExpr: public TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicTernaryExpr>        Ptr;
    typedef shared_ptr<const BasicTernaryExpr>  ConstPtr;

    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument0;
    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument1;
    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument2;

    BasicTernaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2);

protected:
    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const;

    virtual const typename T_EXPR_VALUE::View evaluateImpl
        (const Request &request, const typename T_ARG0::View &arg0,
        const typename T_ARG1::View &arg1, const typename T_ARG1::View &arg2)
        const = 0;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: BasicUnaryExpr                                         - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_EXPR_VALUE>
BasicUnaryExpr<T_ARG0, T_EXPR_VALUE>::BasicUnaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0)
    :   UnaryExpr<T_ARG0, T_EXPR_VALUE>(arg0)
{
}

template <typename T_ARG0, typename T_EXPR_VALUE>
const T_EXPR_VALUE BasicUnaryExpr<T_ARG0, T_EXPR_VALUE>::evaluateExpr
    (const Request &request, Cache &cache) const
{
    T_EXPR_VALUE result;

    // Evaluate arguments.
    const T_ARG0 arg0 = argument0()->evaluate(request, cache);

    // Evaluate flags.
    result.setFlags(arg0.flags());

    // Compute main value.
    result.assign(evaluateImpl(request, arg0.view()));

    // Compute perturbed values.
    typename T_ARG0::Iterator it0(arg0);
    while(!it0.atEnd())
    {
        result.assign(it0.key(), evaluateImpl(request, it0.value()));
        it0.advance();
    }

    return result;
}

// -------------------------------------------------------------------------- //
// - Implementation: BasicBinaryExpr                                        - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
BasicBinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::BasicBinaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1)
    :   BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>(arg0, arg1)
{
}

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
const T_EXPR_VALUE BasicBinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::evaluateExpr
    (const Request &request, Cache &cache) const
{
    // Allocate result.
    T_EXPR_VALUE result;

    // Evaluate arguments.
    const T_ARG0 arg0 = argument0()->evaluate(request, cache);
    const T_ARG1 arg1 = argument1()->evaluate(request, cache);

    // Evaluate flags.
    FlagArray flags[2];
    flags[0] = arg0.flags();
    flags[1] = arg1.flags();
    result.setFlags(mergeFlags(flags, flags + 2));

    // Compute main value.
    result.assign(evaluateImpl(request, arg0.view(), arg1.view()));

    // Compute perturbed values.
    typename T_ARG0::Iterator it0(arg0);
    typename T_ARG1::Iterator it1(arg1);

    PValueKey key;
    while(!(it0.atEnd() && it1.atEnd()))
    {
        key = std::min(it0.key(), it1.key());
        result.assign(key, evaluateImpl(request, it0.value(key),
            it1.value(key)));
        it0.advance(key);
        it1.advance(key);
    }

    return result;
}

// -------------------------------------------------------------------------- //
// - Implementation: BasicTernaryExpr                                       - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
BasicTernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::BasicTernaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2)
    :   TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>(arg0, arg1, arg2)
{
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
const T_EXPR_VALUE BasicTernaryExpr<T_ARG0, T_ARG1, T_ARG2,
    T_EXPR_VALUE>::evaluateExpr(const Request &request, Cache &cache) const
{
    // Allocate result.
    T_EXPR_VALUE result;

    // Evaluate arguments.
    const T_ARG0 arg0 = argument0()->evaluate(request, cache);
    const T_ARG1 arg1 = argument1()->evaluate(request, cache);
    const T_ARG2 arg2 = argument2()->evaluate(request, cache);

    // Evaluate flags.
    FlagArray flags[3];
    flags[0] = arg0.flags();
    flags[1] = arg1.flags();
    flags[2] = arg2.flags();
    result.setFlags(mergeFlags(flags, flags + 3));

    // Compute main value.
    result.assign(evaluateImpl(request, arg0.view(), arg1.view(), arg2.view()));

    // Compute perturbed values.
    typename T_ARG0::Iterator it0(arg0);
    typename T_ARG1::Iterator it1(arg1);
    typename T_ARG2::Iterator it2(arg2);

    PValueKey key;
    while(!(it0.atEnd() && it1.atEnd() && it2.atEnd()))
    {
        key = std::min(std::min(it0.key(), it1.key()), it2.key());
        result.assign(key, evaluateImpl(request, it0.value(key),
            it1.value(key), it2.value(key)));
        it0.advance(key);
        it1.advance(key);
        it2.advance(key);
    }

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
