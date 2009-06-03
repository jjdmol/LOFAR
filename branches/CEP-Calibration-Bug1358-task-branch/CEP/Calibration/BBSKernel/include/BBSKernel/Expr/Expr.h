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
#include <limits>
using std::numeric_limits;

//#include <blitz/array.h>
//#define ARRAY(__T_NUMERIC)  blitz::Array<__T_NUMERIC, 2>
//#define ARRAY_WITH_RANK(__T_NUMERIC, __RANK)  blitz::Array<__T_NUMERIC, __RANK>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{


//template <typename T_RESULT>
//class Expr;

typedef size_t NodeId;

//class Cache;

class Expr
{
public:
    typedef shared_ptr<Expr>        Ptr;
    typedef shared_ptr<const Expr>  ConstPtr;

    Expr()
        :   itsId(theirId++),
            itsConsumerCount(0)
    {
    }

    virtual ~Expr()
    {
    }

    // Recurse the experssion tree to determine which solvable parameters
    // (coefficients) the expression and its sub-expressions depend on. This
    // information is cached internally.
    void updateSolvables() const
    {
        set<PValueKey> solvables;
        updateSolvables(solvables);
    }        

    const set<PValueKey> &getSolvables() const
    {
        return itsSolvables;
    }

    NodeId getId() const
    {
        return itsId;
    }
    
    void upConsumerCount() const
    {
        ++itsConsumerCount;
    }
    
    void downConsumerCount() const
    {
        --itsConsumerCount;
    }

    unsigned int getConsumerCount() const
    {
        return itsConsumerCount;
    }
    
    virtual ExprResult::ConstPtr evaluate(const Request &request, Cache &cache,
        const PValueKey &key = PValueKey()) const = 0;

    // Provide a description of this node in human readable form.
//    virtual string getDescription() const = 0;
    
//  TODO: Create hook for visitors, like:
//    virtual void accept(ExprVisitor &visitor, size_t level = 0) const = 0;

protected:
    // Dependencies between nodes are uni-directional and point from the node
    // that produces a result to the node(s) that consumes (uses) this result.
    //
    // Consumer nodes are resposible for calling registerConsumer() on the
    // producer node to signal their interest in its result. Care should be
    // taken to balance this with a call to dropConsumer() when the dependency
    // is no longer needed.
    // 
    // Currently, a node only keeps track of the _number_ of nodes that depend
    // on its result. This information is used to decide if a node should cache
    // its result. If a list of references to consumer nodes would be needed
    // in the future, these methods can be extended accordingly.
    //
    // NB. These methods ARE NOT thread safe.
    // <group>
    // </group>
    
    virtual unsigned int getInputCount() const = 0;
    virtual Expr::ConstPtr getInput(unsigned int i) const = 0;
    
    virtual void updateSolvables(set<PValueKey> &solvables) const
    {
        for(unsigned int i = 0; i < getInputCount(); ++i)
        {
            getInput(i)->updateSolvables(solvables);
        }
        
        itsSolvables.clear();
        itsSolvables.insert(solvables.begin(), solvables.end());
    }
    
    mutable set<PValueKey>  itsSolvables;

private:
    // Forbid copy and assignment as it is probably not needed. Should it be
    // needed in the future, care has to be taken that the consumer count is
    // _not_ copied. Also, any derived classes should be checked for data
    // members that should not be copied.
    Expr(const Expr &);
    Expr &operator=(const Expr &);
    
    NodeId                  itsId;

    // The number of registered consumers. This attribute is mutable such that
    // an expression tree can be constructed without requiring non-const access
    // to the constituent nodes (derivatives of Expr). The consumer count is
    // used to decide if it makes sense to cache a result. As caching is an
    // implementation detail, anything related to it should not be detectable
    // outside the Expr hierarchy.
    mutable unsigned int    itsConsumerCount;

    static NodeId           theirId;    
};

class ExprTerminus: public Expr
{
public:
    typedef shared_ptr<ExprTerminus>        Ptr;
    typedef shared_ptr<const ExprTerminus>  ConstPtr;

