//# Expr.h: Expression base class
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPR_H
#define LOFAR_BBSKERNEL_EXPR_EXPR_H

// \file
// Expression base class

#include <Common/lofar_smartptr.h>
#include <Common/lofar_set.h>

#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <BBSKernel/Expr/Cache.h>

// TODO: Reduction functions like MatrixSum
// TODO: Element-wise functions like MatrixSum

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Expression identifier (used for caching).
typedef size_t ExprId;

// Helper function to compute the bitwise or of all the FlagArray instances
// in the range [it, end).
template <typename T_ITER>
FlagArray mergeFlags(T_ITER it, T_ITER end)
{
    FlagArray result;

    // Skip until a non-empty FlagArray is encountered.
    while(it != end && !it->initialized())
    {
        ++it;
    }

    // Merge (bitwise or) all non-empty FlagArray's.
    if(it != end)
    {
        ASSERT(it->initialized());
        result = it->clone();
        ++it;

        while(it != end)
        {
            if(it->initialized())
            {
                result |= *it;
            }
            ++it;
        }
    }

    return result;
}

// Expression base class.
class ExprBase
{
public:
    typedef shared_ptr<ExprBase>                Ptr;
    typedef shared_ptr<const ExprBase>          ConstPtr;

    typedef vector<PValueKey>::const_iterator   const_solvable_iterator;

    ExprBase()
        :   itsConsumerCount(0),
            itsId(theirId++)
    {
    }

    virtual ~ExprBase()
    {
    }

    // Return the unique id of this expression.
    ExprId id() const
    {
        return itsId;
    }

    // Recurse the expression tree to determine which solvable coefficients this
    // expression depends on. This information is cached internally.
    void updateSolvables() const
    {
        set<PValueKey> solvables;
        updateSolvables(solvables);
    }

    // Get iterators to iterate the solvable coefficients this expression
    // depends on.
    //
    // @{
    const_solvable_iterator begin() const
    {
        return itsSolvables.begin();
    }

    const_solvable_iterator end() const
    {
        return itsSolvables.end();
    }
    // @}

protected:
    // Connect/disconnect expressions to/from this expression. The connected
    // expressions constitute this expression's arguments.
    //
    // Currently, these methods only keep track of the number of consumers of
    // the result of an expression. This number is used to decide whether or not
    // to cache a result.
    //
    // @{
    void connect(const ExprBase::ConstPtr &arg) const
    {
        arg->incConsumerCount();
    }

    void disconnect(const ExprBase::ConstPtr &arg) const
    {
        arg->decConsumerCount();
    }

    unsigned int nConsumers() const
    {
        return itsConsumerCount;
    }
    // @}

    // Provide access to the arguments of an expression. Because the type of the
    // arguments must be kept, they are stored in the derived classes (where the
    // type is known). However, there the base class has methods (such as
    // updateSolvables()) that need access to the arguments (as
    // ExprBase::ConstPtr objects).
    //
    // @{
    virtual unsigned int nArguments() const = 0;
    virtual ExprBase::ConstPtr argument(unsigned int i) const = 0;
    // @}

    // Recurse the expression tree to determine which solvable coefficients this
    // Expr depends on. This information is cached internally.
    virtual void updateSolvables(set<PValueKey> &solvables) const
    {
        set<PValueKey> tmp;
        for(unsigned int i = 0; i < nArguments(); ++i)
        {
            argument(i)->updateSolvables(tmp);
        }

        itsSolvables.clear();
        itsSolvables.insert(itsSolvables.begin(), tmp.begin(), tmp.end());
        solvables.insert(tmp.begin(), tmp.end());
    }

    mutable vector<PValueKey>   itsSolvables;

private:
    // Forbid copy and assignment as it is probably not needed. Should it be
    // needed in the future, care has to be taken that the consumer count is
    // _not_ copied. Also, any derived classes should be checked for data
    // members that should not be copied.
    ExprBase(const ExprBase &);
    ExprBase &operator=(const ExprBase &);

