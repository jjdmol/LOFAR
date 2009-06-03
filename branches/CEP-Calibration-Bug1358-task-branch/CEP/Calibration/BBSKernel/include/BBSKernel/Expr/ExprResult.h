//# ExprResult.h: Result of an expression
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRRESULT_H
#define LOFAR_BBSKERNEL_EXPR_EXPRRESULT_H

// \file
// Result of an expression

#include <Common/lofar_smartptr.h>
#include <Common/lofar_map.h>
#include <Common/lofar_numeric.h>

#include <Common/LofarLogger.h>

//#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Types.h>

#include <BBSKernel/Expr/FlagArray.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

//typedef uint8   FlagType;


//static const size_t MAX_RANK = 2;
//////typedef size_t Shape[MAX_RANK];


//struct Shape
//{
//    typedef size_t *iterator;
//    typedef const size_t *const_iterator;
//    
//    size_t  __shape[MAX_RANK];

//    const size_t &operator[](size_t i) const
//    { return __shape[i]; }

//    size_t &operator[](size_t i)
//    { return __shape[i]; }
//    
//    iterator begin()
//    { return __shape; }
//    
//    iterator end()
//    { return __shape + MAX_RANK; }

//    const_iterator begin() const
//    { return __shape; }
//    
//    const_iterator end() const
//    { return __shape + MAX_RANK; }

//    void zero()
//    { fill(__shape, __shape + MAX_RANK, size_t()); }
//};


//Shape merge(const Shape &lhs, const Shape &rhs)
//{
//    // TODO: Use MAX_RANK or itsArray.rank()? (loop unroling?).
//    // TODO: Use DBGASSERT instead of assert().
//    // If either shape[i] or itsArray.itsShape[i] are zero, then the OR
//    // operation is safe. If both are non-zero, they should be equal.
//    Shape result;
//    for(size_t i = 0; i < MAX_RANK; ++i)
//    {
//        ASSERT(lhs[i] == 0 || rhs[i] == 0 || lhs[i] == rhs[i]);
//        result[i] = lhs[i] | rhs[i];
//    }
//    return result;
//}


////class FlagBufferImpl
////{
////    // Must be virtual to recover the type of *this.
////    virtual FlagBuffer?? opOr(const FlagBuffer(Impl?) &rhs)
////    
////};

////class FlagScalar
////{
////    FlagBuffer?? opOr(const FlagBuffer(Impl?) &rhs)
////    {
////        return rhs.opOr(*this);
////    }
////    
////    opOr(const Scalar &lhs)
////    opOr(const Matrix &lhs)
////    
////private:
////    FlagType  itsData;
////};

////class FlagMatrix
////{
////public:
////    operator|=
////private:
////    vector<FlagType>    itsData;    
////};

////FlagMatrix operator|(const FlagScalar &lhs, const FlagMatrix &rhs)
////{
////}



//template <typename T>
//FlagArray<T>
//{
//    FlagArray(const Shape &shape, T value = T())
//    {
//        
//        copy(shape.begin(), shape.end(), itsShape.begin());
//        itsData.resize(std::accumulate(itsShape.begin(), itsShape.end(), 1u,
////            std::multiplies<unsigned int>());
//        
//    }

//    Shape                   itsShape;
//    shared_ptr<vector<T> >  itsData;
//};

//template <typename T>
//FlagArray<T> operator|(const FlagArray<T> &lhs, const FlagArray<T> &rhs)
//{
//    FlagArray<T> result(merge(lhs.shape(), rhs.shape()));

//    FlagArray<T>::const_flat_iterator lhs_it(lhs, shape);
//    FlagArray<T>::const_flat_iterator rhs_it(rhs, shape);
//    FlagArray<T>::flat_iterator result_it

//    for(size_t i = 0; i < result.size(); ++i)
//    {
//        *result_it = *lhs_it | *rhs_it;
//        ++lhs_it;
//        ++rhs_it;
//        ++result_it;
//    }
//}


