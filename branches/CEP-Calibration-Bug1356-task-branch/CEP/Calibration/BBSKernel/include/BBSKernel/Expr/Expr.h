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

#include <BBSKernel/Expr/ExprResult.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Cache.h>

// TODO: Reduction functions like MatrixSum
// TODO: Element-wise functions like MatrixSum
// TODO: Rename Expr{1,2,3} to UnaryExpr, BinaryExpr, TernaryExpr?

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


class ExprBase
{
public:
    typedef shared_ptr<ExprBase>                Ptr;
    typedef shared_ptr<const ExprBase>          ConstPtr;

    typedef vector<PValueKey>::const_iterator   const_solvables_iterator;

    ExprBase()
        :   itsConsumerCount(0),
            itsId(theirId++)
    {
    }

    virtual ~ExprBase()
    {
    }

    ExprId getId() const
    {
        return itsId;
    }

    // Recurse the expression tree to determine which solvable parameters
    // (coefficients) each sub-expression depends on. This information is cached
    // internally.
    void updateSolvables() const
    {
        set<PValueKey> solvables;
        updateSolvables(solvables);
    }

    const_solvables_iterator begin() const
    {
        return itsSolvables.begin();
    }

    const_solvables_iterator end() const
    {
        return itsSolvables.end();
    }

    // Provide a description of this node in human readable form.
//    virtual string getDescription() const = 0;

//  TODO: Create hook for visitors, like:
//    virtual void accept(ExprVisitor &visitor, size_t level = 0) const = 0;

protected:
    void connect(const ExprBase::ConstPtr &expr) const
    {
        expr->incConsumerCount();
    }

    void disconnect(const ExprBase::ConstPtr &expr) const
    {
        expr->decConsumerCount();
    }

    unsigned int getConsumerCount() const
    {
        return itsConsumerCount;
    }

    virtual unsigned int getArgumentCount() const = 0;
    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const = 0;

    virtual void updateSolvables(set<PValueKey> &solvables) const
    {
        set<PValueKey> tmp;
        for(unsigned int i = 0; i < getArgumentCount(); ++i)
        {
            getArgument(i)->updateSolvables(tmp);
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

    // Dependencies between nodes are uni-directional and point from the node
    // that produces a result to the node(s) that consume(s) (uses) the result.
    //
    // Consumer nodes are resposible for calling incConsumerCount() on the
    // producer node to signal their interest in its result. Care should be
    // taken to balance this with a call to decConsumerCount() when the
    // dependency is no longer needed.
    //
    // Currently, a node only keeps track of the _number_ of nodes that depend
    // on its result. This information is used to decide if a node should cache
    // its result. If a list of references to consumer nodes would be needed
    // in the future, these methods can be extended accordingly.
    //
    // NB. These methods ARE NOT thread safe.
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
    // to the constituent nodes (derivatives of ExprBase). The consumer count is
    // used to decide if it makes sense to cache a result. As caching is an
    // implementation detail, anything related to it should not be detectable
    // outside the Expr hierarchy.
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

    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache)
        const = 0;
};


template <typename T_ARG0, typename T_EXPR_VALUE>
class Expr1: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<Expr1>       Ptr;
    typedef shared_ptr<const Expr1> ConstPtr;

    typedef T_ARG0  Arg0Type;

    using ExprBase::connect;
    using ExprBase::disconnect;
    using ExprBase::begin;
    using ExprBase::end;

    using ExprBase::getId;
    using ExprBase::getConsumerCount;

    Expr1(const typename Expr<T_ARG0>::ConstPtr &expr0)
        :   Expr<T_EXPR_VALUE>(),
            itsExpr0(expr0)
    {
        connect(itsExpr0);
    }

    ~Expr1()
    {
        disconnect(itsExpr0);
    }

    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache)
        const
    {
        T_EXPR_VALUE result;

        if(!cache.query(getId(), request.getId(), result))
        {
            // Evaluate flags.
            const T_ARG0 arg0 = itsExpr0->evaluate(request, cache);
            result.setFlags(evaluateFlags(request, arg0.flags()));

            // Compute main value.
            result.assign(evaluateImpl(request, arg0.value()));

            // Compute perturbed values.
            ExprBase::const_solvables_iterator it = begin();
            while(it != end())
            {
                result.assign(*it, evaluateImpl(request, arg0.value(*it)));
                ++it;
            }

            if(getConsumerCount() > 1)
            {
                cache.insert(getId(), request.getId(), result);
            }
        }

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return 1;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        ASSERT(i == 0);
        return itsExpr0;
    }

    virtual const FlagArray evaluateFlags(const Request&, const FlagArray &arg0)
        const
    {
        return arg0;
    }

    virtual const typename T_EXPR_VALUE::proxy evaluateImpl
        (const Request &request, const typename T_ARG0::proxy &arg0) const = 0;

private:
    typename Expr<T_ARG0>::ConstPtr itsExpr0;
};


