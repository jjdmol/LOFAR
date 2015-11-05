//# ExprValue.h: The result of an expression.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRVALUE_H
#define LOFAR_BBSKERNEL_EXPR_EXPRVALUE_H

// \file
// The result of an expression.

#include <Common/LofarLogger.h>

#include <BBSKernel/Expr/ExprValueView.h>
#include <BBSKernel/Expr/ExprValueIterator.h>
#include <BBSKernel/Expr/FlagArray.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// Abstract base class that represents the result of an expression. The base
// class only holds the flags, the data is held by derived classes (Scalar,
// Vector, JonesMatrix).
class ExprValue
{
public:
    virtual ~ExprValue();

    bool hasFlags() const;
    void setFlags(const FlagArray &flags);
    const FlagArray flags() const;

    virtual unsigned int size() const = 0;
    virtual const Element element(unsigned int i0) const = 0;
    virtual void setElement(unsigned int i0, const Element &element) = 0;

private:
    FlagArray   itsFlags;
};

class Scalar: public ExprValue
{
public:
    typedef ExprValueView<Scalar>       View;
    typedef ExprValueIterator<Scalar>   Iterator;

    Scalar();
    Scalar(const Matrix &value);
    Scalar(const Element &element);

    const Matrix value() const;
    const Matrix value(const PValueKey &key) const;
    const View view() const;
    const View view(const PValueKey &key) const;

    void assign(const Matrix &value);
    void assign(const PValueKey &key, const Matrix &value);
    void assign(const View &value);
    void assign(const PValueKey &key, const View &value);

    const Element element() const;
    void setElement(const Element &element);

    // @{
    // Support for flat Element indexing.
    unsigned int size() const;
    const Element element(unsigned int i0) const;
    void setElement(unsigned int i0, const Element &element);
    // @}

private:
    Element itsElement;
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
    typedef ExprValueView<Vector<LENGTH> >      View;
    typedef ExprValueIterator<Vector<LENGTH> >  Iterator;

    const Matrix value(unsigned int i0) const;
    const Matrix value(unsigned int i0, const PValueKey &key) const;
    const View view() const;
    const View view(const PValueKey &key) const;

    void assign(const View &value);
    void assign(const PValueKey &key, const View &value);
    void assign(unsigned int i0, const Matrix &value);
    void assign(unsigned int i0, const PValueKey &key, const Matrix &value);

    // @{
    // Support for flat Element indexing.
    unsigned int size() const;
    const Element element(unsigned int i0) const;
    void setElement(unsigned int i0, const Element &element);
    // @}

private:
    Element itsElement[LENGTH];
};

class JonesMatrix: public ExprValue
{
public:
    typedef ExprValueView<JonesMatrix>      View;
    typedef ExprValueIterator<JonesMatrix>  Iterator;

    JonesMatrix();
    JonesMatrix(const Matrix &el00, const Matrix &el01, const Matrix &el10,
        const Matrix &el11);
    JonesMatrix(const Element &el00, const Element &el01, const Element &el10,
        const Element &el11);

    const Matrix value(unsigned int i0, unsigned int i1) const;
    const Matrix value(unsigned int i0, unsigned int i1, const PValueKey &key)
        const;
    const View view() const;
    const View view(const PValueKey &key) const;

    void assign(unsigned int i0, unsigned int i1, const Matrix &value);
    void assign(unsigned int i0, unsigned int i1, const PValueKey &key,
        const Matrix &value);
    void assign(const View &value);
    void assign(const PValueKey &key, const View &value);

    const Element element(unsigned int i0, unsigned int i1) const;
    void setElement(unsigned int i0, unsigned int i1, const Element &element);

    // Support for flat Element indexing.
    // @{
    unsigned int size() const;
    const Element element(unsigned int i0) const;
    void setElement(unsigned int i0, const Element &element);
    // @}

private:
    Element itsElement[4];
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValue                                              - //
// -------------------------------------------------------------------------- //

inline bool ExprValue::hasFlags() const
{
    return itsFlags.initialized();
}

inline void ExprValue::setFlags(const FlagArray &flags)
{
    itsFlags = flags;
}

inline const FlagArray ExprValue::flags() const
{
    return itsFlags;
}

// -------------------------------------------------------------------------- //
// - Implementation: Scalar                                                 - //
// -------------------------------------------------------------------------- //

inline const Matrix Scalar::value() const
{
    return itsElement.value();
}

inline const Matrix Scalar::value(const PValueKey &key) const
{
    return itsElement.value(key);
}

inline void Scalar::assign(const Matrix &value)
{
    itsElement.assign(value);
}

inline void Scalar::assign(const PValueKey &key, const Matrix &value)
{
    itsElement.assign(key, value);
}

inline const Element Scalar::element() const
{
    return itsElement;
}

inline void Scalar::setElement(const Element &element)
{
    itsElement = element;
}

inline unsigned int Scalar::size() const
{
    return 1;
}

#if defined(DEBUG) || defined(LOFAR_DEBUG)
inline const Element Scalar::element(unsigned int i0) const
#else
inline const Element Scalar::element(unsigned int) const
#endif
{
    DBGASSERT(i0 == 0);
    return itsElement;
}

#if defined(DEBUG) || defined(LOFAR_DEBUG)
inline void Scalar::setElement(unsigned int i0, const Element &element)
#else
inline void Scalar::setElement(unsigned int, const Element &element)
#endif
{
    DBGASSERT(i0 == 0);
    itsElement = element;
}

// -------------------------------------------------------------------------- //
// - Implementation: Vector<LENGTH>                                         - //
// -------------------------------------------------------------------------- //

template <unsigned int LENGTH>
inline const Matrix Vector<LENGTH>::value(unsigned int i0) const
{
    DBGASSERT(i0 < LENGTH);
    return itsElement[i0].value();
}

template <unsigned int LENGTH>
inline const Matrix Vector<LENGTH>::value(unsigned int i0, const PValueKey &key)
    const
{
    DBGASSERT(i0 < LENGTH);
    return itsElement[i0].value(key);
}

template <unsigned int LENGTH>
const typename Vector<LENGTH>::View Vector<LENGTH>::view() const
{
    View view;
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        view.assign(i, itsElement[i].value());
    }

