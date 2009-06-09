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

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Unique (sub)expression identifier.
typedef size_t ExprId;

// Helper function to compute the bitwise or of all the ExprValueSet instances
// in the range [it, end).
template <typename T_ITER>
FlagArray mergeFlags(T_ITER it, T_ITER end)
{
    FlagArray result;

    // Skip until a non-empty FlagArray is encountered.
    while(it != end && !it->hasFlags())
    {
        ++it;
    }

    // Merge (bitwise or) all non-empty FlagArray's.
    if(it != end)
    {
        ASSERT(it->hasFlags());
        result = it->flags().clone();
        ++it;

        while(it != end)
        {
            if(it->hasFlags())
            {
                result |= it->flags();
            }
            ++it;
        }
    }

    return result;
}

class Expr
{
public:
    typedef shared_ptr<Expr>        Ptr;
    typedef shared_ptr<const Expr>  ConstPtr;

    typedef vector<PValueKey>::const_iterator   const_solvables_iterator;

    Expr()
        :   itsConsumerCount(0),
            itsId(theirId++)
    {
    }

    virtual ~Expr()
    {
        vector<Expr::ConstPtr>::const_iterator it = itsArguments.begin();
        while(it != itsArguments.end())
        {
            disconnect(*it);
            ++it;
        }
    }

    ExprId id() const
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

    virtual const ExprValueSet evaluate(const Request &request, Cache &cache)
        const = 0;

    // Provide a description of this node in human readable form.
//    virtual string getDescription() const = 0;

//  TODO: Create hook for visitors, like:
//    virtual void accept(ExprVisitor &visitor, size_t level = 0) const = 0;

protected:
    // Dependencies between nodes are uni-directional and point from the node
    // that produces a result to the node(s) that consume(s) (uses) the result.
    //
    // Consumer nodes are resposible for calling connect() passing the producer
    // node as an argument to signal their interest in its result. Care should
    // be taken to balance this with a call to disconnect() when the dependency
    // is no longer needed (at the moment, Expr does this automatically in the
    // destructor).
    //
    // Currently, a node only keeps track of the _number_ of nodes that depend
    // on its result. This information is used to decide if a node should cache
    // its result. If a list of references to consumer nodes would be needed
    // in the future, these methods can be extended accordingly.
    //
    // NB. These methods ARE NOT thread safe.
    // @{
    void connect(const Expr::ConstPtr &expr) const
    {
        expr->incConsumerCount();
    }

    void disconnect(const Expr::ConstPtr &expr) const
    {
        expr->decConsumerCount();
    }
    // @}

    unsigned int getConsumerCount() const
    {
        return itsConsumerCount;
    }

    virtual void updateSolvables(set<PValueKey> &solvables) const
    {
        set<PValueKey> tmp;
        for(unsigned int i = 0; i < itsArguments.size(); ++i)
        {
            itsArguments[i]->updateSolvables(tmp);
        }

        itsSolvables.clear();
        itsSolvables.insert(itsSolvables.begin(), tmp.begin(), tmp.end());
        solvables.insert(tmp.begin(), tmp.end());
    }

    mutable vector<PValueKey>   itsSolvables;
    vector<Expr::ConstPtr>      itsArguments;

private:
    // Forbid copy and assignment as it is probably not needed. Should it be
    // needed in the future, care has to be taken that the consumer count is
    // _not_ copied. Also, any derived classes should be checked for data
    // members that should not be copied.
    Expr(const Expr&);
    Expr &operator=(const Expr&);

    void incConsumerCount() const
    {
        ++itsConsumerCount;
    }

    void decConsumerCount() const
    {
        --itsConsumerCount;
    }

    // The number of registered consumers. This attribute is mutable such that
    // an expression tree can be constructed without requiring non-const access
    // to the constituent nodes (derivatives of Expr). The consumer count is
    // used to decide if it makes sense to cache a result. As caching is an
    // implementation detail, anything related to it should not be detectable
    // outside the Expr hierarchy.
    mutable unsigned int        itsConsumerCount;

    ExprId                      itsId;
    static ExprId               theirId;
};

template <unsigned int COUNT>
class ExprStatic: public Expr
{
public:
    typedef shared_ptr<ExprStatic>          Ptr;
    typedef shared_ptr<const ExprStatic>    ConstPtr;

    ExprStatic()
        :   Expr()
    {
        itsArguments.resize(COUNT);
    }

    void connect(unsigned int i, const Expr::ConstPtr &expr)
    {
        ASSERT(i < COUNT);

        Expr::connect(expr);
        itsArguments[i] = expr;
    }

    virtual const ExprValueSet evaluate(const Request &request, Cache &cache)
        const
    {
        // Create an (empty) result.
        ExprValueSet result;

        // Query the cache.
        // TODO: Does this hinder return value optimization (because it creates
        // an additional return path)?
        if(getConsumerCount() > 1 && cache.query(id(), request.getId(), result))
        {
            return result;
        }

        // Evaluate arguments.
        ExprValueSet arguments[COUNT];
        for(unsigned int i = 0; i < COUNT; ++i)
        {
            arguments[i] = itsArguments[i]->evaluate(request, cache);
        }

        // Resize the result.
        result.resize(shape(arguments));

        // Evaluate flags.
        result.assignFlags(evaluateFlags(request, arguments));

        // Compute main result.
        ExprValue argv[COUNT];
        for(unsigned int i = 0; i < COUNT; ++i)
        {
            argv[i] = arguments[i].value();
        }

        ExprValue temporary(result.shape());
        evaluateImpl(request, argv, temporary);
        result.assign(temporary);

        // Compute perturbed values.
        Expr::const_solvables_iterator it = begin();
        while(it != end())
        {
            for(unsigned int i = 0; i < COUNT; ++i)
            {
                argv[i] = arguments[i].value(*it);
            }

            temporary.clear();
            evaluateImpl(request, argv, temporary);
            result.assign(*it, temporary);

            ++it;
        }

        // Store result in cache.
        if(getConsumerCount() > 1)
        {
            cache.insert(id(), request.getId(), result);
        }

        return result;
    }

protected:
    virtual Shape shape(const ExprValueSet (&arguments)[COUNT]) const = 0;