template <typename T_ARG0, typename T_ARG1, typename T_EXPR_VALUE>
class Expr2: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<Expr2>       Ptr;
    typedef shared_ptr<const Expr2> ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;

    using ExprBase::connect;
    using ExprBase::disconnect;
    using ExprBase::begin;
    using ExprBase::end;

    using ExprBase::getId;
    using ExprBase::getConsumerCount;

    Expr2(const typename Expr<T_ARG0>::ConstPtr &expr0,
        const typename Expr<T_ARG1>::ConstPtr &expr1)
        :   Expr<T_EXPR_VALUE>(),
            itsExpr0(expr0),
            itsExpr1(expr1)
    {
        connect(itsExpr0);
        connect(itsExpr1);
    }

    ~Expr2()
    {
        disconnect(itsExpr0);
        disconnect(itsExpr1);
    }

    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        T_EXPR_VALUE result;

        if(!cache.query(getId(), request.getId(), result))
        {
            // Evaluate flags.
            const T_ARG0 arg0 = itsExpr0->evaluate(request, cache);
            const T_ARG1 arg1 = itsExpr1->evaluate(request, cache);

            FlagArray flags[2];
            flags[0] = arg0.flags();
            flags[1] = arg1.flags();
            result.setFlags(evaluateFlags(request, flags));

            // Compute main value.
            result.assign(evaluateImpl(request, arg0.value(), arg1.value()));

            // Compute perturbed values.
            ExprBase::const_solvables_iterator it = begin();
            while(it != end())
            {
                result.assign(*it, evaluateImpl(request, arg0.value(*it),
                    arg1.value(*it)));
                ++it;
            }

            if(getConsumerCount() > 1)
            {
                cache.insert(getId(), request.getId(), result);
            }
        }

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return 2;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        ASSERT(i < 2);
        return i == 0 ? static_pointer_cast<const ExprBase>(itsExpr0)
            : static_pointer_cast<const ExprBase>(itsExpr1);
    }

    virtual const FlagArray evaluateFlags(const Request&,
        const FlagArray (&flags)[2]) const
    {
        return mergeFlags(flags, flags + 2);
    }

    virtual const typename T_EXPR_VALUE::proxy evaluateImpl
        (const Request &request, const typename T_ARG0::proxy &arg0,
        const typename T_ARG1::proxy &arg1) const = 0;

private:
    typename Expr<T_ARG0>::ConstPtr itsExpr0;
    typename Expr<T_ARG1>::ConstPtr itsExpr1;
};


template <typename T_ARG0, typename T_ARG1, typename T_ARG2, typename T_EXPR_VALUE>
class Expr3: public Expr<T_EXPR_VALUE>
{
public:
    typedef shared_ptr<Expr3>       Ptr;
    typedef shared_ptr<const Expr3> ConstPtr;

    typedef T_ARG0  Arg0Type;
    typedef T_ARG1  Arg1Type;
    typedef T_ARG2  Arg2Type;

    using ExprBase::connect;
    using ExprBase::disconnect;
    using ExprBase::begin;
    using ExprBase::end;

    using ExprBase::getId;
    using ExprBase::getConsumerCount;

    Expr3(const typename Expr<T_ARG0>::ConstPtr &expr0,
        const typename Expr<T_ARG1>::ConstPtr &expr1,
        const typename Expr<T_ARG2>::ConstPtr &expr2)
        :   Expr<T_EXPR_VALUE>(),
            itsExpr0(expr0),
            itsExpr1(expr1),
            itsExpr2(expr2)
    {
        connect(itsExpr0);
        connect(itsExpr1);
        connect(itsExpr2);
    }

    ~Expr3()
    {
        disconnect(itsExpr0);
        disconnect(itsExpr1);
        disconnect(itsExpr2);
    }

    virtual const T_EXPR_VALUE evaluate(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        T_EXPR_VALUE result;

        if(!cache.query(getId(), request.getId(), result))
        {
            // Evaluate arguments.
            const T_ARG0 arg0 = itsExpr0->evaluate(request, cache);
            const T_ARG1 arg1 = itsExpr1->evaluate(request, cache);
            const T_ARG2 arg2 = itsExpr2->evaluate(request, cache);

            // Evaluate flags.
            FlagArray flags[3];
            flags[0] = arg0.flags();
            flags[1] = arg1.flags();
            flags[2] = arg2.flags();
            result.setFlags(evaluateFlags(request, flags));

            // Compute main value.
            result.assign(evaluateImpl(request, arg0.value(), arg1.value(),
                arg2.value()));

            // Compute perturbed values.
            ExprBase::const_solvables_iterator it = begin();
            while(it != end())
            {
                result.assign(*it, evaluateImpl(request, arg0.value(*it),
                    arg1.value(*it), arg2.value(*it)));
                ++it;
            }

            if(getConsumerCount() > 1)
            {
                cache.insert(getId(), request.getId(), result);
            }
        }

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return 3;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        switch(i)
        {
            case 0:
                return itsExpr0;
            case 1:
                return itsExpr1;
            case 2:
                return itsExpr2;
            default:
                ASSERT(false);
        }
    }

    virtual const FlagArray evaluateFlags(const Request&,
        const FlagArray (&flags)[3]) const
    {
        return mergeFlags(flags, flags + 3);
    }

