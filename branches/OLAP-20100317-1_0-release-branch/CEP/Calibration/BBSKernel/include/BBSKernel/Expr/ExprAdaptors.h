//# ExprAdaptors.h: Helper classes to convert between different Expr types.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRADAPTORS_H
#define LOFAR_BBSKERNEL_EXPR_EXPRADAPTORS_H

// \file
// Helper classes to convert between different Expr types.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

template <typename T_EXPR_VALUE>
class AsExpr;

// Adaptor class to bundle multiple Expr<Scalar> into a single Expr<Vector<N> >.
template <>
template <unsigned int LENGTH>
class AsExpr<Vector<LENGTH> >: public Expr<Vector<LENGTH> >
{
public:
    typedef shared_ptr<AsExpr>          Ptr;
    typedef shared_ptr<const AsExpr>    ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    AsExpr();

    template <typename T_ITER>
    AsExpr(T_ITER first, T_ITER last);

    ~AsExpr();

    void connect(unsigned int i0, const typename Expr<Scalar>::ConstPtr &arg);

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Vector<LENGTH> evaluateExpr(const Request &request,
        Cache &cache, unsigned int grid) const;

private:
    typename Expr<Scalar>::ConstPtr itsArg[LENGTH];
};

// Adaptor class to bundle multiple Expr<Scalar> into a single
// Expr<JonesMatrix>.
template <>
class AsExpr<JonesMatrix>: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<AsExpr>          Ptr;
    typedef shared_ptr<const AsExpr>    ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    AsExpr();
    AsExpr(const Expr<Scalar>::ConstPtr &element00,
        const Expr<Scalar>::ConstPtr &element01,
        const Expr<Scalar>::ConstPtr &element10,
        const Expr<Scalar>::ConstPtr &element11);

    ~AsExpr();

    void connect(unsigned int i1, unsigned int i0,
        const Expr<Scalar>::ConstPtr &arg);

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    Expr<Scalar>::ConstPtr  itsArg[4];
};

// Adaptor class to bundle two Expr<Scalar> into a single diagonal
// Expr<JonesMatrix>.
class AsDiagonalMatrix: public BinaryExpr<Scalar, Scalar, JonesMatrix>
{
public:
    typedef shared_ptr<AsDiagonalMatrix>        Ptr;
    typedef shared_ptr<const AsDiagonalMatrix>  ConstPtr;

    AsDiagonalMatrix(const Expr<Scalar>::ConstPtr &element00,
        const Expr<Scalar>::ConstPtr &element11);

protected:
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;
};

// Adaptor class to bundle two real Expr<Scalar> into a single complex
// Expr<Scalar>, where the two input Expr represent the real and imaginary part
// respectively.
class AsComplex: public BasicBinaryExpr<Scalar, Scalar, Scalar>
{
public:
    typedef shared_ptr<AsComplex>       Ptr;
    typedef shared_ptr<const AsComplex> ConstPtr;

    AsComplex(const Expr<Scalar>::ConstPtr &re,
        const Expr<Scalar>::ConstPtr &im);

protected:
    virtual const Scalar::View evaluateImpl(const Grid&,
        const Scalar::View &re, const Scalar::View &im) const;
};


// Adaptor class to bundle two real Expr<Scalar> into a single complex
// Expr<Scalar>, where the two input Expr represent the complex modulus
// (amplitude) and the complex argument (phase) respectively.
class AsPolar: public BasicBinaryExpr<Scalar, Scalar, Scalar>
{
public:
    typedef shared_ptr<AsPolar>         Ptr;
    typedef shared_ptr<const AsPolar>   ConstPtr;

    AsPolar(const Expr<Scalar>::ConstPtr &modulus,
        const Expr<Scalar>::ConstPtr &argument);

protected:
    virtual const Scalar::View evaluateImpl(const Grid&,
        const Scalar::View &mod, const Scalar::View &arg) const;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: AsExpr<Vector<N> >                                     - //
// -------------------------------------------------------------------------- //

template <unsigned int LENGTH>
AsExpr<Vector<LENGTH> >::AsExpr()
{
}

template <unsigned int LENGTH>
template <typename T_ITER>
AsExpr<Vector<LENGTH> >::AsExpr(T_ITER first, T_ITER last)
{
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        ASSERT(first != last);
        itsArg[i] = *first++;
        connect(itsArg[i]);
    }
    ASSERT(first == last);
}

template <unsigned int LENGTH>
AsExpr<Vector<LENGTH> >::~AsExpr()
{
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        disconnect(itsArg[i]);
    }
}

template <unsigned int LENGTH>
void AsExpr<Vector<LENGTH> >::connect(unsigned int i0,
    const typename Expr<Scalar>::ConstPtr &arg)
{
    connect(arg);
    itsArg[i0] = arg;
}

template <unsigned int LENGTH>
const Vector<LENGTH> AsExpr<Vector<LENGTH> >::evaluateExpr
    (const Request &request, Cache &cache, unsigned int grid) const
{
    // Allocate result.
    Vector<LENGTH> result;

    // Evaluate arguments (pass through).
    Scalar args[LENGTH];
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        args[i] = itsArg[i]->evaluate(request, cache, grid);
        result.setValueSet(i, args[i].getValueSet());
    }

    // Evaluate flags.
    FlagArray flags[LENGTH];
    for(unsigned int i = 0; i < LENGTH; ++i)
    {
        flags[i] = args[i].flags();
    }
    result.setFlags(mergeFlags(flags, flags + LENGTH));

    return result;
}

template <unsigned int LENGTH>
unsigned int AsExpr<Vector<LENGTH> >::nArguments() const
{
    return LENGTH;
}

template <unsigned int LENGTH>
ExprBase::ConstPtr AsExpr<Vector<LENGTH> >::argument(unsigned int i) const
{
    ASSERTSTR(i < LENGTH, "Invalid argument index specified.");
    return itsArg[i];
}

} //# namespace BBS
} //# namespace LOFAR

#endif
