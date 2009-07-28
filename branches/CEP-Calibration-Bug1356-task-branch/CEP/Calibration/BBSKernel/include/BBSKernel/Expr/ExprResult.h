//# ExprResult.h: The result of an expression.
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
// The result of an expression.

#include <Common/lofar_map.h>
#include <Common/lofar_numeric.h>

#include <Common/LofarLogger.h>

#include <BBSKernel/Expr/FlagArray.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
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


// This class represents a 2-D scalar field (where the default axes are
// frequency and time). Multiple variants of the scalar field can be carried
// along, linked to different (parameter, coefficient) pairs. This can be used
// to hold e.g. the value of the scalar field perturbed for a (parameter,
// coefficient) pair, or the value of the partial derivative with respect to a
// (parameter, coefficient) pair.
// TODO: Use hashed container instead of map<> to optimize key look-up. However,
// need iteration in sorted order (?).
// TODO: Rename to ValueSetImpl?
class ValueSetImpl: public RefCountable
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


// Reference counting proxy class that manages a ValueSetImpl.
class ValueSet: public RefCounted<ValueSetImpl>
{
public:
    typedef ValueSetImpl::iterator          iterator;
    typedef ValueSetImpl::const_iterator    const_iterator;

    ValueSet();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    const Matrix &value() const;
    // Returns the field associated with "key", or the unkeyed (main) field if
    // not found.
    const Matrix &value(const PValueKey &key) const;
    // Returns the field associated with "key", or the unkeyed (main) field if
    // not found. The "found" argument is indicates if a field associated with
    // "key" was found.
    const Matrix &value(const PValueKey &key, bool &found) const;

    Matrix &value();
    // Returns a writeable reference to the field associated with "key". If no
    // such field exists yet it will be created.
    Matrix &value(const PValueKey &key);

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);
};


class Scalar;

template <unsigned int LENGTH>
class Vector;

class JonesMatrix;


template <typename T>
class ExprValueView;

template <>
class ExprValueView<Scalar>
{
public:
    ExprValueView()
        :   itsDirtyFlag(false)
    {
    }

    // TODO: Rename this to e.g. "value()"? (To avoid constructs like "arg0()").
    const Matrix &operator()() const
    {
        return itsValue;
    }

//    operator const Matrix &() const
//    {
//        return itsValue;
//    }

//    operator Matrix &()
//    {
//        return itsValue;
//    }

    bool valid() const
    {
        return !itsValue.isNull();
    }

    bool dirty() const
    {
        return itsDirtyFlag;
    }

    void assign(const Matrix &value, bool dirty = true)
    {
        itsValue = value;
        itsDirtyFlag = dirty;
    }

//    void setDirtyFlag(bool dirty)
//    {
//        itsDirtyFlag = dirty;
//    }

private:
    Matrix          itsValue;
    bool            itsDirtyFlag;
};

template <>
template <unsigned int LENGTH>
class ExprValueView<Vector<LENGTH> >
{
public:
    ExprValueView()
    {
        fill(itsDirtyFlag, itsDirtyFlag + LENGTH, false);
    }

    const Matrix &operator()(unsigned int i0) const
    {
        return itsValue[i0];
    }

//    Matrix &operator()(unsigned int i0)
//    {
//        return itsValue[i0];
//    }

    bool valid(unsigned int i0) const
    {
        return !itsValue[i0].isNull();
    }

    bool dirty(unsigned int i0) const
    {
        return itsDirtyFlag[i0];
    }

    void assign(unsigned int i0, const Matrix &value, bool dirty = true)
    {
        itsValue[i0] = value;
        itsDirtyFlag[i0] = dirty;
    }

//    void setDirtyFlag(bool dirty)
//    {
//        itsDirtyFlag = dirty;
//    }

private:
    Matrix          itsValue[LENGTH];
    bool            itsDirtyFlag[LENGTH];
};

template <>
class ExprValueView<JonesMatrix>
{
public:
    ExprValueView()
    {
        fill(itsDirtyFlag, itsDirtyFlag + 4, false);
    }
//    bool isDependent(unsigned int i0, unsigned int i1) const
//    {
//        return itsDepMask[i0][i1];
//    }

    const Matrix &operator()(unsigned int i0, unsigned int i1) const
    {
        return itsValue[i0 * 2 + i1];
    }

//    Matrix &operator()(unsigned int i0, unsigned int i1)
//    {
//        return itsValue[i0][i1];
//    }

    bool valid(unsigned int i0, unsigned int i1) const
    {
        return !itsValue[i0 * 2 + i1].isNull();
    }

    void assign(unsigned int i0, unsigned int i1, const Matrix &value,
        bool dirty = true)
    {
        itsValue[i0 * 2 + i1] = value;
        itsDirtyFlag[i0 * 2 + i1] = dirty;
    }

    bool dirty(unsigned int i0, unsigned int i1) const
    {
        return itsDirtyFlag[i0 * 2 + i1];
    }

//    void setDirtyFlag(unsigned int i0, unsigned int i1, bool dirty)
//    {
//        itsDirtyFlag[i0][i1] = dirty;
//    }

private:
    Matrix          itsValue[4];
    bool            itsDirtyFlag[4];
};