    virtual const typename T_EXPR_VALUE::proxy evaluateImpl
        (const Request &request, const typename T_ARG0::proxy &arg0,
        const typename T_ARG1::proxy &arg1, const typename T_ARG1::proxy &arg2)
        const = 0;

private:
    typename Expr<T_ARG0>::ConstPtr itsExpr0;
    typename Expr<T_ARG1>::ConstPtr itsExpr1;
    typename Expr<T_ARG2>::ConstPtr itsExpr2;
};


template <typename T_EXPR_VALUE>
class AsExpr;


// Adaptor class to bundle multiple Expr<Scalar> into a single Expr<Vector<N> >.
template <>
template <unsigned int LENGTH>
class AsExpr<Vector<LENGTH> >: public Expr<Vector<LENGTH> >
{
public:
    typedef shared_ptr<AsExpr>  Ptr;
    typedef shared_ptr<AsExpr>  ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    ~AsExpr()
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            disconnect(itsExpr[i]);
        }
    }

    void connect(unsigned int i0, const typename Expr<Scalar>::ConstPtr &expr)
    {
        connect(expr);
        itsExpr[i0] = expr;
    }

    virtual const Vector<LENGTH> evaluate(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        Vector<LENGTH> result;

        // Evaluate arguments (pass through).
        Scalar args[LENGTH];
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            args[i] = itsExpr[i]->evaluate(request, cache);
            result.setFieldSet(i, args[i].getFieldSet());
        }

        // Evaluate flags.
        FlagArray flags[LENGTH];
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            flags[i] = args[i].flags();
        }
        result.setFlags(mergeFlags(flags, flags + LENGTH));

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return LENGTH;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        ASSERT(i < LENGTH);
        return itsExpr[i];
    }

private:
    typename Expr<Scalar>::ConstPtr itsExpr[LENGTH];
};


// Adaptor class to bundle multiple Expr<Scalar> into a single
// Expr<JonesMatrix>.
template <>
class AsExpr<JonesMatrix>: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<AsExpr>  Ptr;
    typedef shared_ptr<AsExpr>  ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    ~AsExpr()
    {
        for(unsigned int i = 0; i < 4; ++i)
        {
            disconnect(itsExpr[i]);
        }
    }

    void connect(unsigned int i1, unsigned int i0,
        const Expr<Scalar>::ConstPtr &expr)
    {
        DBGASSERT(i1 < 2 && i0 < 2);
        connect(expr);
        itsExpr[i1 * 2 + i0] = expr;
    }

    virtual const JonesMatrix evaluate(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        JonesMatrix result;

        // Evaluate arguments (pass through).
        Scalar args[4];
        for(unsigned int i = 0; i < 4; ++i)
        {
            args[i] = itsExpr[i]->evaluate(request, cache);
            result.setFieldSet(i, args[i].getFieldSet());
        }

        // Evaluate flags.
        FlagArray flags[4];
        for(unsigned int i = 0; i < 4; ++i)
        {
            flags[i] = args[i].flags();
        }
        result.setFlags(mergeFlags(flags, flags + 4));

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return 4;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        ASSERT(i < 4);
        return itsExpr[i];
    }

private:
    Expr<Scalar>::ConstPtr  itsExpr[4];
};


// Adaptor class to bundle two real Expr<Scalar> into a single complex
// Expr<Scalar>, where the two input Expr represent the real and imaginary part
// respectively.
class AsComplex: public Expr2<Scalar, Scalar, Scalar>
{
public:
    typedef shared_ptr<AsComplex>       Ptr;
    typedef shared_ptr<const AsComplex> ConstPtr;

    AsComplex(const Expr<Scalar>::ConstPtr &re,
        const Expr<Scalar>::ConstPtr &im)
        : Expr2<Scalar, Scalar, Scalar>(re, im)
    {
    }

private:
    virtual const Scalar::proxy evaluateImpl(const Request&,
        const Scalar::proxy &re, const Scalar::proxy &im) const
    {
        Scalar::proxy result;
        result.assign(tocomplex(re(), im()));
        return result;
    }
};


// Adaptor class to bundle two real Expr<Scalar> into a single complex
// Expr<Scalar>, where the two input Expr represent the complex modulus
// (amplitude)and the complex argument (phase) respectively.
class AsPolar: public Expr2<Scalar, Scalar, Scalar>
{
public:
    typedef shared_ptr<AsPolar>         Ptr;
    typedef shared_ptr<const AsPolar>   ConstPtr;

    AsPolar(const Expr<Scalar>::ConstPtr &modulus,
        const Expr<Scalar>::ConstPtr &argument)
        : Expr2<Scalar, Scalar, Scalar>(modulus, argument)
    {
    }

private:
    virtual const Scalar::proxy evaluateImpl(const Request&,
        const Scalar::proxy &mod, const Scalar::proxy &arg) const
    {
        Scalar::proxy result;
        result.assign(tocomplex(mod() * cos(arg()), mod() * sin(arg())));
        return result;
    }
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
