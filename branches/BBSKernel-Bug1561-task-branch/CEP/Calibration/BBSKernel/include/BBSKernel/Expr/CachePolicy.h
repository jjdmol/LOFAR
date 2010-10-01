//# CachePolicy.h: Assign sensible cache behaviour settings to all Expr
//# instances of an ExprSet.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_CACHEPOLICY_H
#define LOFAR_BBSKERNEL_EXPR_CACHEPOLICY_H

// \file
// Assign sensible cache behaviour settings to all Expr instances of an ExprSet.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class CachePolicy
{
public:
    typedef shared_ptr<CachePolicy>         Ptr;
    typedef shared_ptr<const CachePolicy>   ConstPtr;

    template <typename T_ITER>
    void apply(T_ITER first, T_ITER last) const;

protected:
    virtual void applyImpl(const ExprBase::ConstPtr &root) const = 0;
};

class DefaultCachePolicy: public CachePolicy
{
public:
    typedef shared_ptr<DefaultCachePolicy>          Ptr;
    typedef shared_ptr<const DefaultCachePolicy>    ConstPtr;

protected:
    virtual void applyImpl(const ExprBase::ConstPtr &root) const;

    void cacheSharedExpr(const ExprBase::ConstPtr &root) const;
};

class ExperimentalCachePolicy: public DefaultCachePolicy
{
public:
    typedef shared_ptr<ExperimentalCachePolicy>         Ptr;
    typedef shared_ptr<const ExperimentalCachePolicy>   ConstPtr;

protected:
    virtual void applyImpl(const ExprBase::ConstPtr &root) const;

    void cacheIndepExpr(const ExprBase::ConstPtr &root) const;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: CachePolicy                                            - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
void CachePolicy::apply(T_ITER first, T_ITER last) const
{
    while(first != last)
    {
        applyImpl(*first);
        ++first;
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