// Abstract base class that represents the result of an expression. The base
// class only holds the flags, the data is held by derived classes (Scalar,
// Vector, JonesMatrix).
//
// TODO: Rename to ExprValue/ExprResult/Result?
class ExprValue
{
public:
    virtual ~ExprValue()
    {
    }

    bool hasFlags() const
    {
        return itsFlags.initialized();
    }

    void setFlags(const FlagArray &flags)
    {
        itsFlags = flags;
    }

    const FlagArray &flags() const
    {
        return itsFlags;
    }

    virtual unsigned int size() const = 0;
    virtual const ValueSet getValueSet(unsigned int i0) const = 0;
    virtual void setValueSet(unsigned int i0, const ValueSet &set) = 0;

private:
    FlagArray   itsFlags;
};


class Scalar: public ExprValue
{
public:
    typedef ExprValueView<Scalar>   view;

    view value(const PValueKey &key = PValueKey()) const
    {
        view result;

        if(!key.valid())
        {
            result.assign(itsValueSet.value());
        }
        else
        {
            bool found;
            const Matrix &value = itsValueSet.value(key, found);
            result.assign(value, found);
        }

        return result;
    }

    void assign(const view &value)
    {
        if(value.dirty())
        {
            itsValueSet.assign(value());
        }
    }

    void assign(const PValueKey &key, const view &value)
    {
        if(value.dirty())
        {
            itsValueSet.assign(key, value());
        }
    }

    void assign(const Matrix &value)
    {
        itsValueSet.assign(value);
    }

    void assign(const PValueKey &key, const Matrix &value)
    {
        itsValueSet.assign(key, value);
    }

    const ValueSet getValueSet() const
    {
        return itsValueSet;
    }

    void setValueSet(const ValueSet &set)
    {
        itsValueSet = set;
    }

    // @{
    // Flat ValueSet indexing support.
    unsigned int size() const
    {
        return 1;
    }

    const ValueSet getValueSet(unsigned int i0) const
    {
        DBGASSERT(i0 == 0);
        return itsValueSet;
    }

    void setValueSet(unsigned int i0, const ValueSet &set)
    {
        DBGASSERT(i0 == 0);
        itsValueSet = set;
    }
    // @}

private:
    ValueSet    itsValueSet;
};


// Default template parameters are more or less useless if the template has
// only one parameter, because in that case you still need to postfix the
// class name with "<>" when you want to instantiate with the default parameter
// value.
//
// In this case, instead of Vector<2> one would need to write Vector<>, which
// does not improve much.
template <unsigned int LENGTH>
class Vector: public ExprValue
{
public:
    typedef ExprValueView<Vector<LENGTH> >          view;

    view value(const PValueKey &key = PValueKey()) const
    {
        view result;

        if(!key.valid())
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                result.assign(i, itsValueSet[i].value());
            }
        }
        else
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                bool found;
                const Matrix &value = itsValueSet[i].value(key, found);
                result.assign(i, value, found);
            }
        }

        return result;
    }

    void assign(const view &value)
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            if(value.dirty(i))
            {
                itsValueSet[i].assign(value(i));
            }
        }
    }

    void assign(const PValueKey &key, const view &value)
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            if(value.dirty(i))
            {
                itsValueSet[i].assign(key, value(i));
            }
        }
    }

    void assign(unsigned int i0, const Matrix &value)
    {
        DBGASSERT(i0 < LENGTH);
        itsValueSet[i0].assign(value);
    }

    void assign(unsigned int i0, const PValueKey &key, const Matrix &value)
    {
        DBGASSERT(i0 < LENGTH);
        itsValueSet[i0].assign(key, value);
    }

    // @{
    // Flat ValueSet indexing support.
    unsigned int size() const
    {
        return LENGTH;
    }

    const ValueSet getValueSet(unsigned int i0) const
    {
        DBGASSERT(i0 < LENGTH);
        return itsValueSet[i0];
    }

    void setValueSet(unsigned int i0, const ValueSet &set)
    {
        DBGASSERT(i0 < LENGTH);
        itsValueSet[i0] = set;
    }
    // @}

private:
    ValueSet    itsValueSet[LENGTH];
};


class JonesMatrix: public ExprValue
{
public:
    typedef ExprValueView<JonesMatrix>  view;

    view value(const PValueKey &key = PValueKey()) const
    {
        view result;

        if(!key.valid())
        {
            result.assign(0, 0, itsValueSet[0].value());
            result.assign(0, 1, itsValueSet[1].value());
            result.assign(1, 0, itsValueSet[2].value());
            result.assign(1, 1, itsValueSet[3].value());
        }
        else
        {
            bool found;
            const Matrix &tmp00 = itsValueSet[0].value(key, found);
            result.assign(0, 0, tmp00, found);
            const Matrix &tmp01 = itsValueSet[1].value(key, found);
            result.assign(0, 1, tmp01, found);
            const Matrix &tmp10 = itsValueSet[2].value(key, found);
            result.assign(1, 0, tmp10, found);
            const Matrix &tmp11 = itsValueSet[3].value(key, found);
            result.assign(1, 1, tmp11, found);
        }

        return result;
    }