////class FlagBufferIterator
////{
////    FlagBufferIterator
////};


////class FlagBuffer
////{
////    enum BufferType
////    {
////        SCALAR,
////        MATRIX
////    };
////    
////    FlagBuffer operator|(const FlagBuffer &rhs)
////    {
////        myptr->or(rhs)
////            : suppose myptr Scalar
////            Scalar::or(const FlagBuffer &rhs)
////                rhs->or(*this)
////                : suppose rhs type Matrix
////                Matrix::or(const Scalar &lhs)
////                    return lhs | *this;
////                        : need operator|({Matrix,Scalar}, {Matrix,Scalar})
////        operator|(
////    }
////    
////    rank
////    shape
////};



// A key that identifies a perturbed value through its associated (parameter id,
// coefficient id) combination.
class PValueKey
{
public:
    PValueKey();
    PValueKey(size_t parmId, size_t coeffId);

    bool valid() const;
    bool operator<(const PValueKey &other) const;
    bool operator==(const PValueKey &other) const;

    size_t  parmId, coeffId;
};

inline PValueKey::PValueKey()
    :   parmId(std::numeric_limits<size_t>::max()),
        coeffId(std::numeric_limits<size_t>::max())
{
}

inline PValueKey::PValueKey(size_t parmId, size_t coeffId)
    :   parmId(parmId),
        coeffId(coeffId)
{
}
    
inline bool PValueKey::valid() const
{
    return parmId != std::numeric_limits<size_t>::max();
}

inline bool PValueKey::operator<(const PValueKey &other) const
{
    return parmId < other.parmId
        || (parmId == other.parmId && coeffId < other.coeffId);
}
    
inline bool PValueKey::operator==(const PValueKey &other) const
{
    return parmId == other.parmId && coeffId == other.coeffId;
}


class ValueSet
{
public:
    typedef shared_ptr<ValueSet>          Ptr;
    typedef shared_ptr<const ValueSet>    ConstPtr;

    static const unsigned int MAX_RANK = 2;
    
    struct Shape
    {
        typedef size_t *iterator;
        typedef const size_t *const_iterator;
        
        size_t  __shape[MAX_RANK];

        const size_t &operator[](size_t i) const
        { return __shape[i]; }

        size_t &operator[](size_t i)
        { return __shape[i]; }
        
        iterator begin()
        { return __shape; }
        
        iterator end()
        { return __shape + MAX_RANK; }

        const_iterator begin() const
        { return __shape; }
        
        const_iterator end() const
        { return __shape + MAX_RANK; }
        
        size_t size()
        {
            return MAX_RANK;
        }

        void zero()
        { fill(__shape, __shape + MAX_RANK, size_t()); }
    };
    
    ValueSet()
        :   itsRank(0),
            itsValues(1)
    {
    }
    
    ValueSet(const Shape &shape)
    {
        unsigned int size = 1;
        unsigned int rank = 0;
        while(rank < MAX_RANK && shape[rank] != 0)
        {
            itsShape[rank] = shape[rank];
            size *= shape[rank];
            ++rank;
        }        
        itsRank = rank;
        itsValues.resize(size);
    }
    
    ValueSet(unsigned int l0)
        :   itsRank(1),
            itsValues(l0)
    {
        itsShape[0] = l0;
    }

    ValueSet(unsigned int l0, unsigned int l1)
        :   itsRank(2),
            itsValues(l0 * l1)
    {
        itsShape[0] = l0;
        itsShape[1] = l1;
    }

    const Matrix &value() const
    {
        ASSERT(size() > 0);
        return itsValues[0];
    }

    const Matrix &value(unsigned int i0) const
    {
        ASSERT(rank() >= 1 && size() > i0);
        return itsValues[i0];
    }

    // i1 is the fastest running dimension (columns)
    const Matrix &value(unsigned int i0, unsigned int i1) const
    {
        ASSERT(rank() >= 2 && size() > (i0 * itsShape[1] + i1));
        return itsValues[i0 * itsShape[1] + i1];
    }
    
