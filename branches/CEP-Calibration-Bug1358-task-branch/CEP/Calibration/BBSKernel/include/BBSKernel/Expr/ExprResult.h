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

// A key that identifies a (perturbed) value through its associated
// (parameter id, coefficient id) combination.
// TODO: Rename to ValueKey / CoeffKey / ParmKey?
// TODO: Merge parmId/coeffId parts into a single uint64 (faster comparisons?)
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


// This class represents a 2-D scalar field (the default axes are frequency
// and time). Multiple instances of the scalar field can be carried along,
// linked to different (parameter, coefficient) pairs. This can be used
// to hold e.g. the value of the scalar field perturbed for a (parameter,
// coefficient) pair, or the value of the partial derivative with respect to a
// (parameter, coefficient) pair.
class FieldSetImpl: public RefCountable
{
public:
    typedef map<PValueKey, Matrix>::iterator        iterator;
    typedef map<PValueKey, Matrix>::const_iterator  const_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    const Matrix &value() const;
    const Matrix &value(const PValueKey &key, bool &found) const;
    Matrix &value();
    Matrix &value(const PValueKey &key);

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);

private:
    Matrix                  itsData;
    map<PValueKey, Matrix>  itsDepData;
};


// Reference counting proxy class that manages a FieldSetImpl.
class FieldSet: public RefCounted<FieldSetImpl>
{
public:
    typedef FieldSetImpl::iterator          iterator;
    typedef FieldSetImpl::const_iterator    const_iterator;

    FieldSet();

    const_iterator begin() const;
    const_iterator end() const;
//    iterator begin();
//    iterator end();

    const Matrix &value() const;
    // Returns the field associated with "key", or the unkeyed (main) field if
    // not found.
    const Matrix &value(const PValueKey &key) const;
    // Returns the field associated with "key", or the unkeyed (main) field if
    // not found. The "found" argument is indicates if a field associated with
    // "key" was found.
    const Matrix &value(const PValueKey &key, bool &found) const;

//    Matrix &value();
//    // Returns a writeable reference to the field associated with "key". If no
//    // such field exists yet it will be created.
//    Matrix &value(const PValueKey &key);

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);
};


// Class describing the shape (2-D) of the result of an expression.
class Shape
{
public:
    Shape()
        :   itsRank(0)
    {
        itsShape[0] = itsShape[1] = 0;
    }

    Shape(unsigned int l0)
        :   itsRank(1)
    {
        itsShape[0] = l0;
        itsShape[1] = 0;
    }

    Shape(unsigned int l0, unsigned int l1)
        :   itsRank(2)
    {
        itsShape[0] = l0;
        itsShape[1] = l1;
    }

    unsigned int rank() const
    {
        return itsRank;
    }

    unsigned int operator[](unsigned int dim) const
    {
        DBGASSERT(dim < rank());
        return itsShape[dim];
    }

    bool operator==(const Shape &rhs) const
    {
        return (itsRank == rhs.itsRank && itsShape[0] == rhs.itsShape[0]
            && itsShape[1] == rhs.itsShape[1]);
    }

private:
    unsigned int    itsRank;
    unsigned int    itsShape[2];
};

// Helper function to compute the number of elements implied by a given Shape.
inline unsigned int size(const Shape &shape)
{
    unsigned int size = 1;
    for(unsigned int i = 0; i < shape.rank(); ++i)
    {
        size *= shape[i];
    }

    return size;
}

// Forward declaration.
class ExprValueSet;

