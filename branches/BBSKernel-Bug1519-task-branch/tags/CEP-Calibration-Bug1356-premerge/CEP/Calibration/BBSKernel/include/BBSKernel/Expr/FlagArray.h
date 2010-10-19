//# FlagArray.h: An array of flags that can transparently handle rank 0 (scalar)
//# rank 2 (matrix) flag data.
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

#ifndef LOFAR_BBSKERNEL_EXPR_FLAGARRAY_H
#define LOFAR_BBSKERNEL_EXPR_FLAGARRAY_H

// \file
// An array of flags that can transparently handle rank 0 (scalar) rank 2
// (matrix) flag data.

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_algorithm.h>
#include <BBSKernel/Expr/RefCounting.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// Typedef that can be used to globally change the numeric type used for flags.
// TODO: Make a templated version of FlagArray?
typedef uint8 FlagType;

class FlagArray;
class FlagArrayImplScalar;
class FlagArrayImplMatrix;

// iotream I/O.
ostream &operator<<(ostream &out, const FlagArray &obj);

// The base implementation class (reference counting "enabled") that is
// referenced by a FlagArray(Temporary) instance. It hides the dimensionality
// (rank) of the underlying data (which is either rank 0 or rank 2).
class FlagArrayImpl: public RefCountable
{
public:
    typedef FlagType*       iterator;
    typedef const FlagType* const_iterator;

    FlagArrayImpl();
    virtual ~FlagArrayImpl();

    virtual FlagArrayImpl *clone() const = 0;
    virtual unsigned int rank() const = 0;
    virtual unsigned int shape(unsigned int n) const = 0;
    virtual unsigned int size() const = 0;

    virtual FlagType value(unsigned int i0, unsigned int i1) const = 0;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual iterator begin() = 0;
    virtual iterator end() = 0;

    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const = 0;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const = 0;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const = 0;

    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const = 0;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const = 0;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const = 0;
};

// Class to represent temporary FlagArray instances. All operations on an
// instance of FlagArrayTemporary will be performed _in place_. The only purpose
// of this class is to avoid the allocation and copying overhead incurred by
// the creation of temporaries in an expression like: a | b | c & d.
class FlagArrayTemporary: public RefCounted<FlagArrayImpl>
{
public:
    FlagArrayTemporary();

    // NB. This constructor is not prefixed with the "explicit" keyword, such
    // that it is possible to use literals in an expression (e.g. 3 | a).
    FlagArrayTemporary(const FlagType &value);

    FlagArrayTemporary(unsigned int l0, unsigned int l1);
    FlagArrayTemporary(unsigned int l0, unsigned int l1, const FlagType &value);

private:
    FlagArrayTemporary(FlagArrayImpl *impl);