    // Change the number of consumers (i.e. expressions that depend on the
    // result of this expression).
    // @{
    void incConsumerCount() const
    {
        ++itsConsumerCount;
    }

    void decConsumerCount() const
    {
        --itsConsumerCount;
    }
    // @}

    // The number of registered consumers. This attribute is mutable such that
    // an expression tree can be constructed without requiring non-const access
    // to the constituent nodes (derivatives of ExprBase).
    //
    // The consumer count is used to decide whether or not to cache a result. As
    // caching is an implementation detail, anything related to it should not be
    // detectable outside the ExprBase hierarchy.
    mutable unsigned int        itsConsumerCount;

    ExprId                      itsId;
    static ExprId               theirId;
};

template <typename T_EXPR_VALUE>
class Expr: public ExprBase
{
public:
    typedef shared_ptr<Expr<T_EXPR_VALUE> >         Ptr;
    typedef shared_ptr<const Expr<T_EXPR_VALUE> >   ConstPtr;

    typedef T_EXPR_VALUE    ExprValueType;

    using ExprBase::id;
    using ExprBase::nConsumers;

    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache)
        const
    {
        T_EXPR_VALUE result;

        // Query the cache.
        if(nConsumers() < 1 || !cache.query(id(), request.id(), result))
        {
            // Compute the result.
            result = evaluateExpr(request, cache);

            // Insert into cache (only if it will be re-used later on).
            if(nConsumers() > 1)
            {
                cache.insert(id(), request.id(), result);
            }
        }

        return result;
    }

    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const = 0;
};

template <typename T_ARG0, typename T_EXPR_VALUE>
class UnaryExpr: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<UnaryExpr>       Ptr;
    typedef shared_ptr<const UnaryExpr> ConstPtr;

    typedef T_ARG0  Arg0Type;

    using ExprBase::connect;
    using ExprBase::disconnect;

    UnaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0)
        :   itsArg0(arg0)
    {
        connect(itsArg0);
    }

    ~UnaryExpr()
    {
        disconnect(itsArg0);
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 1;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        ASSERTSTR(i == 0, "Invalid argument index specified.");
        return itsArg0;
    }

    const typename Expr<T_ARG0>::ConstPtr &argument0() const
    {
        return itsArg0;
    }

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
};

template <typename T_ARG0, typename T_EXPR_VALUE>
class BasicUnaryExpr: public UnaryExpr<T_ARG0, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicUnaryExpr>          Ptr;
    typedef shared_ptr<const BasicUnaryExpr>    ConstPtr;

    using ExprBase::begin;
    using ExprBase::end;
    using UnaryExpr<T_ARG0, T_EXPR_VALUE>::argument0;

    BasicUnaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0)
        :   UnaryExpr<T_ARG0, T_EXPR_VALUE>(arg0)
    {
    }

    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const
    {
        T_EXPR_VALUE result;

        // Evaluate arguments.
        const T_ARG0 arg0 = argument0()->evaluate(request, cache);

        // Evaluate flags.
        result.setFlags(arg0.flags());

        // Compute main value.
        result.assign(evaluateImpl(request, arg0.value()));

        // Compute perturbed values.
        ExprBase::const_solvable_iterator it = begin();
        while(it != end())
        {
            result.assign(*it, evaluateImpl(request, arg0.value(*it)));
            ++it;
        }

        return result;
    }

