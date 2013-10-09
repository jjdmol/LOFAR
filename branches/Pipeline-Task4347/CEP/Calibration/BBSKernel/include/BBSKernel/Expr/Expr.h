//# Expr.h: Expression base class
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPR_H
#define LOFAR_BBSKERNEL_EXPR_EXPR_H

// \file
// Expression base class

#include <Common/lofar_smartptr.h>
#include <Common/lofar_set.h>

#include <BBSKernel/Expr/ExprId.h>
#include <BBSKernel/Expr/ExprValue.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Cache.h>
#include <BBSKernel/Expr/Timer.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// Expression base class.
class ExprBase
{
public:
    typedef shared_ptr<ExprBase>                Ptr;
    typedef shared_ptr<const ExprBase>          ConstPtr;

    ExprBase();
    virtual ~ExprBase();

    // Return the unique id of this expression.
    ExprId id() const;

    // how many nodes depend on the result of this node (important for caching)
    unsigned int nConsumers() const;

    // depends on a parameter that is solved for
    virtual bool isDependent() const;

    void setCachePolicy(Cache::Policy policy) const;
    Cache::Policy getCachePolicy() const;

    // \name Argument access
    // Provide access to the arguments of an expression. Because the type of the
    // arguments must be kept, they are stored in the derived classes (where the
    // type is known). However, access from the base class can be useful to
    // support visitors or base class functions that need to recurse the tree.
    //
    // @{
    virtual unsigned int nArguments() const = 0;
    virtual ExprBase::ConstPtr argument(unsigned int i) const = 0;
    // @}

protected:
    // \name Connection management
    // Connect/disconnect expressions to/from this expression. The connected
    // expressions constitute this expression's arguments. Currently, these
    // methods only keep track of the number of consumers of the result of an
    // expression. This number is used to decide whether or not to cache a
    // result.
    //
    // @{
    void connect(const ExprBase::ConstPtr &arg) const;
    void disconnect(const ExprBase::ConstPtr &arg) const;
    // @}

private:
    // Forbid copy and assignment.
    ExprBase(const ExprBase &);
    ExprBase &operator=(const ExprBase &);

    // @{
    // Change the number of consumers (i.e. expressions that depend on the
    // result of this expression).
    void incConsumerCount() const;
    void decConsumerCount() const;
    // @}

    // The number of registered consumers. This attribute is mutable such that
    // an expression tree can be constructed without requiring non-const access
    // to the constituent nodes (derivatives of ExprBase).
    //
    // The consumer count is used to decide whether or not to cache a result. As
    // caching is an implementation detail, anything related to it should not be
    // detectable outside the ExprBase hierarchy.
    mutable unsigned int        itsConsumerCount;

    mutable Cache::Policy       itsCachePolicy;

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

    // request can contain multiple grids, in practice almost always 0
    // takes care of caching, calls evaluateExpr
    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache,
        unsigned int grid) const;

protected:
    virtual const T_EXPR_VALUE evaluateExpr(const Request &request,
        Cache &cache, unsigned int grid) const = 0;
};

template <typename T_EXPR_VALUE>
class NullaryExpr: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<NullaryExpr>         Ptr;
    typedef shared_ptr<const NullaryExpr>   ConstPtr;

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int) const;
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

    UnaryExpr(const typename Expr<T_ARG0>::ConstPtr &arg0);
    ~UnaryExpr();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    const typename Expr<T_ARG0>::ConstPtr &argument0() const;

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
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
        const typename Expr<T_ARG1>::ConstPtr &arg1);
    ~BinaryExpr();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    const typename Expr<T_ARG0>::ConstPtr &argument0() const;
    const typename Expr<T_ARG1>::ConstPtr &argument1() const;

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
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
        const typename Expr<T_ARG2>::ConstPtr &arg2);
    ~TernaryExpr();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    const typename Expr<T_ARG0>::ConstPtr &argument0() const;
    const typename Expr<T_ARG1>::ConstPtr &argument1() const;
    const typename Expr<T_ARG2>::ConstPtr &argument2() const;

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
    typename Expr<T_ARG2>::ConstPtr itsArg2;
};

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
    typename T_EXPR_VALUE>