// Proxy class that simplifies operating on ExprValueSet instances. Each element
// (of type FieldSet) of an ExprValueSet instance can carry along any number of
// perturbed values (of type Matrix) linked to specific (parameter, coefficient)
// pairs, or keys.
//
// When computing the value of an expression perturbed for a specific key, some
// elements (FieldSets) may contain a value for that key whereas others may not
// (in which case the non-perturbed value should be used instead).
//
// ExprValue instances provide a view on an ExprValueSet for a specific key.
// For example, let A be a Jones matrix represented by an ExprValueSet instance
// with Shape (2, 2). Suppose we want to compute the value of an expression
// perturbed for key (1, 3). Furthermore, suppose that the diagonal elements of
// A have a value associated with key (1, 3) whereas the off-diagonal elements
// do not. In other words, the value of the diagonal elements of A depends on
// the third coefficient of parameter 1 whereas the off-diagonal elements do not
// depend on this coefficient. We can create an ExprValue instance and populate
// it with the perturbed values (lined to key (1, 3)) on the diagonal and the
// appropriate non-perturbed values on the off-diagonal. Thus we have created an
// object represents the value of A for key (1, 3). It has the same Shape as A,
// and can be indexed as an array with that Shape.
//
// The isDependent() methods can be used to check if an element of an ExprValue
// instance points to a linked (perturbed) value or to the non-perturbed value
// of the corresponding FieldSet. This can be used to avoid unnecessary
// computation (see for example Mul::evaluateImpl).
//
// The point is that an ExprValue instance is much easier to work with and leads
// to much cleaner code compared to working with an ExprValueSet instance
// directly.
class ExprValue
{
public:
    ExprValue()
    {
    }

    ExprValue(const Shape &shape)
        :   itsShape(shape)
    {
        const unsigned int size = LOFAR::BBS::size(shape);
        itsDepMask.resize(size);
        itsData.resize(size);
    }

    // Clear the ExprValue instance. This allows an ExprValue instance to be
    // re-used to avoid excessive (re)allocation.
    void clear()
    {
        fill(itsDepMask.begin(), itsDepMask.end(), false);
        fill(itsData.begin(), itsData.end(), Matrix());
    }

    unsigned int size() const
    {
        return itsData.size();
    }

    unsigned int rank() const
    {
        return itsShape.rank();
    }

    const Shape &shape() const
    {
        return itsShape;
    }

    // Methods to check if an element points to a Matrix that was linked to a
    // key in the corresponding FieldSet (and therefore depends on the
    // coefficient identified by that key) or that it points to a Matrix that
    // is independent of that coefficient.
    // @{
    bool isDependent() const
    {
        DBGASSERT(rank() == 0 && size() == 1);
        return itsDepMask[0];
    }

    bool isDependent(unsigned int i0) const
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        return itsDepMask[i0];
    }

    bool isDependent(unsigned int i1, unsigned int i0) const
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        return itsDepMask[i1 * itsShape[0] + i0];
    }
    // @}

    // Element access.
    // @{
    const Matrix &operator()() const
    {
        DBGASSERT(rank() == 0 && size() == 1);
        return itsData[0];
    }

    const Matrix &operator()(unsigned int i0) const
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        return itsData[i0];
    }

    const Matrix &operator()(unsigned int i1, unsigned int i0) const
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        return itsData[i1 * itsShape[0] + i0];
    }

    void assign(const Matrix &value, bool dependent = true)
    {
        DBGASSERT(rank() == 0 && size() == 1);
        itsData[0] = value;
        itsDepMask[0] = dependent;
    }

    void assign(unsigned int i0, const Matrix &value, bool dependent = true)
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        itsData[i0] = value;
        itsDepMask[i0] = dependent;
    }

    void assign(unsigned int i1, unsigned int i0, const Matrix &value,
        bool dependent = true)
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        itsData[i1 * itsShape[0] + i0] = value;
        itsDepMask[i1 * itsShape[0] + i0] = dependent;
    }
    // @}

private:
    friend class ExprValueSet;

    // Private methods used by ExprValueSet to populate an ExprValue instance in
    // a generic way (independent of the rank) by using a flat (1-D) index.
    const Matrix &flat_access(unsigned int i) const
    {
        ASSERT(i < size());
        return itsData[i];
    }

    void flat_assign(unsigned int i, const Matrix &value, bool dependent = true)
    {
        ASSERT(i < size());
        itsData[i] = value;
        itsDepMask[i] = dependent;
    }

    Shape           itsShape;
    vector<bool>    itsDepMask;
    vector<Matrix>  itsData;
};