    void assign(const Matrix &value, bool isPerturbed = false)
    {
        ASSERT(size() > 0);
        itsValues[0] = value;
    }

    void assign(unsigned int i0, const Matrix &value, bool isPerturbed = false)
    {
        ASSERT(rank() >= 1 && size() > i0);
        itsValues[i0] = value;
    }

    void assign(unsigned int i0, unsigned int i1, const Matrix &value,
        bool isPerturbed = false)
    {
        ASSERT(rank() >= 2 && size() > (i0 * itsShape[1] + i1));
        itsValues[i0 * itsShape[1] + i1] = value;
    }

    unsigned int rank() const
    {
        return itsRank;
//        return itsShape.size();
    }
    
    unsigned int size() const
    {
        return itsValues.size();
//        return std::accumulate(itsShape.begin(), itsShape.end(), 1u,
//            std::multiplies<unsigned int>());
    }

    unsigned int shape(unsigned int dim) const
    {
        ASSERT(dim < rank());
        return itsShape[dim];
    }

private:
    unsigned int            itsRank;
    unsigned int            itsShape[2];
//    vector<unsigned int>    itsShape;

    vector<Matrix>          itsValues;
};


class ExprResult
{
public:
    typedef shared_ptr<ExprResult>          Ptr;
    typedef shared_ptr<const ExprResult>    ConstPtr;

    // Flags.
    bool hasFlags() const
    {
        return itsFlags.initialized();
    }
    
    const FlagArray getFlags() const
    {
        return itsFlags;
    }

    void setFlags(const FlagArray &flags)
    {
        itsFlags = flags;
    }
    
    // Value.
    // TODO: Use ref counted proxy objects instead of shared_ptr (more elegant
    // interface).
    ValueSet::ConstPtr getValue() const
    {
        return itsValue;
    }
    
    void setValue(const ValueSet::ConstPtr &value)
    {
        itsValue = value;
    }
    
private:
    FlagArray           itsFlags;
    ValueSet::ConstPtr  itsValue;
//    map<PValueKey, ValueSet::Ptr> itsPValues;
};


class ExprValueBase
{
public:
    // Flags.
    bool hasFlags() const
    {
        return itsFlags.initialized();
    }

    void setFlags(const FlagArray &flags)
    {
        itsFlags = flags;
    }

    const FlagArray getFlags() const
    {
        return itsFlags;
    }

private:
    FlagArray   itsFlags;
};

//class Scalar: public ExprValueBase
//{
//public:
//    unsigned int size() const
//    {
//        return 1;
//    }

//    const Matrix &operator()() const
//    {
//        return itsValue;
//    }
//    
//    const Matrix &operator()(unsigned int) const
//    {
//        return itsValue;
//    }

//    void assign(const Matrix &value, const PValueKey &key = PValueKey())
//    {
//        itsValue[key] = value;
//    }
//    
//private:
//    Matrix                  itsValue;
//    map<PValueKey, Matrix>  itsValueMap;
//};

template <unsigned int LENGTH>
class Vector;

template <typename T>
class Proxy;

template <>
template <unsigned int LENGTH>
class Proxy<Vector<LENGTH> >
{
public:
    bool isDependent(unsigned int i0) const
    {
        return itsDepMask[i0];
    }
    
    const Matrix &operator()(unsigned int i0) const
    {
        return itsValue[i0];
    }

    void assign(unsigned int i0, const Matrix &value, bool dependent)
    {
        itsDepMask[i0] = dependent;
        itsValue[i0] = value;
    }

private:
    const Matrix    itsValue[LENGTH];
    bool            itsDepMask[LENGTH];
};

template <unsigned int LENGTH>
class Vector: public ExprValueBase
{
public:
    typedef Proxy<Vector<LENGTH> >          proxy;
    typedef const Proxy<Vector<LENGTH> >    const_proxy;
    