class Expr4: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<Expr4>       Ptr;
    typedef shared_ptr<const Expr4> ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;
    typedef T_ARG2  Arg2Type;
    typedef T_ARG3  Arg3Type;

    using ExprBase::connect;
    using ExprBase::disconnect;

    Expr4(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2,
        const typename Expr<T_ARG3>::ConstPtr &arg3);
    ~Expr4();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    const typename Expr<T_ARG0>::ConstPtr &argument0() const;
    const typename Expr<T_ARG1>::ConstPtr &argument1() const;
    const typename Expr<T_ARG2>::ConstPtr &argument2() const;
    const typename Expr<T_ARG3>::ConstPtr &argument3() const;

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
    typename Expr<T_ARG2>::ConstPtr itsArg2;
    typename Expr<T_ARG3>::ConstPtr itsArg3;
};

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
    typename T_ARG4, typename T_EXPR_VALUE>
class Expr5: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<Expr5>       Ptr;
    typedef shared_ptr<const Expr5> ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;
    typedef T_ARG2  Arg2Type;
    typedef T_ARG3  Arg3Type;
    typedef T_ARG4  Arg4Type;

    using ExprBase::connect;
    using ExprBase::disconnect;

    Expr5(const typename Expr<T_ARG0>::ConstPtr &arg0,
        const typename Expr<T_ARG1>::ConstPtr &arg1,
        const typename Expr<T_ARG2>::ConstPtr &arg2,
        const typename Expr<T_ARG3>::ConstPtr &arg3,
        const typename Expr<T_ARG4>::ConstPtr &arg4);
    ~Expr5();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    const typename Expr<T_ARG0>::ConstPtr &argument0() const;
    const typename Expr<T_ARG1>::ConstPtr &argument1() const;
    const typename Expr<T_ARG2>::ConstPtr &argument2() const;
    const typename Expr<T_ARG3>::ConstPtr &argument3() const;
    const typename Expr<T_ARG4>::ConstPtr &argument4() const;

private:
    typename Expr<T_ARG0>::ConstPtr itsArg0;
    typename Expr<T_ARG1>::ConstPtr itsArg1;
    typename Expr<T_ARG2>::ConstPtr itsArg2;
    typename Expr<T_ARG3>::ConstPtr itsArg3;
    typename Expr<T_ARG4>::ConstPtr itsArg4;
};

// Helper function to compute the bitwise or of all the FlagArray instances
// in the range [it, end).
template <typename T_ITER>
FlagArray mergeFlags(T_ITER it, T_ITER end);

// @}


// -------------------------------------------------------------------------- //
// - Implementation: ExprBase                                               - //
// -------------------------------------------------------------------------- //

inline ExprId ExprBase::id() const
{
    return itsId;
}

inline unsigned int ExprBase::nConsumers() const
{
    return itsConsumerCount;
}

inline void ExprBase::setCachePolicy(Cache::Policy policy) const
{
    itsCachePolicy = policy;
}

inline Cache::Policy ExprBase::getCachePolicy() const
{
    return itsCachePolicy;
}

// -------------------------------------------------------------------------- //
// - Implementation: Expr                                                   - //
// -------------------------------------------------------------------------- //

template <typename T_EXPR_VALUE>
inline const T_EXPR_VALUE  Expr<T_EXPR_VALUE>::evaluate(const Request &request,
    Cache &cache, unsigned int grid) const
{
    T_EXPR_VALUE result;

    if(getCachePolicy() == Cache::NONE || !cache.query(id(), request.id(),
        result))
    {
        // Compute the result.
        result = evaluateExpr(request, cache, grid);

        // Insert into cache (only if it can potentially be re-used later on).
        if(getCachePolicy() != Cache::NONE)
        {
            cache.insert(id(), request.id(), getCachePolicy(), result);
        }
    }

    return result;
}

// -------------------------------------------------------------------------- //
// - Implementation: NullaryExpr                                            - //
// -------------------------------------------------------------------------- //

template <typename T_EXPR_VALUE>
inline unsigned int NullaryExpr<T_EXPR_VALUE>::nArguments() const
{
    return 0;
}

template <typename T_EXPR_VALUE>
inline ExprBase::ConstPtr NullaryExpr<T_EXPR_VALUE>::argument(unsigned int)
    const
{
    ASSERTSTR(false, "NullaryExpr does not have arguments.");
}