    virtual ExprResult::ConstPtr evaluate(const Request &request, Cache &cache,
        const PValueKey &key = PValueKey()) const
    {
        // Query the cache.
        if(!key.valid() || itsSolvables.find(key) == itsSolvables.end())
        {
            // Have to return main value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                PValueKey());

            if(cached)
            {
                return cached;
            }
        }
        else
        {
            // Have to return perturbed value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                key);

            if(cached)
            {
                return cached;
            }
        }

        // Allocate result.
        ExprResult::Ptr result(new ExprResult());

        // Evaluate flags.
        result->setFlags(evaluateFlags(request));
        
        // Compute main result.
        result->setValue(evaluateImpl(request));

        // Compute perturbed results.
        //      re-use tensors vector
        //      loop over union of pvalue maps
        //      result->setPValue(key, evaluateImpl(request, inputs));
        
        // Store result in cache.
        if(!key.valid() || getConsumerCount() > 1)
        {
            cache.insert(getId(), request.getId(), key, result);
        }
        
        return result;
    }

protected:
    virtual unsigned int getInputCount() const
    {
        return 0;
    }

    virtual Expr::ConstPtr getInput(unsigned int) const
    {
        ASSERT(false);
    }

    virtual const FlagArray evaluateFlags(const Request&) const
    {
        return FlagArray();
    }
    
    virtual ValueSet::ConstPtr evaluateImpl(const Request &request) const = 0;
};


template <unsigned int COUNT>
class ExprStatic: public Expr
{
public:
    typedef shared_ptr<ExprStatic<COUNT> >          Ptr;
    typedef shared_ptr<const ExprStatic<COUNT> >    ConstPtr;

    virtual ~ExprStatic()
    {
        for(unsigned int i = 0; i < COUNT; ++i)
        {
            itsInputExpr[i]->downConsumerCount();
        }
    }
            
    void connect(unsigned int i, const Expr::ConstPtr &expr)
    {
        ASSERT(i < COUNT);

        expr->upConsumerCount();
        itsInputExpr[i] = expr;
    }

    virtual ExprResult::ConstPtr evaluate(const Request &request, Cache &cache,
        const PValueKey &key = PValueKey()) const
    {
        // Query the cache.
        if(!key.valid() || itsSolvables.find(key) == itsSolvables.end())
        {
            // Have to return main value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                PValueKey());

            if(cached)
            {
                return cached;
            }
        }
        else
        {
            // Have to return perturbed value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                key);

            if(cached)
            {
                return cached;
            }
        }
        
        // Evaluate inputs.
        // TODO: Check for NULL inputs.
        // TODO: Keep vector inside class to avoid continuous re-allocation?
        ExprResult::ConstPtr inputResults[COUNT];
        for(unsigned int i = 0; i < COUNT; ++i)
        {
            inputResults[i] = itsInputExpr[i]->evaluate(request, cache, key);
        }

        // Allocate result.
        ExprResult::Ptr result(new ExprResult());

        // Evaluate flags.
        result->setFlags(evaluateFlags(request, inputResults));
        
        // Compute main result.
        ValueSet::ConstPtr values[COUNT];
        for(unsigned int i = 0; i < COUNT; ++i)
        {
            values[i] = inputResults[i]->getValue();
        }

        result->setValue(evaluateImpl(request, values));

        // Compute perturbed results.
        //      re-use tensors vector
        //      loop over union of pvalue maps
        //      result->setPValue(key, evaluateImpl(request, inputs));
        
        // Store result in cache.
        if(!key.valid() || getConsumerCount() > 1)
        {
            cache.insert(getId(), request.getId(), key, result);
        }
        
        return result;
    }
    
protected:
    virtual unsigned int getInputCount() const
    {
        return COUNT;
    }

    virtual Expr::ConstPtr getInput(unsigned int i) const
    {
        ASSERT(i < COUNT);
        return itsInputExpr[i];
    }

    virtual const FlagArray evaluateFlags(const Request&,
        const ExprResult::ConstPtr (&inputs)[COUNT]) const
    {
        FlagArray flags;

        unsigned int i = 0;
        while(i < COUNT)
        {
            if(inputs[i]->hasFlags())
            {
                flags = inputs[i]->getFlags().clone();
            }
            ++i;
        }
        
        while(i < COUNT)
        {
            if(inputs[i]->hasFlags())
            {
                flags |= inputs[i]->getFlags();
            }
            ++i;
        }
        
        return flags;
    }        

