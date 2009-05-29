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


// This class represents a 2-D scalar field (where the default axes are
// frequency and time). Multiple variants of the scalar field can be carried
// along, linked to different (parameter, coefficient) pairs. This can be used
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
    iterator begin();
    iterator end();

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
class Proxy;

template <>
class Proxy<Scalar>
{
public:
    bool isDependent() const
    {
        return itsDepMask;
    }

    // TODO: Rename this to e.g. "value()"? (To avoid constructs like "arg0()").
    const Matrix &operator()() const
    {
        return itsData;
    }

    void assign(const Matrix &value, bool dependent = true)
    {
        itsDepMask = dependent;
        itsData = value;
    }

private:
    Matrix          itsData;
    bool            itsDepMask;
};

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
        return itsData[i0];
    }

    void assign(unsigned int i0, const Matrix &value, bool dependent = true)
    {
        itsDepMask[i0] = dependent;
        itsData[i0] = value;
    }

private:
    Matrix          itsData[LENGTH];
    bool            itsDepMask[LENGTH];
};

template <>
class Proxy<JonesMatrix>
{
public:
    bool isDependent(unsigned int i1, unsigned int i0) const
    {
        return itsDepMask[i1][i0];
    }

    const Matrix &operator()(unsigned int i1, unsigned int i0) const
    {
        return itsData[i1][i0];
    }

    void assign(unsigned int i1, unsigned int i0, const Matrix &value,
        bool dependent = true)
    {
        itsDepMask[i1][i0] = dependent;
        itsData[i1][i0] = value;
    }

private:
    Matrix          itsData[2][2];
    bool            itsDepMask[2][2];
};


// Base class that represents the result of an expression. The base class only
// holds the flags, the data is held by the specialisations (Scalar, Vector,
// JonesMatrix).
//
// TODO: Rename?
class ExprValueBase
{
public:
    virtual ~ExprValueBase()
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

    const FlagArray flags() const
    {
        return itsFlags;
    }

private:
    FlagArray   itsFlags;
};


class Scalar: public ExprValueBase
{
public:
    typedef Proxy<Scalar>   proxy;

    const proxy value(const PValueKey &key = PValueKey()) const
    {
        proxy result;

        if(!key.valid())
        {
            result.assign(itsFieldSet.value());
        }
        else
        {
            bool dependent = false;
            const Matrix &value = itsFieldSet.value(key, dependent);
            result.assign(value, dependent);
        }

        return result;
    }

    void assign(const proxy &value)
    {
        if(!value().isNull())
        {
            itsFieldSet.assign(value());
        }
    }

    void assign(const PValueKey &key, const proxy &value)
    {
        if(!value().isNull())
        {
            itsFieldSet.assign(key, value());
        }
    }

    const FieldSet getFieldSet() const
    {
        return itsFieldSet;
    }

    void setFieldSet(const FieldSet &set)
    {
        itsFieldSet = set;
    }

    // @{
    // Flat FieldSet indexing support.
    unsigned int size() const
    {
        return 1;
    }

    const FieldSet getFieldSet(unsigned int i) const
    {
        DBGASSERT(i == 0);
        return itsFieldSet;
    }

    void setFieldSet(unsigned int i, const FieldSet &set)
    {
        DBGASSERT(i == 0);
        itsFieldSet = set;
    }
    // @}

private:
    FieldSet    itsFieldSet;
};


// Default template parameters are more or less useless if the template has
// only one parameter, because in that case you still need to postfix the
// class name with "<>" when you want to instantiate with the default parameter
// value.
//
// In this case, instead of Vector<2> one would need to write Vector<>, which
// does not improve much.
template <unsigned int LENGTH>
class Vector: public ExprValueBase
{
public:
    typedef Proxy<Vector<LENGTH> >          proxy;