    // Non-member functions are used to allow the use of literals on both sides
    // of the operator, e.g. both 3 | a and a | 3 will work (see "Effective
    // C++" (Meyers), third edition, Item 24.
    //
    // Note that the (FlagArray, FlagArray) variant is _only_ included here
    // because it needs to call FlagArrayTemporary(FlagArrayImpl *impl). It
    // obviously does not need to call FlagArrayTemporary::instance().
    friend const FlagArrayTemporary operator|(const FlagArray &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator|(const FlagArray &lhs,
        const FlagArrayTemporary &rhs);
    friend const FlagArrayTemporary operator|(const FlagArrayTemporary &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator|(const FlagArrayTemporary &lhs,
        const FlagArrayTemporary &rhs);

    friend const FlagArrayTemporary operator&(const FlagArray &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator&(const FlagArray &lhs,
        const FlagArrayTemporary &rhs);
    friend const FlagArrayTemporary operator&(const FlagArrayTemporary &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator&(const FlagArrayTemporary &lhs,
        const FlagArrayTemporary &rhs);

    friend class FlagArray;
};

class FlagArray: public RefCounted<FlagArrayImpl>
{
public:
    typedef FlagArrayImpl::iterator         iterator;
    typedef FlagArrayImpl::const_iterator   const_iterator;

    FlagArray();
    explicit FlagArray(const FlagType &value);

    // FlagArray uses a column-major memory layout (FORTRAN order) to be
    // consistent with the Matrix class.
    FlagArray(unsigned int l0, unsigned int l1);
    FlagArray(unsigned int l0, unsigned int l1, const FlagType &value);

    // Construct a FlagArray from a temporary (reference semantics).
    FlagArray(const FlagArrayTemporary &other);

    FlagArray clone() const;
    unsigned int rank() const;
    unsigned int shape(unsigned int n) const;
    unsigned int size() const;

    // FlagArray uses a column-major memory layout (FORTRAN order) to be
    // consistent with the Matrix class. In particular, i0 (the index belonging
    // to first dimension) is the fastest running dimension.
    FlagType operator()(unsigned int i0, unsigned int i1) const;

    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();

    void operator|=(const FlagArray &rhs);
    void operator|=(const FlagArrayTemporary &rhs);

    void operator&=(const FlagArray &rhs);
    void operator&=(const FlagArrayTemporary &rhs);

private:
    // Construct a FlagArray from a FlagArrayImpl pointer.
    FlagArray(FlagArrayImpl *impl);

    // Non-member functions are used to allow the use of literals on both sides
    // of the operator, e.g. both 3 | a and a | 3 will work (see "Effective
    // C++" (Meyers), third edition, Item 24.
    friend const FlagArrayTemporary operator|(const FlagArray &lhs,
        const FlagArrayTemporary &rhs);
    friend const FlagArrayTemporary operator|(const FlagArrayTemporary &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator|(const FlagArray &lhs,
        const FlagArray &rhs);

    friend const FlagArrayTemporary operator&(const FlagArray &lhs,
        const FlagArrayTemporary &rhs);
    friend const FlagArrayTemporary operator&(const FlagArrayTemporary &lhs,
        const FlagArray &rhs);
    friend const FlagArrayTemporary operator&(const FlagArray &lhs,
        const FlagArray &rhs);
};

// Class that holds rank 0 (scalar) flag data.
class FlagArrayImplScalar: public FlagArrayImpl
{
public:
    FlagArrayImplScalar();
    explicit FlagArrayImplScalar(const FlagType &value);

    virtual FlagArrayImpl *clone() const;

    virtual unsigned int rank() const;
    virtual unsigned int shape(unsigned int n) const;
    virtual unsigned int size() const;

    virtual FlagType value(unsigned int i0, unsigned int i1) const;

    virtual const_iterator begin() const;
    virtual const_iterator end() const;
    virtual iterator begin();
    virtual iterator end();

    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const;

    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const;

    const FlagType &value() const;
    FlagType &value();

private:
    FlagType    itsData;
};

// Class that holds rank 2 (matrix) flag data.
class FlagArrayImplMatrix: public FlagArrayImpl
{
public:
    FlagArrayImplMatrix(unsigned int l0, unsigned int l1);
    FlagArrayImplMatrix(unsigned int l0, unsigned int l1,
        const FlagType &value);
    virtual ~FlagArrayImplMatrix();

    FlagArrayImplMatrix(const FlagArrayImplMatrix &other);
    virtual FlagArrayImpl *clone() const;

    virtual unsigned int rank() const;
    virtual unsigned int shape(unsigned int n) const;
    virtual unsigned int size() const;

    virtual FlagType value(unsigned int i0, unsigned int i1) const;

    virtual const_iterator begin() const;
    virtual const_iterator end() const;
    virtual iterator begin();
    virtual iterator end();

    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseOr(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const;

    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImpl &rhs, bool lhsTmp,
        bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplScalar &lhs,
        bool lhsTmp, bool rhsTmp) const;
    virtual FlagArrayImpl *opBitWiseAnd(const FlagArrayImplMatrix &lhs,
        bool lhsTmp, bool rhsTmp) const;

private:
    unsigned int        itsShape[2];
    unsigned int        itsSize;
    FlagType            *itsData;
};


// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayImpl                                          - //
// -------------------------------------------------------------------------- //

inline FlagArrayImpl::FlagArrayImpl()
    :   RefCountable()
{
}

inline FlagArrayImpl::~FlagArrayImpl()
{
}

// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayTemporary                                     - //
// -------------------------------------------------------------------------- //

inline FlagArrayTemporary::FlagArrayTemporary()
    :   RefCounted<FlagArrayImpl>()
{
}

inline FlagArrayTemporary::FlagArrayTemporary(const FlagType &value)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplScalar(value))
{
}

inline FlagArrayTemporary::FlagArrayTemporary(unsigned int l0,
    unsigned int l1)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplMatrix(l0, l1))
{
}

inline FlagArrayTemporary::FlagArrayTemporary(unsigned int l0,
    unsigned int l1, const FlagType &value)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplMatrix(l0, l1, value))
{
}

inline FlagArrayTemporary::FlagArrayTemporary(FlagArrayImpl *impl)
    :   RefCounted<FlagArrayImpl>(impl)
{
}

// -------------------------------------------------------------------------- //
// - Implementation: Non-member functions                                   - //
// -------------------------------------------------------------------------- //

inline const FlagArrayTemporary operator|(const FlagArray &lhs,
    const FlagArray &rhs)
{
    return lhs.instance().opBitWiseOr(rhs.instance(), false, false);
}

inline const FlagArrayTemporary operator|(const FlagArrayTemporary &lhs,
    const FlagArray &rhs)
{
    return lhs.instance().opBitWiseOr(rhs.instance(), true, false);
}

inline const FlagArrayTemporary operator|(const FlagArray &lhs,
    const FlagArrayTemporary &rhs)
{
    return lhs.instance().opBitWiseOr(rhs.instance(), false, true);
}

inline const FlagArrayTemporary operator|(const FlagArrayTemporary &lhs,
    const FlagArrayTemporary &rhs)
{
    return lhs.instance().opBitWiseOr(rhs.instance(), true, true);
}

inline const FlagArrayTemporary operator&(const FlagArray &lhs,
    const FlagArray &rhs)
{
    return lhs.instance().opBitWiseAnd(rhs.instance(), false, false);
}

inline const FlagArrayTemporary operator&(const FlagArrayTemporary &lhs,
    const FlagArray &rhs)
{
    return lhs.instance().opBitWiseAnd(rhs.instance(), true, false);
}

inline const FlagArrayTemporary operator&(const FlagArray &lhs,
    const FlagArrayTemporary &rhs)
{
    return lhs.instance().opBitWiseAnd(rhs.instance(), false, true);
}

inline const FlagArrayTemporary operator&(const FlagArrayTemporary &lhs,
    const FlagArrayTemporary &rhs)
{
    return lhs.instance().opBitWiseAnd(rhs.instance(), true, true);
}

// -------------------------------------------------------------------------- //
// - Implementation: FlagArray                                              - //
// -------------------------------------------------------------------------- //

inline FlagArray::FlagArray()
    :   RefCounted<FlagArrayImpl>()
{
}

inline FlagArray::FlagArray(const FlagType &value)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplScalar(value))
{
}

inline FlagArray::FlagArray(unsigned int l0, unsigned int l1)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplMatrix(l0, l1))
{
}