    virtual ValueSet::ConstPtr evaluateImpl(const Request &request,
        const ValueSet::ConstPtr (&inputs)[COUNT]) const = 0;
    
private:
    Expr::ConstPtr  itsInputExpr[COUNT];    
};

class ExprDynamic: public Expr
{
public:
    typedef shared_ptr<ExprDynamic>         Ptr;
    typedef shared_ptr<const ExprDynamic>   ConstPtr;

    virtual ~ExprDynamic()
    {
        for(unsigned int i = 0; i < itsInputExpr.size(); ++i)
        {
            itsInputExpr[i]->downConsumerCount();
        }
    }

    void connect(const Expr::ConstPtr &input)
    {
        input->upConsumerCount();
        itsInputExpr.push_back(input);
    }
    
    virtual ExprResult::ConstPtr evaluate(const Request &request, Cache &cache,
        const PValueKey &key = PValueKey()) const
    {
        // Query the cache.
        if(!key.valid() || itsSolvables.find(key) == itsSolvables.end())
        {
            // Have to return main value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                PValueKey());

            if(cached)
            {
                return cached;
            }
        }
        else
        {
            // Have to return perturbed value.
            // Check if there is a cached result available for this request.
            ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
                key);

            if(cached)
            {
                return cached;
            }
        }
        
        // Evaluate inputs.
        // TODO: Check for NULL inputs.
        // TODO: Keep vector inside class to avoid continuous re-allocation?
        vector<ExprResult::ConstPtr> inputResults(itsInputExpr.size());
        for(unsigned int i = 0; i < itsInputExpr.size(); ++i)
        {
            inputResults[i] = itsInputExpr[i]->evaluate(request, cache, key);
        }

        // Allocate result.
        ExprResult::Ptr result(new ExprResult());

        // Evaluate flags.
        result->setFlags(evaluateFlags(request, inputResults));
        
        // Compute main result.
        vector<ValueSet::ConstPtr> values(itsInputExpr.size());
        for(unsigned int i = 0; i < itsInputExpr.size(); ++i)
        {
            values[i] = inputResults[i]->getValue();
        }

        result->setValue(evaluateImpl(request, values));

        // Compute perturbed results.
        //      re-use tensors vector
        //      loop over union of pvalue maps
        //      result->setPValue(key, evaluateImpl(request, inputs));
        
        // Store result in cache.
        if(!key.valid() || getConsumerCount() > 1)
        {
            cache.insert(getId(), request.getId(), key, result);
        }
        
        return result;
    }

protected:
    virtual unsigned int getInputCount() const
    {
        return itsInputExpr.size();
    }

    virtual Expr::ConstPtr getInput(unsigned int i) const
    {
        ASSERT(i < itsInputExpr.size());
        return itsInputExpr[i];
    }

    virtual const FlagArray evaluateFlags(const Request&,
        const vector<ExprResult::ConstPtr> &inputs) const
    {        
        FlagArray flags;

        vector<ExprResult::ConstPtr>::const_iterator inputIt = inputs.begin();
        while(inputIt != inputs.end())
        {
            if((*inputIt)->hasFlags())
            {
                flags = (*inputIt)->getFlags().clone();
            }
            ++inputIt;
        }
        
        while(inputIt != inputs.end())
        {
            if((*inputIt)->hasFlags())
            {
                flags |= (*inputIt)->getFlags();
            }
            ++inputIt;
        }
        
        return flags;
    }
    
    virtual ValueSet::ConstPtr evaluateImpl(const Request &request,
        const vector<ValueSet::ConstPtr> &inputs) const = 0;

private:
    vector<Expr::ConstPtr>  itsInputExpr;
};

class AsComplex: public ExprStatic<2>
{
public:
    typedef shared_ptr<AsComplex>       Ptr;
    typedef shared_ptr<const AsComplex> ConstPtr;

    enum Inputs
    {
        RE,
        IM,
        N_Inputs
    };
    
    AsComplex()
        : ExprStatic<AsComplex::N_Inputs>()
    {
    }