    void assign(const view &value)
    {
        if(value.dirty(0, 0))
            itsValueSet[0].assign(value(0, 0));
        if(value.dirty(0, 1))
            itsValueSet[1].assign(value(0, 1));
        if(value.dirty(1, 0))
            itsValueSet[2].assign(value(1, 0));
        if(value.dirty(1, 1))
            itsValueSet[3].assign(value(1, 1));
    }

    void assign(const PValueKey &key, const view &value)
    {
        if(value.dirty(0, 0))
            itsValueSet[0].assign(key, value(0, 0));
        if(value.dirty(0, 1))
            itsValueSet[1].assign(key, value(0, 1));
        if(value.dirty(1, 0))
            itsValueSet[2].assign(key, value(1, 0));
        if(value.dirty(1, 1))
            itsValueSet[3].assign(key, value(1, 1));
    }

    void assign(unsigned int i0, unsigned int i1, const Matrix &value)
    {
        DBGASSERT(i0 < 2 && i1 < 2);
        itsValueSet[i0 * 2 + i1].assign(value);
    }

    void assign(unsigned int i0, unsigned int i1, const PValueKey &key,
        const Matrix &value)
    {
        DBGASSERT(i0 < 2 && i1 < 2);
        itsValueSet[i0 * 2 + i1].assign(key, value);
    }

    const ValueSet getValueSet(unsigned int i0, unsigned int i1) const
    {
        DBGASSERT(i0 < 2 && i1 < 2);
        return itsValueSet[i0 * 2 + i1];
    }

    void setValueSet(unsigned int i0, unsigned int i1, const ValueSet &set)
    {
        DBGASSERT(i0 < 2 && i1 < 2);
        itsValueSet[i0 * 2 + i1] = set;
    }

    // Flat ValueSet indexing support.
    // @{
    unsigned int size() const
    {
        return 4;
    }

    const ValueSet getValueSet(unsigned int i0) const
    {
        DBGASSERT(i0 < 4);
//        return itsValueSet[i / 2][i % 2];
        return itsValueSet[i0];
    }

    void setValueSet(unsigned int i0, const ValueSet &set)
    {
        DBGASSERT(i0 < 4);
//        itsValueSet[i / 2][i % 2] = set;
        itsValueSet[i0] = set;
    }
    // @}

private:
    ValueSet    itsValueSet[4];
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
// - Implementation: ValueSetImpl                                           - //
// -------------------------------------------------------------------------- //

inline ValueSetImpl::const_iterator ValueSetImpl::begin() const
{
    return itsDepData.begin();
}

inline ValueSetImpl::const_iterator ValueSetImpl::end() const
{
    return itsDepData.end();
}

inline ValueSetImpl::iterator ValueSetImpl::begin()
{
    return itsDepData.begin();
}

inline ValueSetImpl::iterator ValueSetImpl::end()
{
    return itsDepData.end();
}

inline const Matrix &ValueSetImpl::value() const
{
    return itsData;
}

inline const Matrix &ValueSetImpl::value(const PValueKey &key, bool &found)
    const
{
    map<PValueKey, Matrix>::const_iterator it = itsDepData.find(key);
    found = (it != itsDepData.end());
    return (found ? it->second : itsData);
}

inline Matrix &ValueSetImpl::value()
{
    return itsData;
}

inline Matrix &ValueSetImpl::value(const PValueKey &key)
{
    return itsDepData[key];
}

inline void ValueSetImpl::assign(const Matrix &value)
{
    itsData = value;
}

inline void ValueSetImpl::assign(const PValueKey &key, const Matrix &value)
{
    itsDepData[key] = value;
}


// -------------------------------------------------------------------------- //
// - Implementation: ValueSet                                               - //
// -------------------------------------------------------------------------- //

inline ValueSet::ValueSet()
    :   RefCounted<ValueSetImpl>(new ValueSetImpl())
{
}

inline ValueSet::iterator ValueSet::begin()
{
    return instance().begin();
}

inline ValueSet::iterator ValueSet::end()
{
    return instance().end();
}

inline ValueSet::const_iterator ValueSet::begin() const
{
    return instance().begin();
}

inline ValueSet::const_iterator ValueSet::end() const
{
    return instance().end();
}

inline const Matrix &ValueSet::value() const
{
    return instance().value();
}

inline const Matrix &ValueSet::value(const PValueKey &key) const
{
    bool tmp;
    return instance().value(key, tmp);
}

inline const Matrix &ValueSet::value(const PValueKey &key, bool &found) const
{
    return instance().value(key, found);
}

inline Matrix &ValueSet::value()
{
    return instance().value();
}

inline Matrix &ValueSet::value(const PValueKey &key)
{
    return instance().value(key);
}

inline void ValueSet::assign(const Matrix &value)
{
    instance().assign(value);
}

inline void ValueSet::assign(const PValueKey &key, const Matrix &value)
{
    return instance().assign(key, value);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