// This class holds the value of an expression and a perturbed value for each of
// the solvable coefficients that the expression depends on.
class ExprValueSet
{
public:
    ExprValueSet()
    {
    }

    ExprValueSet(const Shape &shape)
        :   itsShape(shape)
    {
        const unsigned int size = LOFAR::BBS::size(shape);
        itsSets.reserve(size);
        for(unsigned int i = 0; i < size; ++i)
        {
            itsSets.push_back(FieldSet());
        }
    }

    // Resize the ExprValueSet. All the elements will point to newly allocated
    // FieldSet instances.
    void resize(const Shape &shape)
    {
        itsShape = shape;
        itsSets.clear();

        const unsigned int size = LOFAR::BBS::size(shape);
        itsSets.reserve(size);
        for(unsigned int i = 0; i < size; ++i)
        {
            itsSets.push_back(FieldSet());
        }
    }

    unsigned int size() const
    {
        return itsSets.size();
    }

    unsigned int rank() const
    {
        return itsShape.rank();
    }

    const Shape &shape() const
    {
        return itsShape;
    }

    // Flags.
    bool hasFlags() const
    {
        return itsFlags.initialized();
    }

    const FlagArray &flags() const
    {
        return itsFlags;
    }

    void assignFlags(const FlagArray &flags)
    {
        itsFlags = flags;
    }

    // ExprValue access.
    // @{
    ExprValue value() const
    {
        ExprValue value(shape());
        for(unsigned int i = 0; i < size(); ++i)
        {
            value.flat_assign(i, itsSets[i].value());
        }

        return value;
    }

    ExprValue value(const PValueKey &key) const
    {
        ExprValue value(shape());
        for(unsigned int i = 0; i < size(); ++i)
        {
            bool dependent;
            const Matrix &m = itsSets[i].value(key, dependent);
            value.flat_assign(i, m, dependent);
        }

        return value;
    }

    void assign(const ExprValue &value)
    {
        ASSERT(value.size() == size());
        for(unsigned int i = 0; i < value.size(); ++i)
        {
            if(!value.flat_access(i).isNull())
            {
                itsSets[i].assign(value.flat_access(i));
            }
        }
    }

    void assign(const PValueKey &key, const ExprValue &value)
    {
        ASSERT(value.size() == size());
        for(unsigned int i = 0; i < value.size(); ++i)
        {
            if(!value.flat_access(i).isNull())
            {
                itsSets[i].assign(key, value.flat_access(i));
            }
        }
    }
    // @}

    // Non-perturbed value access.
    // @{
    const Matrix &operator()() const
    {
        DBGASSERT(rank() == 0 && size() == 1);
        return itsSets[0].value();
    }

    const Matrix &operator()(unsigned int i0) const
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        return itsSets[i0].value();
    }

    const Matrix &operator()(unsigned int i1, unsigned int i0) const
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        return itsSets[i1 * itsShape[0] + i0].value();
    }

    void assign(const Matrix &value)
    {
        DBGASSERT(rank() == 0 && size() == 1);
        itsSets[0].assign(value);
    }

    void assign(unsigned int i0, const Matrix &value)
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        itsSets[i0].assign(value);
    }

    void assign(unsigned int i1, unsigned int i0, const Matrix &value)
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        return itsSets[i1 * itsShape[0] + i0].assign(value);
    }
    // @}

    // Perturbed value access.
    // @{
    const Matrix &operator()(const PValueKey &key) const
    {
        DBGASSERT(rank() == 0 && size() == 1);
        return itsSets[0].value(key);
    }

    const Matrix &operator()(const PValueKey &key, unsigned int i0) const
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        return itsSets[i0].value(key);
    }

    const Matrix &operator()(const PValueKey &key, unsigned int i1,
        unsigned int i0) const
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        return itsSets[i1 * itsShape[0] + i0].value(key);
    }

    void assign(const PValueKey &key, const Matrix &value)
    {
        DBGASSERT(rank() == 0 && size() == 1);
        itsSets[0].assign(key, value);
    }

    void assign(const PValueKey &key, unsigned int i0, const Matrix &value)
    {
        DBGASSERT(rank() == 1 && i0 < itsShape[0]);
        itsSets[i0].assign(key, value);
    }

    void assign(const PValueKey &key, unsigned int i1, unsigned int i0,
        const Matrix &value)
    {
        DBGASSERT(rank() == 2 && i0 < itsShape[0] && i1 < itsShape[1]);
        itsSets[i1 * itsShape[0] + i0].assign(key, value);
    }
    // @}

    // FieldSet access. This is mainly used to "pass-through" the value and all
    // perturbed values of an element in one go (see for example the
    // implementation of Zip::evaluate or ExprParm::evaluate). Only flat (1-D)
    // indexing is provided at the moment (can easily be extended if and when
    // necessary).
    // @{
    const FieldSet &getFieldSet(unsigned int i) const
    {
        DBGASSERT(i < size());
        return itsSets[i];
    }

    void setFieldSet(unsigned int i, const FieldSet &set)
    {
        DBGASSERT(i < size());
        itsSets[i] = set;
    }
    // @}