    unsigned int size() const
    {
        return LENGTH;
    }

//    const Matrix &operator()(unsigned int i0) const
//    {
//        return itsValue(i);
//    }

//    const Matrix &operator()(const PValueKey &key, unsigned int i0) const
//    {
//        map<PValueKey, Matrix>::const_iterator it = itsValueMap[i0].find(key);
//        return (it != itsValueMap
//        ASSERT(it != itsValueMap[i].end());
//        return it->second;
//    }

    const_proxy operator()(const PValueKey &key = PValueKey()) const
    {
        proxy result;
        
        if(!key.valid())
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                result.assign(i, itsValue[i], true);
            }            
        }
        else
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                map<PValueKey, Matrix>::const_iterator it =
                    itsValueMap[i].find(key);
                    
                if(it != itsValueMap[i].end())
                {                
                    result.assign(i, it->second, true);
                }
                else
                {
                    result.assign(i, itsValue[i], false);
                }
            }
        }
        
        return result;        
    }
    
    void assign(unsigned int i0, const Matrix &value)
    {
        itsValue[i0] = value;
    }
    
    void assign(const PValueKey &key, unsigned int i0, const Matrix &value)
    {
        itsValueMap[i0][key] = value;
    }
    
private:
    Matrix                  itsValue[LENGTH];
    map<PValueKey, Matrix>  itsValueMap[LENGTH];
};




//template <typename T>
//class ExprValueIterator;


//template <>
//template <unsigned int LENGTH>
//class ExprValueIterator<Vector<LENGTH> >
//{
//public:
//    typedef Proxy<Vector<LENGTH> >          proxy;
//    typedef const Proxy<Vector<LENGTH> >    const_proxy;
//    
//    ExprValueIterator(const Vector<LENGTH> &iterated)
//    {
//        for(unsigned int i = 0; i < LENGTH; ++i)
//        {
//            itsValue[i] = iterated(i);
//            itsIter[i] = iterated.map(i).begin();
//            itsEnd[i] = iterated.map(i).end();
//        }
//        update();
//    }
//    
//    const PValueKey &key() const
//    {
//        return itsMinKey;
//    }
//    
//    void operator++()
//    {
//        for(unsigned int i = 0; i < LENGTH; ++i)
//        {
//            if(itsIter[i] == itsMinKey)
//            {
//                ++itsIter[i];
//            }
//        }
//        
//        update();
//    }
//    
//    const_proxy operator*(unsigned int i0) const
//    {
//        return itsProxy;
//    }    
//    
//private:
//    void update()
//    {
//        itsAtEnd = true;
//        itsMinKey = PValueKey();
//        for(unsigned int i = 0; i < LENGTH; ++i)
//        {
//            if(itsIter[i] != itsEnd[i]
//                && itsIter[i]->first.parmId < itsMinKey.parmId)
//            {                
//                itsMinKey = itsIter[i]->first;
//                itsAtEnd = false;
//            }
//        }
//        
//        for(unsigned int i = 0; i < LENGTH; ++i)
//        {
//            if(itsIter[i] != itsEnd[i] && itsIter[i]->first == itsMinKey)
//            {
//                itsProxy.setValue(i, itsIter->second);
//                itsProxy.setDepMask(i, true);
//            }
//            else
//            {
//                itsProxy.setValue(i, itsValue[i]);
//                itsProxy.setDepMask(i, false);
//            }
//        }
//    }
//    
//    const Matrix                            itsValue[LENGTH]
//    map<PValueKey, Matrix>::const_iterator  itsIter[LENGTH];
//    map<PValueKey, Matrix>::const_iterator  itsEnd[LENGTH];
//    PValueKey                               itsMinKey;
//    proxy                                   itsProxy;
//    bool                                    itsAtEnd;
//};