    const proxy value(const PValueKey &key = PValueKey()) const
    {
        proxy result;

        if(!key.valid())
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                result.assign(i, itsFieldSet[i].value());
            }
        }
        else
        {
            for(unsigned int i = 0; i < LENGTH; ++i)
            {
                bool dependent = false;
                const Matrix &value = itsFieldSet[i].value(key, dependent);
                result.assign(i, value, dependent);
            }
        }

        return result;
    }

    void assign(const proxy &value)
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            if(!value(i).isNull())
            {
                itsFieldSet[i].assign(value(i));
            }
        }
    }

    void assign(const PValueKey &key, const proxy &value)
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            if(!value(i).isNull())
            {
                itsFieldSet[i].assign(key, value(i));
            }
        }
    }

    // @{
    // Flat FieldSet indexing support.
    unsigned int size() const
    {
        return LENGTH;
    }

    const FieldSet getFieldSet(unsigned int i) const
    {
        DBGASSERT(i < LENGTH);
        return itsFieldSet[i];
    }

    void setFieldSet(unsigned int i, const FieldSet &set)
    {
        DBGASSERT(i < LENGTH);
        itsFieldSet[i] = set;
    }
    // @}

private:
    FieldSet    itsFieldSet[LENGTH];
};


class JonesMatrix: public ExprValueBase
{
public:
    typedef Proxy<JonesMatrix>  proxy;

    const proxy value(const PValueKey &key = PValueKey()) const
    {
        proxy result;

        if(!key.valid())
        {
            result.assign(0, 0, itsFieldSet[0][0].value());
            result.assign(0, 1, itsFieldSet[0][1].value());
            result.assign(1, 0, itsFieldSet[1][0].value());
            result.assign(1, 1, itsFieldSet[1][1].value());
        }
        else
        {
            bool dependent = false;
            const Matrix &tmp00 = itsFieldSet[0][0].value(key, dependent);
            result.assign(0, 0, tmp00, dependent);
            const Matrix &tmp01 = itsFieldSet[0][1].value(key, dependent);
            result.assign(0, 1, tmp01, dependent);
            const Matrix &tmp10 = itsFieldSet[1][0].value(key, dependent);
            result.assign(1, 0, tmp10, dependent);
            const Matrix &tmp11 = itsFieldSet[1][1].value(key, dependent);
            result.assign(1, 1, tmp11, dependent);
        }

        return result;
    }

    void assign(const proxy &value)
    {
        if(!value(0, 0).isNull())
            itsFieldSet[0][0].assign(value(0, 0));
        if(!value(0, 1).isNull())
            itsFieldSet[0][1].assign(value(0, 1));
        if(!value(1, 0).isNull())
            itsFieldSet[1][0].assign(value(1, 0));
        if(!value(1, 1).isNull())
            itsFieldSet[1][1].assign(value(1, 1));
    }

    void assign(const PValueKey &key, const proxy &value)
    {
        if(!value(0, 0).isNull())
            itsFieldSet[0][0].assign(key, value(0, 0));
        if(!value(0, 1).isNull())
            itsFieldSet[0][1].assign(key, value(0, 1));
        if(!value(1, 0).isNull())
            itsFieldSet[1][0].assign(key, value(1, 0));
        if(!value(1, 1).isNull())
            itsFieldSet[1][1].assign(key, value(1, 1));
    }

    const FieldSet getFieldSet(unsigned int i1, unsigned int i0) const
    {
        DBGASSERT(i1 < 2 && i0 < 2);
        return itsFieldSet[i1][i0];
    }

    void setFieldSet(unsigned int i1, unsigned int i0, const FieldSet &set)
    {
        DBGASSERT(i1 < 2 && i0 < 2);
        itsFieldSet[i1][i0] = set;
    }

    // Flat FieldSet indexing support.
    // @{
    unsigned int size() const
    {
        return 4;
    }

    const FieldSet getFieldSet(unsigned int i) const
    {
        DBGASSERT(i < 4);
        return itsFieldSet[i / 2][i % 2];
    }

    void setFieldSet(unsigned int i, const FieldSet &set)
    {
        DBGASSERT(i < 4);
        itsFieldSet[i / 2][i % 2] = set;
    }
    // @}

private:
    FieldSet    itsFieldSet[2][2];
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

inline FieldSet::iterator FieldSet::begin()
{
    return instance().begin();
}

inline FieldSet::iterator FieldSet::end()
{
    return instance().end();
}

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

inline Matrix &FieldSet::value()
{
    return instance().value();
}

inline Matrix &FieldSet::value(const PValueKey &key)
{
    return instance().value(key);
}

inline void FieldSet::assign(const Matrix &value)
{
    instance().assign(value);
}

inline void FieldSet::assign(const PValueKey &key, const Matrix &value)
{
    return instance().assign(key, value);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