protected:
    virtual const typename T_EXPR_VALUE::view evaluateImpl
        (const Request &request, const typename T_ARG0::view &arg0) const = 0;
};

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
class BinaryExpr: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BinaryExpr>          Ptr;
    typedef shared_ptr<const BinaryExpr>    ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;

    using ExprBase::connect;
    using ExprBase::disconnect;

    BinaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1)
        :   itsArg0(arg0),
            itsArg1(arg1)
    {
        connect(itsArg0);
        connect(itsArg1);
    }

    ~BinaryExpr()
    {
        disconnect(itsArg0);
        disconnect(itsArg1);
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 2;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        switch(i)
        {
            case 0:
                return itsArg0;
            case 1:
                return itsArg1;
            default:
                ASSERTSTR(false, "Invalid argument index specified.");
        }
    }

    const typename Expr<T_ARG0>::ConstPtr &argument0() const
    {
        return itsArg0;
    }

    const typename Expr<T_ARG1>::ConstPtr &argument1() const
    {
        return itsArg1;
    }

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
};

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
class BasicBinaryExpr: public BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicBinaryExpr>         Ptr;
    typedef shared_ptr<const BasicBinaryExpr>   ConstPtr;

    using ExprBase::begin;
    using ExprBase::end;
    using BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::argument0;
    using BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::argument1;

    BasicBinaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1)
        :   BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>(arg0, arg1)
    {
    }

    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const
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
        result.assign(evaluateImpl(request, arg0.value(), arg1.value()));

        // Compute perturbed values.
        ExprBase::const_solvable_iterator it = begin();
        while(it != end())
        {
            result.assign(*it, evaluateImpl(request, arg0.value(*it),
                arg1.value(*it)));
            ++it;
        }

        return result;
    }

protected:
    virtual const typename T_EXPR_VALUE::view evaluateImpl
        (const Request &request, const typename T_ARG0::view &arg0,
        const typename T_ARG1::view &arg1) const = 0;
};

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
class TernaryExpr: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<TernaryExpr>         Ptr;
    typedef shared_ptr<const TernaryExpr>   ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;
    typedef T_ARG2  Arg2Type;

    using ExprBase::connect;
    using ExprBase::disconnect;

    TernaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2)
        :   itsArg0(arg0),
            itsArg1(arg1),
            itsArg2(arg2)
    {
        connect(itsArg0);
        connect(itsArg1);
        connect(itsArg2);
    }

    ~TernaryExpr()
    {
        disconnect(itsArg0);
        disconnect(itsArg1);
        disconnect(itsArg2);
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 3;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        switch(i)
        {
            case 0:
                return itsArg0;
            case 1:
                return itsArg1;
            case 2:
                return itsArg2;
            default:
                ASSERTSTR(false, "Invalid argument index specified.");
        }
    }

    const typename Expr<T_ARG0>::ConstPtr &argument0() const
    {
        return itsArg0;
    }

    const typename Expr<T_ARG1>::ConstPtr &argument1() const
    {
        return itsArg1;
    }

    const typename Expr<T_ARG2>::ConstPtr &argument2() const
    {
        return itsArg2;
    }

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
    typename Expr<T_ARG2>::ConstPtr itsArg2;
};

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
class BasicTernaryExpr: public TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>
{
public:
    typedef shared_ptr<BasicTernaryExpr>        Ptr;
    typedef shared_ptr<const BasicTernaryExpr>  ConstPtr;

    using ExprBase::begin;
    using ExprBase::end;
    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument0;
    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument1;
    using TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::argument2;

    BasicTernaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2)
        :   TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>(arg0, arg1, arg2)
    {
    }

    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache) const
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
        result.assign(evaluateImpl(request, arg0.value(), arg1.value(),
            arg2.value()));

        // Compute perturbed values.
        ExprBase::const_solvable_iterator it = begin();
        while(it != end())
        {
            result.assign(*it, evaluateImpl(request, arg0.value(*it),
                arg1.value(*it), arg2.value(*it)));
            ++it;
        }

        return result;
    }

protected:
    virtual const typename T_EXPR_VALUE::view evaluateImpl
        (const Request &request, const typename T_ARG0::view &arg0,
        const typename T_ARG1::view &arg1, const typename T_ARG1::view &arg2)
        const = 0;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