//template <typename T_NUMERIC>
//class Scalar: public ExprResult
//{
//public:
//    typedef shared_ptr<Scalar<T_NUMERIC> >          Ptr;
//    typedef shared_ptr<const Scalar<T_NUMERIC> >    ConstPtr;

//    typedef T_NUMERIC   NumericType;
//    
////    bool isPerturbed() const
////    {
////        return itsPerturbedValueFlag;
////    }
//    
//    unsigned int size() const
//    {
//        return 1;
//    }

//    const ARRAY(T_NUMERIC) &value(unsigned int i = 0) const
//    {
//        assert(i == 0);
//        return itsValue;
//    }
//    
//    void assign(const ARRAY(T_NUMERIC) &value, bool isPerturbed = false)
//    {
////        itsPerturbedValueFlag = isPerturbed;
//        itsValue.reference(value);
//    }
//    
//    virtual size_t memory() const
//    {
//        return ExprResult::memory() + itsValue.size() * sizeof(T_NUMERIC);
//    }
//    
//private:
////    bool                itsPerturbedValueFlag;
//    ARRAY(T_NUMERIC)    itsValue;
//};


//template <typename T_NUMERIC, unsigned int SIZE = 2>
//class Vector: public ExprResult
//{
//public:
//    typedef shared_ptr<Vector<T_NUMERIC, SIZE> >        Ptr;
//    typedef shared_ptr<const Vector<T_NUMERIC, SIZE> >  ConstPtr;

//    typedef T_NUMERIC   NumericType;

////    bool isPerturbed(unsigned int i) const
////    {
////        assert(i < SIZE);
////        return itsPerturbedValueFlag[i];
////    }

//    unsigned int size() const
//    {
//        return SIZE;
//    }
//    
//    const ARRAY(T_NUMERIC) &value(unsigned int i) const
//    {
//        assert(i < SIZE);
//        return itsValue[i];
//    }
//    
//    void assign(unsigned int i, const ARRAY(T_NUMERIC) &value,
//        bool isPerturbed = false)
//    {
//        assert(i < SIZE);
////        itsPerturbedValueFlag[i] = isPerturbed;
//        itsValue[i].reference(value);
//    }
//    
//    virtual size_t memory() const
//    {
//        size_t mem = ExprResult::memory();
//        for(size_t i = 0; i < SIZE; ++i)
//        {
//            mem += itsValue[i].size() * sizeof(T_NUMERIC);
//        }
//        return mem;
//    }

//private:
////    bool                itsPerturbedValueFlag[SIZE];
//    ARRAY(T_NUMERIC)    itsValue[SIZE];
//};


//template <typename T_NUMERIC>
//class JonesMatrix: public ExprResult
//{
//public:
//    typedef shared_ptr<JonesMatrix<T_NUMERIC> >         Ptr;
//    typedef shared_ptr<const JonesMatrix<T_NUMERIC> >   ConstPtr;

//    typedef T_NUMERIC   NumericType;

////    bool isPerturbed(unsigned int i, unsigned int j) const
////    {
////        assert(i < 2 && j < 2);
////        return itsPerturbedValueFlag[i * 2 + j];
////    }

//    unsigned int size() const
//    {
//        return 4;
//    }
//    
//    const ARRAY(T_NUMERIC) &value(unsigned int i, unsigned int j) const
//    {
//        assert(i < 2 && j < 2);
//        return itsValue[i * 2 + j];
//    }
//    
//    const ARRAY(T_NUMERIC) &value(unsigned int i) const
//    {
//        assert(i < 4);
//        return itsValue[i];
//    }

//    void assign(unsigned int i, unsigned int j, const ARRAY(T_NUMERIC) &value,
//        bool isPerturbed = false)
//    {
//        assert(i < 2 && j < 2);
////        itsPerturbedValueFlag[i] = isPerturbed;
//        itsValue[i * 2 + j].reference(value);
//    }
//    
//    virtual size_t memory() const
//    {
//        return ExprResult::memory()
//            + sizeof(T_NUMERIC) * (itsValue[0].size() + itsValue[2].size()
//            + itsValue[3].size() + itsValue[4].size());
//    }
//    
//private:
////    bool                itsPerturbedValueFlag[2][2];
//    ARRAY(T_NUMERIC)    itsValue[4];
//};