    AsComplex(const Expr::ConstPtr &re, const Expr::ConstPtr &im)
        : ExprStatic<AsComplex::N_Inputs>()
    {
        connect(RE, re);
        connect(IM, im);
    }

private:
    // Compute a result for the given request.
    virtual ValueSet::ConstPtr evaluateImpl(const Request &request,
        const ValueSet::ConstPtr (&inputs)[AsComplex::N_Inputs]) const
    {
        ValueSet::Ptr result(new ValueSet());
        result->assign(tocomplex(inputs[RE]->value(), inputs[IM]->value()));
        return result;
    }
};

class Zip: public ExprDynamic
{
public:
    typedef shared_ptr<Zip>         Ptr;
    typedef shared_ptr<const Zip>   ConstPtr;

    Zip()
        : ExprDynamic()
    {
        itsShape.zero();
    }

    Zip(const ValueSet::Shape &shape)
        : ExprDynamic()
    {
        copy(shape.begin(), shape.end(), itsShape.begin());
    }        

private:
    virtual ValueSet::ConstPtr evaluateImpl(const Request&,
        const vector<ValueSet::ConstPtr> &inputs) const
    {
        ValueSet::Ptr result(new ValueSet(itsShape));
        ASSERT(result->size() == inputs.size());
        
        for(unsigned int i = 0; i < inputs.size(); ++i)
        {
            ASSERT(inputs[i]->rank() == 0);
            result->assign(i, inputs[i]->value());
        }

        return result;
    }
    
    ValueSet::Shape   itsShape;
};


//template <typename T_RESULT>
//class Expr: public ExprBase
//{
//public:
//    typedef shared_ptr<Expr<T_RESULT> >         Ptr;
//    typedef shared_ptr<const Expr<T_RESULT> >   ConstPtr;
//    
//    typedef T_RESULT                            ResultType;

//    using ExprBase::getInputCount;
//    
//    Expr()
//        :   ExprBase()
//    {
//    }

//    Expr(unsigned int nInputs)
//        :   ExprBase(nInputs)
//    {
//    }
//    
//    // Compute the result of this node in a thread-safe way. The return value
//    // is strictly READ ONLY: It could reference a cached value, and if it does
//    // any write would be visible to all consumers. Moreover, writes to the
//    // return value are NOT guaranteed to be thread safe. To resolve these
//    // issues the result type (T) should provide copy-on-write semantics in a
//    // thread safe way.
//    //
//    // TODO: Should this method have an abstract counterpart in ExprBase?
//    //
//    // NB. This method IS thread safe.
////    const T &evaluate(const Request &request, T &buffer) const
//    virtual typename T_RESULT::ConstPtr evaluate(const Request &request,
//        const PValueKey &key, bool perturbed, Cache &cache) const
//    {
//        // Check if there is a cached result available for this request.
////        if(itsCachedId == request.getId())
////        {
//            // TODO: Check for empty cache.
////            return itsCache;
////        }
//        
//        // No cached result, so compute the value and store it in the buffer
//        // that was passed in.
////        return evaluateImpl(request, key, perturbed);

////        return static_pointer_cast<const T_RESULT>(evaluateBase(request, key,
////            perturbed));

//        // TODO: Check for NULL inputs.
//        
//        return static_pointer_cast<const T_RESULT>(evaluateBase(request, key,
//            perturbed, cache));
//    }

//    // Compute and cache the result of this node.
//    //
//    // TODO: Should this method have an abstract counterpart in ExprBase?
//    //
//    // NB. This method IS NOT thread safe.
////    void cache(const Request &request) const
////    {
////        itsCache = evaluateImpl(request);
//////        itsCachedId = request.getId();
////    }
//    
////protected:
////    // These attributes implement caching. As caching is an implementation
////    // detail, the attributes are mutable.
//////    mutable RequestId   itsCachedId;
////    mutable typename T::ConstPtr  itsCache;

////private:
////    virtual typename T_RESULT::ConstPtr evaluateImpl(const Request &request,
////        const PValueKey &key, bool perturbed) const = 0;

////    virtual void evaluateImpl(const Request &request, const PValueKey &key,
////        bool perturbed, const typename T_RESULT::Ptr &result) const = 0;
//};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