inline FlagArray::FlagArray(unsigned int l0, unsigned int l1,
    const FlagType &value)
    :   RefCounted<FlagArrayImpl>(new FlagArrayImplMatrix(l0, l1, value))
{
}

inline FlagArray::FlagArray(const FlagArrayTemporary &other)
    :   RefCounted<FlagArrayImpl>(other)
{
}

inline FlagArray::FlagArray(FlagArrayImpl *impl)
    :   RefCounted<FlagArrayImpl>(impl)
{
}

inline FlagArray FlagArray::clone() const
{
    return initialized() ? FlagArray(instance().clone()) : FlagArray();
}

inline unsigned int FlagArray::rank() const
{
    return instance().rank();
}

inline unsigned int FlagArray::shape(unsigned int n) const
{
    return instance().shape(n);
}

inline unsigned int FlagArray::size() const
{
    return instance().size();
}

inline FlagType FlagArray::operator()(unsigned int i0, unsigned int i1) const
{
    return instance().value(i0, i1);
}

inline FlagArray::const_iterator FlagArray::begin() const
{
    return instance().begin();
}

inline FlagArray::const_iterator FlagArray::end() const
{
    return instance().end();
}

inline FlagArray::iterator FlagArray::begin()
{
    return instance().begin();
}

inline FlagArray::iterator FlagArray::end()
{
    return instance().end();
}

inline void FlagArray::operator|=(const FlagArray &rhs)
{
    instance().opBitWiseOr(rhs.instance(), true, false);
}

inline void FlagArray::operator|=(const FlagArrayTemporary &rhs)
{
    instance().opBitWiseOr(rhs.instance(), true, true);
}

inline void FlagArray::operator&=(const FlagArray &rhs)
{
    instance().opBitWiseAnd(rhs.instance(), true, false);
}

inline void FlagArray::operator&=(const FlagArrayTemporary &rhs)
{
    instance().opBitWiseAnd(rhs.instance(), true, true);
}

// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayImplScalar                                    - //
// -------------------------------------------------------------------------- //

inline FlagArrayImplScalar::FlagArrayImplScalar()
    :   FlagArrayImpl()
{
}

inline FlagArrayImplScalar::FlagArrayImplScalar(const FlagType &value)
    :   FlagArrayImpl(),
        itsData(value)
{
}

inline FlagArrayImpl *FlagArrayImplScalar::clone() const
{
    return new FlagArrayImplScalar(*this);
}

inline unsigned int FlagArrayImplScalar::rank() const
{
    return 0;
}

inline unsigned int FlagArrayImplScalar::shape(unsigned int) const
{
    ASSERT(false);
}