//// @}

//class AsComplex: public Expr<Scalar<complex_t> >
//{
//public:
//    enum Inputs
//    {
//        RE,
//        IM,
//        N_Inputs
//    };

//    AsComplex()
//        :   Expr<Scalar<complex_t> >(N_Inputs)
//    {
//    }

//    AsComplex(const Expr<Scalar<real_t> >::ConstPtr &re,
//        const Expr<Scalar<real_t> >::ConstPtr &im)
//        :   Expr<Scalar<complex_t> >(N_Inputs)
//    {
//        connect(RE, re);
//        connect(IM, im);
//    }

//    virtual ExprBase::ConstResultPtr evaluateImpl(const Request &request,
//        const vector<ExprBase::ConstResultPtr> &inputs) const
////    virtual ResultType::ConstPtr evaluateImpl(const Request &request,
////        const PValueKey &key, bool perturbed) const
//    {
////        Expr<Scalar<real_t> >::ConstPtr inputRe =
////            static_pointer_cast<const Expr<Scalar<double> > >(getInput(RE));
////        Expr<Scalar<real_t> >::ConstPtr inputIm =
////            static_pointer_cast<const Expr<Scalar<double> > >(getInput(IM));

////        Scalar<real_t>::ConstPtr re =
////            inputRe->evaluate(request, key, perturbed);
////        Scalar<real_t>::ConstPtr im =
////            inputIm->evaluate(request, key, perturbed);

//        Scalar<real_t>::ConstPtr re =
//            static_pointer_cast<const Scalar<double> >(inputs[RE]);
//        Scalar<real_t>::ConstPtr im =
//            static_pointer_cast<const Scalar<double> >(inputs[IM]);

//        ResultType::Ptr result(new ResultType());
//        result->assign(ARRAY(complex_t)(blitz::zip(re->value(), im->value(),
//            complex_t())));

//        return result;        
//    }

//protected:
//    virtual void tryConnection(const ExprBase::ConstPtr &input, unsigned int)
//        const
//    {
//        assertInputType<Scalar<real_t> >(input);
//    }
//};


//template <typename T>
//class AsExpr;

//template <>
//template <typename T_NUMERIC, unsigned int LENGTH>
//class AsExpr<Vector<T_NUMERIC, LENGTH> >
//    :   public Expr<Vector<T_NUMERIC, LENGTH> >
//{
//    using Expr<Vector<T_NUMERIC, LENGTH> >::getInput;

//public:    
//    AsExpr()
//        :   Expr<Vector<T_NUMERIC, LENGTH> >(LENGTH)
//    {
//    }

//    virtual ExprBase::ConstResultPtr evaluateImpl(const Request &request,
//        const vector<ExprBase::ConstResultPtr> &inputs) const
////    virtual typename Expr<Vector<T_NUMERIC, LENGTH> >::ResultType::ConstPtr
////    evaluateImpl(const Request &request, const PValueKey &key, bool perturbed)
////        const
//    {
//        typename Expr<Vector<T_NUMERIC, LENGTH> >::ResultType::Ptr
//            result(new typename Expr<Vector<T_NUMERIC, LENGTH> >::ResultType());

//        for(size_t i = 0; i < LENGTH; ++i)
//        {
////            typename Expr<Scalar<T_NUMERIC> >::ConstPtr elementExpr =
////                static_pointer_cast<const Expr<Scalar<T_NUMERIC> > >
////                    (getInput(i));

////            typename Scalar<T_NUMERIC>::ConstPtr element =
////                elementExpr->evaluate(request, key, perturbed);

//            typename Scalar<T_NUMERIC>::ConstPtr element =
//                static_pointer_cast<const Scalar<T_NUMERIC> >(inputs[i]);