    return view;
}

template <unsigned int LENGTH>
const typename Vector<LENGTH>::View Vector<LENGTH>::view(const PValueKey &key)
    const
{
    DBGASSERT(key.valid());

    View view;
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        bool found;
        const Matrix tmp = itsElement[i].value(key, found);
        view.assign(i, tmp, found);
    }

    return view;
}

template <unsigned int LENGTH>
inline void Vector<LENGTH>::assign(unsigned int i0, const Matrix &value)
{
    DBGASSERT(i0 < LENGTH);
    itsElement[i0].assign(value);
}

template <unsigned int LENGTH>
inline void Vector<LENGTH>::assign(unsigned int i0, const PValueKey &key,
    const Matrix &value)
{
    DBGASSERT(i0 < LENGTH);
    itsElement[i0].assign(key, value);
}

template <unsigned int LENGTH>
void Vector<LENGTH>::assign(const View &value)
{
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        if(value.bound(i))
        {
            itsElement[i].assign(value(i));
        }
    }
}

template <unsigned int LENGTH>
void Vector<LENGTH>::assign(const PValueKey &key, const View &value)
{
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        if(value.bound(i))
        {
            itsElement[i].assign(key, value(i));
        }
    }
}

template <unsigned int LENGTH>
inline unsigned int Vector<LENGTH>::size() const
{
    return LENGTH;
}

template <unsigned int LENGTH>
inline const Element Vector<LENGTH>::element(unsigned int i0) const
{
    DBGASSERT(i0 < LENGTH);
    return itsElement[i0];
}

template <unsigned int LENGTH>
inline void Vector<LENGTH>::setElement(unsigned int i0, const Element &element)
{
    DBGASSERT(i0 < LENGTH);
    itsElement[i0] = element;
}

// -------------------------------------------------------------------------- //
// - Implementation: JonesMatrix                                            - //
// -------------------------------------------------------------------------- //

inline const Matrix JonesMatrix::value(unsigned int i0, unsigned int i1) const
{
    DBGASSERT(i0 < 2 && i1 < 2);
    return itsElement[i0 * 2 + i1].value();
}

inline const Matrix JonesMatrix::value(unsigned int i0, unsigned int i1,
    const PValueKey &key) const
{
    DBGASSERT(i0 < 2 && i1 < 2);
    return itsElement[i0 * 2 + i1].value(key);
}

inline void JonesMatrix::assign(unsigned int i0, unsigned int i1,
    const Matrix &value)
{
    DBGASSERT(i0 < 2 && i1 < 2);
    itsElement[i0 * 2 + i1].assign(value);
}

inline void JonesMatrix::assign(unsigned int i0, unsigned int i1,
    const PValueKey &key, const Matrix &value)
{
    DBGASSERT(i0 < 2 && i1 < 2);
    itsElement[i0 * 2 + i1].assign(key, value);
}

inline const Element JonesMatrix::element(unsigned int i0, unsigned int i1)
    const
{
    DBGASSERT(i0 < 2 && i1 < 2);
    return itsElement[i0 * 2 + i1];
}

inline void JonesMatrix::setElement(unsigned int i0, unsigned int i1,
    const Element &element)
{
    DBGASSERT(i0 < 2 && i1 < 2);
    itsElement[i0 * 2 + i1] = element;
}

inline unsigned int JonesMatrix::size() const
{
    return 4;
}

inline const Element JonesMatrix::element(unsigned int i0) const
{
    DBGASSERT(i0 < 4);
    return itsElement[i0];
}

inline void JonesMatrix::setElement(unsigned int i0, const Element &element)
{
    DBGASSERT(i0 < 4);
    itsElement[i0] = element;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