private:
    Shape               itsShape;
    FlagArray           itsFlags;
    vector<FieldSet>    itsSets;
};


// -------------------------------------------------------------------------- //
// - Implementation: PValueKey                                              - //
// -------------------------------------------------------------------------- //

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


// -------------------------------------------------------------------------- //
// - Implementation: FieldSetImpl                                           - //
// -------------------------------------------------------------------------- //

inline FieldSetImpl::const_iterator FieldSetImpl::begin() const
{
    return itsDepData.begin();
}

inline FieldSetImpl::const_iterator FieldSetImpl::end() const
{
    return itsDepData.end();
}

inline FieldSetImpl::iterator FieldSetImpl::begin()
{
    return itsDepData.begin();
}

inline FieldSetImpl::iterator FieldSetImpl::end()
{
    return itsDepData.end();
}

inline const Matrix &FieldSetImpl::value() const
{
    return itsData;
}

inline const Matrix &FieldSetImpl::value(const PValueKey &key, bool &found)
    const
{
    map<PValueKey, Matrix>::const_iterator it = itsDepData.find(key);
    found = (it != itsDepData.end());
    return (found ? it->second : itsData);
}

inline Matrix &FieldSetImpl::value()
{
    return itsData;
}

inline Matrix &FieldSetImpl::value(const PValueKey &key)
{
    return itsDepData[key];
}

inline void FieldSetImpl::assign(const Matrix &value)
{
    itsData = value;
}

inline void FieldSetImpl::assign(const PValueKey &key, const Matrix &value)
{
    itsDepData[key] = value;
}


// -------------------------------------------------------------------------- //
// - Implementation: FieldSet                                               - //
// -------------------------------------------------------------------------- //

inline FieldSet::FieldSet()
    :   RefCounted<FieldSetImpl>(new FieldSetImpl())
{
}

//inline void FieldSet::initialize()
//{
//    reset(new FieldSetImpl());
//}

//inline FieldSet::iterator FieldSet::begin()
//{
//    return instance().begin();
//}

//inline FieldSet::iterator FieldSet::end()
//{
//    return instance().end();
//}

inline FieldSet::const_iterator FieldSet::begin() const
{
    return instance().begin();
}

inline FieldSet::const_iterator FieldSet::end() const
{
    return instance().end();
}

inline const Matrix &FieldSet::value() const
{
    return instance().value();
}

inline const Matrix &FieldSet::value(const PValueKey &key) const
{
    bool tmp;
    return instance().value(key, tmp);
}

inline const Matrix &FieldSet::value(const PValueKey &key, bool &found) const
{
    return instance().value(key, found);
}

//inline Matrix &FieldSet::value()
//{
//    return instance().value();
//}

//inline Matrix &FieldSet::value(const PValueKey &key)
//{
//    return instance().value(key);
//}

inline void FieldSet::assign(const Matrix &value)
{
    instance().assign(value);
}

inline void FieldSet::assign(const PValueKey &key, const Matrix &value)
{
    return instance().assign(key, value);
}

} //# namespace BBS
} //# namespace LOFAR

#endif
