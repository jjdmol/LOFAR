//# ExprValueView.h: A view on an ExprValue instance of the unbound value or the
//# value bound to a specific (parameter, coefficient) pair.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRVALUEVIEW_H
#define LOFAR_BBSKERNEL_EXPR_EXPRVALUEVIEW_H

// \file
// A view on an ExprValue instance of the unbound value or the value bound to a
// specific (parameter, coefficient) pair.

#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>

#include <Common/lofar_algorithm.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Scalar;

template <unsigned int LENGTH>
class Vector;

class JonesMatrix;

template <typename T_EXPR_VALUE>
class ExprValueView;

template <>
class ExprValueView<Scalar>
{
public:
    ExprValueView();

    bool valid() const;
    bool bound() const;

    const Matrix operator()() const;
    void assign(const Matrix &value, bool bound = true);

private:
    Matrix  itsValue;
    bool    itsBindMask;
};

template <>
template <unsigned int LENGTH>
class ExprValueView<Vector<LENGTH> >
{
public:
    ExprValueView();

    bool valid(unsigned int i0) const;
    bool bound(unsigned int i0) const;

    const Matrix operator()(unsigned int i0) const;
    void assign(unsigned int i0, const Matrix &value, bool bound = true);

private:
    Matrix  itsValue[LENGTH];
    bool    itsBindMask[LENGTH];
};

template <>
class ExprValueView<JonesMatrix>
{
public:
    ExprValueView();

    bool valid(unsigned int i0, unsigned int i1) const;
    bool bound(unsigned int i0, unsigned int i1) const;

    const Matrix operator()(unsigned int i0, unsigned int i1) const;
    void assign(unsigned int i0, unsigned int i1, const Matrix &value,
        bool bound = true);

private:
    Matrix  itsValue[4];
    bool    itsBindMask[4];
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueView<Scalar>                                  - //
// -------------------------------------------------------------------------- //

inline bool ExprValueView<Scalar>::valid() const
{
    return !itsValue.isNull();
}

inline bool ExprValueView<Scalar>::bound() const
{
    return itsBindMask;
}

inline const Matrix ExprValueView<Scalar>::operator()() const
{
    return itsValue;
}

inline void ExprValueView<Scalar>::assign(const Matrix &value, bool bound)
{
    itsValue = value;
    itsBindMask = bound;
}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueView<Vector<LENGTH> >                         - //
// -------------------------------------------------------------------------- //

template <unsigned int LENGTH>
ExprValueView<Vector<LENGTH> >::ExprValueView()
{
    fill(itsBindMask, itsBindMask + LENGTH, false);
}

template <unsigned int LENGTH>
inline bool ExprValueView<Vector<LENGTH> >::valid(unsigned int i0) const
{
    return !itsValue[i0].isNull();
}

template <unsigned int LENGTH>
inline bool ExprValueView<Vector<LENGTH> >::bound(unsigned int i0) const
{
    return itsBindMask[i0];
}

template <unsigned int LENGTH>
inline const Matrix ExprValueView<Vector<LENGTH> >::operator()(unsigned int i0)
    const
{
    return itsValue[i0];
}

template <unsigned int LENGTH>
inline void ExprValueView<Vector<LENGTH> >::assign(unsigned int i0,
    const Matrix &value, bool bound)
{
    itsValue[i0] = value;
    itsBindMask[i0] = bound;
}

// -------------------------------------------------------------------------- //
// - Implementation: ExprValueView<JonesMatrix>                             - //
// -------------------------------------------------------------------------- //

inline bool ExprValueView<JonesMatrix>::valid(unsigned int i0, unsigned int i1)
    const
{
    return !itsValue[i0 * 2 + i1].isNull();
}

inline bool ExprValueView<JonesMatrix>::bound(unsigned int i0, unsigned int i1)
    const
{
    return itsBindMask[i0 * 2 + i1];
}

inline const Matrix ExprValueView<JonesMatrix>::operator()(unsigned int i0,
    unsigned int i1) const
{
    return itsValue[i0 * 2 + i1];
}

inline void ExprValueView<JonesMatrix>::assign(unsigned int i0, unsigned int i1,
    const Matrix &value, bool bound)
{
    itsValue[i0 * 2 + i1] = value;
    itsBindMask[i0 * 2 + i1] = bound;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
