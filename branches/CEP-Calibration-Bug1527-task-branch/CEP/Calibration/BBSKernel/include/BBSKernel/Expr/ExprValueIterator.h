//# ExprValueIterator.h: Iterate the bound values of an ExprValue instance.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRVALUEITERATOR_H
#define LOFAR_BBSKERNEL_EXPR_EXPRVALUEITERATOR_H

// \file
// Iterate the bound values of an ExprValue instance.

#include <BBSKernel/Expr/Element.h>
#include <BBSKernel/Expr/ExprValueView.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

template <typename T_EXPR_VALUE>
class ExprValueIterator;

template <>
class ExprValueIterator<Scalar>
{
public:
    ExprValueIterator(const Scalar &value);

    bool atEnd() const;

    const PValueKey &key() const;

    const ExprValueView<Scalar> value() const;
    const ExprValueView<Scalar> value(const PValueKey &key) const;

    void advance();
    void advance(const PValueKey &key);

private:
    PValueKey               itsKey;
    Element                 itsElement;
    Element::const_iterator itsIterator;
};

template <>
template <unsigned int LENGTH>
class ExprValueIterator<Vector<LENGTH> >
{
public:
    ExprValueIterator(const Vector<LENGTH> &value);

    bool atEnd() const;

    const PValueKey &key() const;

    const ExprValueView<Vector<LENGTH> > value() const;
    const ExprValueView<Vector<LENGTH> > value(const PValueKey &key) const;

    void advance();
    void advance(const PValueKey &key);

private:
    bool                    itsAtEnd;
    PValueKey               itsKey;
    Element                 itsElement[LENGTH];
    Element::const_iterator itsIterator[LENGTH];
};

template <>
class ExprValueIterator<JonesMatrix>
{
public:
    ExprValueIterator(const JonesMatrix &value);

    bool atEnd() const;

    const PValueKey &key() const;

    const ExprValueView<JonesMatrix> value() const;
    const ExprValueView<JonesMatrix> value(const PValueKey &key) const;

    void advance();
    void advance(const PValueKey &key);

private:
    bool                    itsAtEnd;
    PValueKey               itsKey;
    Element                 itsElement[4];
    Element::const_iterator itsIterator[4];
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueIterator<Scalar>                              - //
// -------------------------------------------------------------------------- //

inline bool ExprValueIterator<Scalar>::atEnd() const
{
    return itsIterator == itsElement.end();
}

inline const PValueKey &ExprValueIterator<Scalar>::key() const
{
    return itsKey;
}

inline const ExprValueView<Scalar> ExprValueIterator<Scalar>::value() const
{
    return value(key());
}

inline void ExprValueIterator<Scalar>::advance()
{
    advance(key());
}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueIterator<Vector<LENGTH> >                     - //
// -------------------------------------------------------------------------- //

template <unsigned int LENGTH>
ExprValueIterator<Vector<LENGTH> >::ExprValueIterator
    (const Vector<LENGTH> &value)
{
    itsAtEnd = true;
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        itsElement[i] = value.getElement(i);
        itsIterator[i] = itsElement[i].begin();

        if(itsIterator[i] != itsElement[i].end())
        {
            itsAtEnd = false;
            itsKey = std::min(itsKey, itsIterator[i]->first);
        }
    }
}

template <unsigned int LENGTH>
inline bool ExprValueIterator<Vector<LENGTH> >::atEnd() const
{
    return itsAtEnd;
}

template <unsigned int LENGTH>
inline const PValueKey &ExprValueIterator<Vector<LENGTH> >::key() const
{
    return itsKey;
}

template <unsigned int LENGTH>
const ExprValueView<Vector<LENGTH> >
ExprValueIterator<Vector<LENGTH> >::value(const PValueKey &key) const
{
    ExprValueView<Vector<LENGTH> > view;

    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        bool bound = itsIterator[i] != itsElement[i].end()
            && key == itsIterator[i]->first;
        view.assign(i,
            bound ? itsIterator[i]->second : itsElement[i].value(), bound);
    }

    return view;
}

template <unsigned int LENGTH>
inline const ExprValueView<Vector<LENGTH> >
ExprValueIterator<Vector<LENGTH> >::value() const
{
    return value(key());
}

template <unsigned int LENGTH>
void ExprValueIterator<Vector<LENGTH> >::advance(const PValueKey &key)
{
    if(!atEnd() && key == itsKey)
    {
        itsAtEnd = true;
        itsKey = PValueKey();
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            if(itsIterator[i] != itsElement[i].end()
                && key == itsIterator[i]->first)
            {
                ++itsIterator[i];
            }

            if(itsIterator[i] != itsElement[i].end())
            {
                itsAtEnd = false;
                itsKey = std::min(itsKey, itsIterator[i]->first);
            }
        }

        DBGASSERT(itsAtEnd != itsKey.valid());
    }
}

template <unsigned int LENGTH>
inline void ExprValueIterator<Vector<LENGTH> >::advance()
{
    advance(key());
}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueIterator<JonesMatrix>                         - //
// -------------------------------------------------------------------------- //

inline bool ExprValueIterator<JonesMatrix>::atEnd() const
{
    return itsAtEnd;
}

inline const PValueKey &ExprValueIterator<JonesMatrix>::key() const
{
    return itsKey;
}

inline const ExprValueView<JonesMatrix>
ExprValueIterator<JonesMatrix>::value() const
{
    return value(key());
}

inline void ExprValueIterator<JonesMatrix>::advance()
{
    advance(key());
}

} //# namespace BBS
} //# namespace LOFAR

#endif