//            result->assign(i, element->value());
//        }
//        
//        return result;        
//    }

//protected:
//    virtual void tryConnection(const ExprBase::ConstPtr &input, unsigned int i)
//        const
//    {
//    }
//};


//template <>
//template <typename T_NUMERIC>
//class AsExpr<JonesMatrix<T_NUMERIC> >: public Expr<JonesMatrix<T_NUMERIC> >
//{
////    using typename Expr<JonesMatrix<T_NUMERIC> >::ResultType;
////    using ExprBase::assertInputType;

//public:    
//    enum Inputs
//    {
//        ELEMENT00,
//        ELEMENT01,
//        ELEMENT10,
//        ELEMENT11,
//        N_Inputs
//    };
//    
//    AsExpr()
//        :   Expr<JonesMatrix<T_NUMERIC> >(N_Inputs)
//    {
//    }
//            
//    virtual ExprBase::ConstResultPtr evaluateImpl(const Request &request,
//        const vector<ExprBase::ConstResultPtr> &inputs) const
////    virtual typename Expr<JonesMatrix<T_NUMERIC> >::ResultType::ConstPtr
////    evaluateImpl(const Request &request, const PValueKey &key, bool perturbed)
////        const
//    {
////        typename Expr<Scalar<T_NUMERIC> >::ConstPtr input00 =
////            static_pointer_cast<const Expr<Scalar<T_NUMERIC> > >
////                (getInput(ELEMENT00));
////        typename Expr<Scalar<T_NUMERIC> >::ConstPtr input01 =
////            static_pointer_cast<const Expr<Scalar<T_NUMERIC> > >
////                (getInput(ELEMENT01));
////        typename Expr<Scalar<T_NUMERIC> >::ConstPtr input10 =
////            static_pointer_cast<const Expr<Scalar<T_NUMERIC> > >
////                (getInput(ELEMENT10));
////        typename Expr<Scalar<T_NUMERIC> >::ConstPtr input11 =
////            static_pointer_cast<const Expr<Scalar<T_NUMERIC> > >
////                (getInput(ELEMENT11));

////        typename Scalar<T_NUMERIC>::ConstPtr element00 =
////            input00->evaluate(request, key, perturbed);
////        typename Scalar<T_NUMERIC>::ConstPtr element01 =
////            input10->evaluate(request, key, perturbed);
////        typename Scalar<T_NUMERIC>::ConstPtr element10 =
////            input01->evaluate(request, key, perturbed);
////        typename Scalar<T_NUMERIC>::ConstPtr element11 =
////            input11->evaluate(request, key, perturbed);

//        typename Scalar<T_NUMERIC>::ConstPtr element00 =
//            static_pointer_cast<const Scalar<T_NUMERIC> >(inputs[ELEMENT00]);
//        typename Scalar<T_NUMERIC>::ConstPtr element01 =
//            static_pointer_cast<const Scalar<T_NUMERIC> >(inputs[ELEMENT01]);
//        typename Scalar<T_NUMERIC>::ConstPtr element10 =
//            static_pointer_cast<const Scalar<T_NUMERIC> >(inputs[ELEMENT10]);
//        typename Scalar<T_NUMERIC>::ConstPtr element11 =
//            static_pointer_cast<const Scalar<T_NUMERIC> >(inputs[ELEMENT11]);

//        typename Expr<JonesMatrix<T_NUMERIC> >::ResultType::Ptr
//            result(new typename Expr<JonesMatrix<T_NUMERIC> >::ResultType());
//        result->assign(0, 0, element00->value());
//        result->assign(0, 1, element01->value());
//        result->assign(1, 0, element10->value());
//        result->assign(1, 1, element11->value());

//        return result;        
//    }

//protected:
//    virtual void tryConnection(const ExprBase::ConstPtr &input, unsigned int i)
//        const
//    {
//    }
//};

} //# namespace BBS
} //# namespace LOFAR

#endif
