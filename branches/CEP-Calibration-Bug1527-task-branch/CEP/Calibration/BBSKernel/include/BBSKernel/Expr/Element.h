//# Element.h: A scalar field that transparantly handles 0-D (constant) and 2-D
//# fields.
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

#ifndef LOFAR_BBSKERNEL_EXPR_VALUESET_H
#define LOFAR_BBSKERNEL_EXPR_VALUESET_H

// \file
// A scalar field that transparantly handles 0-D (constant) and 2-D fields.

#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/RefCounting.h>

#include <Common/lofar_map.h>
#include <Common/lofar_numeric.h>
#include <limits>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// A key that identifies a value bound to a specific (parameter id, coefficient
// id) pair.
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

// This class represents a scalar field that transparantly handles 0-D
// (constant) and 2-D fields (where the default axes are frequency and time).
// Multiple variants of the scalar field can be carried along, bound to
// different (parameter, coefficient) pairs. This can be used to hold e.g. the
// value of the scalar field perturbed for a (parameter, coefficient) pair, or
// the value of the partial derivative with respect to a (parameter,
// coefficient) pair.
// TODO: Use hashed container instead of map<> to optimize key look-up. However,
// need iteration in sorted order (?).
class ElementImpl: public RefCountable
{
public:
    typedef map<PValueKey, Matrix>::iterator        iterator;
    typedef map<PValueKey, Matrix>::const_iterator  const_iterator;

    size_t size() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    const Matrix value() const;
    const Matrix value(const PValueKey &key, bool &found) const;
    Matrix value();
    Matrix value(const PValueKey &key);

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);

private:
    Matrix                  itsData;
    map<PValueKey, Matrix>  itsDepData;
};

// Reference counting proxy class that manages a ElementImpl.
// TODO: return Matrix& or Matrix in value(*)?
class Element: public RefCounted<ElementImpl>
{
public:
    typedef ElementImpl::iterator          iterator;
    typedef ElementImpl::const_iterator    const_iterator;

    Element();

    // Return the number of scalar fields in the set. This number is at least
    // one, because there is always the unbound scalar field. Note that this
    // does not imply that the main scalar field (or any of the scalar fields)
    // actually contains data.
    size_t size() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    const Matrix value() const;
    // Returns the field bound to "key", or the unbound (main) field if no field
    // bound to "key" could be found.
    const Matrix value(const PValueKey &key) const;
    // Returns the field bound to "key", or the unbound (main) field if no field
    // bound to "key" could be found. The "found" argument indicates whether or not a
    // field bound to "key" was found.
    const Matrix value(const PValueKey &key, bool &found) const;

    Matrix value();
    // Returns a writeable reference to the field bound to "key". If no field
    // bound to "key" exists it will be created.
    Matrix value(const PValueKey &key);

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);
};

// @}

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
// - Implementation: ElementImpl                                           - //
// -------------------------------------------------------------------------- //

inline size_t ElementImpl::size() const
{
    return 1 + itsDepData.size();
}

inline ElementImpl::const_iterator ElementImpl::begin() const
{
    return itsDepData.begin();
}

inline ElementImpl::const_iterator ElementImpl::end() const
{
    return itsDepData.end();
}

inline ElementImpl::iterator ElementImpl::begin()
{
    return itsDepData.begin();
}

inline ElementImpl::iterator ElementImpl::end()
{
    return itsDepData.end();
}

inline const Matrix ElementImpl::value() const
{
    return itsData;
}

inline const Matrix ElementImpl::value(const PValueKey &key, bool &found)
    const
{
    map<PValueKey, Matrix>::const_iterator it = itsDepData.find(key);
    found = (it != itsDepData.end());
    return (found ? it->second : itsData);
}

inline Matrix ElementImpl::value()
{
    return itsData;
}

inline Matrix ElementImpl::value(const PValueKey &key)
{
    return itsDepData[key];
}

inline void ElementImpl::assign(const Matrix &value)
{
    itsData = value;
}

inline void ElementImpl::assign(const PValueKey &key, const Matrix &value)
{
    itsDepData[key] = value;
}


// -------------------------------------------------------------------------- //
// - Implementation: Element                                               - //
// -------------------------------------------------------------------------- //

inline size_t Element::size() const
{
    return instance().size();
}

inline Element::iterator Element::begin()
{
    return instance().begin();
}

inline Element::iterator Element::end()
{
    return instance().end();
}

inline Element::const_iterator Element::begin() const
{
    return instance().begin();
}

inline Element::const_iterator Element::end() const
{
    return instance().end();
}

inline const Matrix Element::value() const
{
    return instance().value();
}

inline const Matrix Element::value(const PValueKey &key) const
{
    bool tmp;
    return instance().value(key, tmp);
}

inline const Matrix Element::value(const PValueKey &key, bool &found) const
{
    return instance().value(key, found);
}

inline Matrix Element::value()
{
    return instance().value();
}

inline Matrix Element::value(const PValueKey &key)
{
    return instance().value(key);
}

inline void Element::assign(const Matrix &value)
{
    instance().assign(value);
}

inline void Element::assign(const PValueKey &key, const Matrix &value)
{
    return instance().assign(key, value);
}

} //# namespace BBS
} //# namespace LOFAR

#endif