// -------------------------------------------------------------------------- //
// - Implementation: UnaryExpr                                              - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_EXPR_VALUE>
UnaryExpr<T_ARG0, T_EXPR_VALUE>::UnaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0)
    :   itsArg0(arg0)
{
    connect(itsArg0);
}

template <typename T_ARG0, typename T_EXPR_VALUE>
UnaryExpr<T_ARG0, T_EXPR_VALUE>::~UnaryExpr()
{
    disconnect(itsArg0);
}

template <typename T_ARG0, typename T_EXPR_VALUE>
inline unsigned int UnaryExpr<T_ARG0, T_EXPR_VALUE>::nArguments() const
{
    return 1;
}

template <typename T_ARG0, typename T_EXPR_VALUE>
inline ExprBase::ConstPtr UnaryExpr<T_ARG0, T_EXPR_VALUE>::argument
    (unsigned int i) const
{
    ASSERTSTR(i == 0, "Invalid argument index specified.");
    return itsArg0;
}

template <typename T_ARG0, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG0>::ConstPtr&
UnaryExpr<T_ARG0, T_EXPR_VALUE>::argument0() const
{
    return itsArg0;
}

// -------------------------------------------------------------------------- //
// - Implementation: BinaryExpr                                             - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::BinaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
    const typename Expr<T_ARG1>::ConstPtr &arg1)
    :   itsArg0(arg0),
        itsArg1(arg1)
{
    connect(itsArg0);
    connect(itsArg1);
}

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::~BinaryExpr()
{
    disconnect(itsArg1);
    disconnect(itsArg0);
}

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
inline unsigned int BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::nArguments() const
{
    return 2;
}

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
inline ExprBase::ConstPtr BinaryExpr<T_ARG0, T_ARG1, T_EXPR_VALUE>::argument
    (unsigned int i) const
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

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG0>::ConstPtr &BinaryExpr<T_ARG0, T_ARG1,
    T_EXPR_VALUE>::argument0() const
{
    return itsArg0;
}

template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG1>::ConstPtr&BinaryExpr<T_ARG0, T_ARG1,
    T_EXPR_VALUE>::argument1() const
{
    return itsArg1;
}