inline unsigned int FlagArrayImplScalar::size() const
{
    return 1;
}

inline FlagType FlagArrayImplScalar::value(unsigned int, unsigned int) const
{
    return itsData;
}

inline FlagArrayImplScalar::const_iterator FlagArrayImplScalar::begin() const
{
    return &itsData;
}

inline FlagArrayImplScalar::const_iterator FlagArrayImplScalar::end() const
{
    return (&itsData) + 1;
}

inline FlagArrayImplScalar::iterator FlagArrayImplScalar::begin()
{
    return &itsData;
}

inline FlagArrayImplScalar::iterator FlagArrayImplScalar::end()
{
    return (&itsData) + 1;
}

inline FlagArrayImpl *FlagArrayImplScalar::opBitWiseOr(const FlagArrayImpl &rhs,
    bool lhsTmp, bool rhsTmp) const
{
    return rhs.opBitWiseOr(*this, lhsTmp, rhsTmp);
}

inline
FlagArrayImpl *FlagArrayImplScalar::opBitWiseAnd(const FlagArrayImpl &rhs,
    bool lhsTmp, bool rhsTmp) const
{
    return rhs.opBitWiseAnd(*this, lhsTmp, rhsTmp);
}

inline const FlagType &FlagArrayImplScalar::value() const
{
    return itsData;
}

inline FlagType &FlagArrayImplScalar::value()
{
    return itsData;
}

// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayImplMatrix                                    - //
// -------------------------------------------------------------------------- //

inline FlagArrayImplMatrix::FlagArrayImplMatrix(unsigned int l0,
    unsigned int l1)
    :   FlagArrayImpl(),
        itsSize(l0 * l1),
        itsData(0)
{
    itsShape[0] = l0;
    itsShape[1] = l1;

    if(itsSize > 0)
    {
        itsData = new FlagType[itsSize];
    }
}

inline FlagArrayImplMatrix::FlagArrayImplMatrix(unsigned int l0,
    unsigned int l1, const FlagType &value)
    :   FlagArrayImpl(),
        itsSize(l0 * l1),
        itsData(0)
{
    itsShape[0] = l0;
    itsShape[1] = l1;

    if(itsSize > 0)
    {
        itsData = new FlagType[itsSize];
        fill(begin(), end(), value);
    }
}

inline FlagArrayImplMatrix::~FlagArrayImplMatrix()
{
    delete[] itsData;
}

inline FlagArrayImplMatrix::FlagArrayImplMatrix
    (const FlagArrayImplMatrix &other)
    :   FlagArrayImpl(other)
{
    itsSize = other.itsSize;
    copy(other.itsShape, other.itsShape + 2, itsShape);
    if(itsSize > 0)
    {
        itsData = new FlagType[itsSize];
        copy(other.itsData, other.itsData + other.itsSize, itsData);
    }
}

inline FlagArrayImpl *FlagArrayImplMatrix::clone() const
{
    return new FlagArrayImplMatrix(*this);
}

inline unsigned int FlagArrayImplMatrix::rank() const
{
    return 2;
}

inline unsigned int FlagArrayImplMatrix::shape(unsigned int n) const
{
    DBGASSERT(n < rank());
    return itsShape[n];
}

inline unsigned int FlagArrayImplMatrix::size() const
{
    return itsSize;
}

inline FlagType FlagArrayImplMatrix::value(unsigned int i0, unsigned int i1)
    const
{
    DBGASSERT(i0 < shape(0) && i1 < shape(1));
    return itsData[i0 + shape(0) * i1];
}

inline FlagArrayImplMatrix::const_iterator FlagArrayImplMatrix::begin() const
{
    return itsData;
}

inline FlagArrayImplMatrix::const_iterator FlagArrayImplMatrix::end() const
{
    return itsData + itsSize;
}

inline FlagArrayImplMatrix::iterator FlagArrayImplMatrix::begin()
{
    return itsData;
}

inline FlagArrayImplMatrix::iterator FlagArrayImplMatrix::end()
{
    return itsData + itsSize;
}

inline FlagArrayImpl *FlagArrayImplMatrix::opBitWiseOr(const FlagArrayImpl &rhs,
    bool lhsTmp, bool rhsTmp) const
{
    return rhs.opBitWiseOr(*this, lhsTmp, rhsTmp);
}

inline
FlagArrayImpl *FlagArrayImplMatrix::opBitWiseAnd(const FlagArrayImpl &rhs,
    bool lhsTmp, bool rhsTmp) const
{
    return rhs.opBitWiseAnd(*this, lhsTmp, rhsTmp);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