    virtual const FlagArray evaluateFlags(const Request&,
        const ExprValueSet (&arguments)[COUNT]) const
    {
        return mergeFlags(arguments, arguments + COUNT);
    }

    virtual void evaluateImpl(const Request &request,
        const ExprValue (&arguments)[COUNT], ExprValue &result) const = 0;
};


class ExprDynamic: public Expr
{
public:
    typedef shared_ptr<ExprDynamic>         Ptr;
    typedef shared_ptr<const ExprDynamic>   ConstPtr;

    ExprDynamic()
        :   Expr()
    {
    }

    void connect(const Expr::ConstPtr &expr)
    {
        Expr::connect(expr);
        itsArguments.push_back(expr);
    }

    virtual const ExprValueSet evaluate(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        ExprValueSet result;

        // Query the cache.
        // TODO: Does this hinder return value optimization (because it creates
        // an additional return path)?
        if(getConsumerCount() > 1 && cache.query(id(), request.getId(), result))
        {
            return result;
        }

        // Evaluate arguments.
        // TODO: Keep vector inside class to avoid continuous re-allocation?
        vector<ExprValueSet> arguments(itsArguments.size());
        for(unsigned int i = 0; i < itsArguments.size(); ++i)
        {
            arguments[i] = itsArguments[i]->evaluate(request, cache);
        }

        // Resize the result.
        result.resize(shape(arguments));

        // Evaluate flags.
        result.assignFlags(evaluateFlags(request, arguments));

        // Compute main result.
        vector<ExprValue> argv(itsArguments.size());
        for(unsigned int i = 0; i < itsArguments.size(); ++i)
        {
            argv[i] = arguments[i].value();
        }

        ExprValue temporary(result.shape());
        evaluateImpl(request, argv, temporary);
        result.assign(temporary);

        // Compute perturbed values.
        Expr::const_solvables_iterator it = begin();
        while(it != end())
        {
            for(unsigned int i = 0; i < itsArguments.size(); ++i)
            {
                argv[i] = arguments[i].value(*it);
            }

            temporary.clear();
            evaluateImpl(request, argv, temporary);
            result.assign(*it, temporary);

            ++it;
        }

        // Store result in cache.
        if(getConsumerCount() > 1)
        {
            cache.insert(id(), request.getId(), result);
        }

        return result;
    }

protected:
    virtual Shape shape(const vector<ExprValueSet> &arguments) const = 0;

    virtual const FlagArray evaluateFlags(const Request&,
        const vector<ExprValueSet> &arguments) const
    {
        return mergeFlags(arguments.begin(), arguments.end());
    }

    virtual void evaluateImpl(const Request &request,
        const vector<ExprValue> &arguments, ExprValue &result) const = 0;
};


class AsComplex: public ExprStatic<2>
{
public:
    typedef shared_ptr<AsComplex>       Ptr;
    typedef shared_ptr<const AsComplex> ConstPtr;

    enum Arguments
    {
        RE,
        IM,
        N_Arguments
    };

    AsComplex()
        : ExprStatic<AsComplex::N_Arguments>()
    {
    }

    AsComplex(const Expr::ConstPtr &re, const Expr::ConstPtr &im)
        : ExprStatic<AsComplex::N_Arguments>()
    {
        connect(RE, re);
        connect(IM, im);
    }

private:
    virtual Shape shape(const ExprValueSet (&arguments)[AsComplex::N_Arguments])
        const
    {
        DBGASSERT(arguments[RE].shape() == Shape()
            && arguments[IM].shape() == Shape());
        return Shape();
    }

    virtual void evaluateImpl(const Request&,
        const ExprValue (&arguments)[AsComplex::N_Arguments],
        ExprValue &result) const
    {
        result.assign(tocomplex(arguments[RE](), arguments[IM]()));
    }
};


class Zip: public Expr
{
public:
    typedef shared_ptr<Zip>         Ptr;
    typedef shared_ptr<const Zip>   ConstPtr;

    Zip(const Shape &shape)
        :   Expr(),
            itsShape(shape)
    {
        unsigned int size = 1;
        for(unsigned int i = 0; i < shape.rank(); ++i)
        {
            size *= shape[i];
        }
        itsSize = size;
    }

    void connect(const Expr::ConstPtr &expr)
    {
        ASSERT(itsArguments.size() < itsSize);
        Expr::connect(expr);
        itsArguments.push_back(expr);
    }

private:
    virtual const ExprValueSet evaluate(const Request &request, Cache &cache)
        const
    {
        ExprValueSet result(itsShape);
        ASSERT(result.size() == itsArguments.size());
        ASSERT(result.rank() == 1);

        // Evaluate arguments.
        // TODO: Keep vector inside class to avoid continuous re-allocation?
        vector<ExprValueSet> arguments(itsArguments.size());
        for(unsigned int i = 0; i < itsArguments.size(); ++i)
        {
            arguments[i] = itsArguments[i]->evaluate(request, cache);
            ASSERT(arguments[i].rank() == 0);
            result.setFieldSet(i, arguments[i].getFieldSet(0));
        }

        result.assignFlags(mergeFlags(arguments.begin(), arguments.end()));

        return result;
    }

    Shape           itsShape;
    unsigned int    itsSize;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
