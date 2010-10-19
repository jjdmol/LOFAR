//# Result.h: The result of the evaluation of an expression on a given request
//# grid.
//#
//# Copyright (C) 2008
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

#ifndef EXPR_RESULT_H
#define EXPR_RESULT_H

// \file
// The result of the evaluation of an expression on a given request grid.

#include <BBSKernel/Expr/Matrix.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

#include <limits>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// A key that identifies a perturbed value through its associated (parameter id,
// coefficient id) combination.
class PValueKey
{
public:
    PValueKey(size_t parmId, size_t coeffId);

    bool operator<(const PValueKey &other) const;
    bool operator==(const PValueKey &other) const;

    size_t  parmId, coeffId;
};

// Result of the evaluation of an expression on a given request grid. This is a
// representation class that contains the actual result data and provides an
// internal reference count.
class ResultRep
{
public:
    ResultRep();
    ~ResultRep();

    // Support for reference counting (NOT THREAD SAFE).
    ResultRep *link();
    static void unlink(ResultRep *rep);

    // Access the main result value.
    void setValue(const Matrix &value);
    const Matrix &getValue() const;
    Matrix &getValueRW();

    // Access a perturbed value associated with a certain (parameter,
    // coefficient) combination.
    void setPerturbedValue(const PValueKey &key, const Matrix &value);
    const Matrix &getPerturbedValue(const PValueKey &key) const;
    Matrix &getPerturbedValueRW(const PValueKey &key);

    // Find out if this result contains a perturbed value for the given
    // (parameter, coefficient) combination.
    bool isPerturbed(const PValueKey &key) const;

    // Remove all perturbed values.
    void clearPerturbedValues();

    // Get the number of perturbed values available.
    size_t getPerturbedValueCount() const;
    
private:
    // Forbid copy and assignment.
    ResultRep(const ResultRep&);
    ResultRep &operator=(const ResultRep&);

    // Allow iterators access to private attributes.
    friend class PValueIterator;
    friend class PValueConstIterator;

    typedef map<PValueKey, size_t>  PValueMap;

    Matrix          itsValue;
    vector<Matrix>  itsPValues;
    PValueMap       itsPValueMap;

    size_t          itsRefCount;
};

// A proxy class that manages a ResultRep class. Note that the default
// constructor creates a Result WITHOUT creating the underlying
// ResultRep object. This behaviour is mainly useful when passing the
// Result instance directly to Expr::getResult(Vec)Synced(). Before
// calling any methods on Result, init() should be called first to construct
// the underlying ResultRep object.
class Result
{
public:
    // NB: The default constructor DOES NOT allocate an underlying ResultRep
    // instance.
    Result();
    ~Result();

    Result(const Result &other);
    Result &operator=(const Result &other);

    // Allocate the underlying ResultRep object.
    void init();

    // Access the main result value.
    void setValue(const Matrix &value);
    const Matrix &getValue() const;
    Matrix &getValueRW();

    // Access a perturbed value associated with a certain (parameter,
    // coefficient) combination.
    void setPerturbedValue(const PValueKey &key, const Matrix &value);
    const Matrix &getPerturbedValue(const PValueKey &key) const;
    Matrix &getPerturbedValueRW(const PValueKey &key);
    
    // Find out if this result contains a perturbed value for the given
    // (parameter, coefficient) combination.
    bool isPerturbed(const PValueKey &key) const;

    // Remove all perturbed values.
    void clearPerturbedValues();

    // Get the number of perturbed values available.
    size_t getPerturbedValueCount() const;
    
private:
    // Allow iterators access to the underlying ResultRep instance.
    friend class PValueConstIterator;
    friend class PValueIterator;
    
    ResultRep    *itsRep;
};

// -------------------------------------------------------------------------- //
// IMPLEMENTATION                                                             //
// -------------------------------------------------------------------------- //

inline PValueKey::PValueKey(size_t parmId, size_t coeffId)
    :   parmId(parmId),
        coeffId(coeffId)
{
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

inline ResultRep *ResultRep::link()
{
    ++itsRefCount;
    return this;
}

inline void ResultRep::unlink(ResultRep *rep)
{
    if(rep != 0 && --rep->itsRefCount == 0)
    {
        delete rep;
    }
}        

inline void ResultRep::setValue(const Matrix &value)
{ itsValue = value; }

inline const Matrix &ResultRep::getValue() const
{ return itsValue; }

inline Matrix &ResultRep::getValueRW()
{ return itsValue; }

inline size_t ResultRep::getPerturbedValueCount() const
{ return itsPValues.size(); }

inline void Result::setValue(const Matrix &value)
{
    ASSERT(itsRep);
    itsRep->setValue(value);
}

inline const Matrix &Result::getValue() const
{
    ASSERT(itsRep);
    return itsRep->getValue();
}

inline Matrix &Result::getValueRW()
{
    ASSERT(itsRep);
    return itsRep->getValueRW();
}

inline void Result::setPerturbedValue(const PValueKey &key,
    const Matrix &value)
{
    ASSERT(itsRep);
    itsRep->setPerturbedValue(key, value);
}

inline const Matrix &Result::getPerturbedValue(const PValueKey &key) const
{
    ASSERT(itsRep);
    return itsRep->getPerturbedValue(key);
}

inline Matrix &Result::getPerturbedValueRW(const PValueKey &key)
{
    ASSERT(itsRep);
    return itsRep->getPerturbedValueRW(key);
}
    
inline bool Result::isPerturbed(const PValueKey &key) const
{
    ASSERT(itsRep);
    return itsRep->isPerturbed(key);
}

inline void Result::clearPerturbedValues()
{
    ASSERT(itsRep);
    itsRep->clearPerturbedValues();
}

inline size_t Result::getPerturbedValueCount() const
{
    ASSERT(itsRep);
    return itsRep->getPerturbedValueCount();
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