// -------------------------------------------------------------------------- //
// - Implementation: TernaryExpr                                            - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::TernaryExpr
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
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

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
TernaryExpr<T_ARG0, T_ARG1, T_ARG2, T_EXPR_VALUE>::~TernaryExpr()
{
    disconnect(itsArg2);
    disconnect(itsArg1);
    disconnect(itsArg0);
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
inline unsigned int TernaryExpr<T_ARG0, T_ARG1, T_ARG2,
    T_EXPR_VALUE>::nArguments() const
{
    return 3;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
inline ExprBase::ConstPtr TernaryExpr<T_ARG0, T_ARG1, T_ARG2,
    T_EXPR_VALUE>::argument(unsigned int i) const
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

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
inline const typename Expr<T_ARG0>::ConstPtr &TernaryExpr<T_ARG0, T_ARG1,
    T_ARG2, T_EXPR_VALUE>::argument0() const
{
    return itsArg0;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
inline const typename Expr<T_ARG1>::ConstPtr &TernaryExpr<T_ARG0, T_ARG1,
    T_ARG2, T_EXPR_VALUE>::argument1() const
{
    return itsArg1;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2,
    typename T_EXPR_VALUE>
inline const typename Expr<T_ARG2>::ConstPtr &TernaryExpr<T_ARG0, T_ARG1,
    T_ARG2, T_EXPR_VALUE>::argument2() const
{
    return itsArg2;
}

// -------------------------------------------------------------------------- //
// - Implementation: Expr4                                                  - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
Expr4<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_EXPR_VALUE>::Expr4
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
    const typename Expr<T_ARG1>::ConstPtr &arg1,
    const typename Expr<T_ARG2>::ConstPtr &arg2,
    const typename Expr<T_ARG3>::ConstPtr &arg3)
    :   itsArg0(arg0),
        itsArg1(arg1),
        itsArg2(arg2),
        itsArg3(arg3)
{
    connect(itsArg0);
    connect(itsArg1);
    connect(itsArg2);
    connect(itsArg3);
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
Expr4<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_EXPR_VALUE>::~Expr4()
{
    disconnect(itsArg3);
    disconnect(itsArg2);
    disconnect(itsArg1);
    disconnect(itsArg0);
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
inline unsigned int Expr4<T_ARG0, T_ARG1, T_ARG2, T_ARG3,
    T_EXPR_VALUE>::nArguments() const
{
    return 4;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
    typename T_EXPR_VALUE>
inline ExprBase::ConstPtr Expr4<T_ARG0, T_ARG1, T_ARG2, T_ARG3,
    T_EXPR_VALUE>::argument(unsigned int i) const
{
    switch(i)
    {
        case 0:
            return itsArg0;
        case 1:
            return itsArg1;
        case 2:
            return itsArg2;
        case 3:
            return itsArg3;
        default:
            ASSERTSTR(false, "Invalid argument index specified.");
    }
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
inline const typename Expr<T_ARG0>::ConstPtr &Expr4<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_EXPR_VALUE>::argument0() const
{
    return itsArg0;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
inline const typename Expr<T_ARG1>::ConstPtr &Expr4<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_EXPR_VALUE>::argument1() const
{
    return itsArg1;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_EXPR_VALUE>
inline const typename Expr<T_ARG2>::ConstPtr &Expr4<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_EXPR_VALUE>::argument2() const
{
    return itsArg2;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
    typename T_EXPR_VALUE>
inline const typename Expr<T_ARG3>::ConstPtr &Expr4<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_EXPR_VALUE>::argument3() const
{
    return itsArg3;
}

// -------------------------------------------------------------------------- //
// - Implementation: Expr5                                                  - //
// -------------------------------------------------------------------------- //

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
Expr5<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_EXPR_VALUE>::Expr5
    (const typename Expr<T_ARG0>::ConstPtr &arg0,
    const typename Expr<T_ARG1>::ConstPtr &arg1,
    const typename Expr<T_ARG2>::ConstPtr &arg2,
    const typename Expr<T_ARG3>::ConstPtr &arg3,
    const typename Expr<T_ARG4>::ConstPtr &arg4)
    :   itsArg0(arg0),
        itsArg1(arg1),
        itsArg2(arg2),
        itsArg3(arg3),
        itsArg4(arg4)
{
    connect(itsArg0);
    connect(itsArg1);
    connect(itsArg2);
    connect(itsArg3);
    connect(itsArg4);
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
Expr5<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_EXPR_VALUE>::~Expr5()
{
    disconnect(itsArg4);
    disconnect(itsArg3);
    disconnect(itsArg2);
    disconnect(itsArg1);
    disconnect(itsArg0);
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline unsigned int Expr5<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_ARG4,
    T_EXPR_VALUE>::nArguments() const
{
    return 5;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline ExprBase::ConstPtr Expr5<T_ARG0, T_ARG1, T_ARG2, T_ARG3, T_ARG4,
    T_EXPR_VALUE>::argument(unsigned int i) const
{
    switch(i)
    {
        case 0:
            return itsArg0;
        case 1:
            return itsArg1;
        case 2:
            return itsArg2;
        case 3:
            return itsArg3;
        case 4:
            return itsArg4;
        default:
            ASSERTSTR(false, "Invalid argument index specified.");
    }
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG0>::ConstPtr &Expr5<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_ARG4, T_EXPR_VALUE>::argument0() const
{
    return itsArg0;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG1>::ConstPtr &Expr5<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_ARG4, T_EXPR_VALUE>::argument1() const
{
    return itsArg1;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG2>::ConstPtr &Expr5<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_ARG4, T_EXPR_VALUE>::argument2() const
{
    return itsArg2;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG3>::ConstPtr &Expr5<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_ARG4, T_EXPR_VALUE>::argument3() const
{
    return itsArg3;
}

template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_ARG3,
     typename T_ARG4, typename T_EXPR_VALUE>
inline const typename Expr<T_ARG4>::ConstPtr &Expr5<T_ARG0, T_ARG1, T_ARG2,
    T_ARG3, T_ARG4, T_EXPR_VALUE>::argument4() const
{
    return itsArg4;
}

// -------------------------------------------------------------------------- //
// - Implementation: mergeFlags                                             - //
// -------------------------------------------------------------------------- //

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

} //# namespace BBS
} //# namespace LOFAR

#endif
