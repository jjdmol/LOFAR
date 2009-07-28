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

#ifndef LOFAR_BBS_EXPR_EXPRADAPTORS_H
#define LOFAR_BBS_EXPR_EXPRADAPTORS_H

// \file
// Helper classes to convert between different Expr types.

#include <BBSKernel/Expr/Expr.h>

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
    typedef shared_ptr<AsExpr>  Ptr;
    typedef shared_ptr<AsExpr>  ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    AsExpr()
    {
    }

    template <typename T_ITERATOR>
    AsExpr(T_ITERATOR first, T_ITERATOR last)
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            ASSERT(first != last);
            itsArg[i] = *first++;
            connect(itsArg[i]);
        }
        ASSERT(first == last);
    }

    ~AsExpr()
    {
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            disconnect(itsArg[i]);
        }
    }

    void connect(unsigned int i0, const typename Expr<Scalar>::ConstPtr &arg)
    {
        connect(arg);
        itsArg[i0] = arg;
    }

    virtual const Vector<LENGTH> evaluateExpr(const Request &request,
        Cache &cache) const
    {
        // Allocate result.
        Vector<LENGTH> result;

        // Evaluate arguments (pass through).
        Scalar args[LENGTH];
        for(unsigned int i = 0; i < LENGTH; ++i)
        {
            args[i] = itsArg[i]->evaluate(request, cache);
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

protected:
    virtual unsigned int nArguments() const
    {
        return LENGTH;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        ASSERTSTR(i < LENGTH, "Invalid argument index specified.");
        return itsArg[i];
    }

private:
    typename Expr<Scalar>::ConstPtr itsArg[LENGTH];
};

// Adaptor class to bundle multiple Expr<Scalar> into a single
// Expr<JonesMatrix>.
template <>
class AsExpr<JonesMatrix>: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<AsExpr>  Ptr;
    typedef shared_ptr<AsExpr>  ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    AsExpr()
    {
    }

    AsExpr(const Expr<Scalar>::ConstPtr &element00,
        const Expr<Scalar>::ConstPtr &element01,
        const Expr<Scalar>::ConstPtr &element10,
        const Expr<Scalar>::ConstPtr &element11)
    {
        itsArg[0] = element00;
        connect(itsArg[0]);
        itsArg[1] = element01;
        connect(itsArg[1]);
        itsArg[2] = element10;
        connect(itsArg[2]);
        itsArg[3] = element11;
        connect(itsArg[3]);
    }

    ~AsExpr()
    {
        for(unsigned int i = 0; i < 4; ++i)
        {
            disconnect(itsArg[i]);
        }
    }

    void connect(unsigned int i1, unsigned int i0,
        const Expr<Scalar>::ConstPtr &arg)
    {
        DBGASSERT(i1 < 2 && i0 < 2);
        connect(arg);
        itsArg[i1 * 2 + i0] = arg;
    }

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        JonesMatrix result;

        // Evaluate arguments (pass through).
        Scalar args[4];
        for(unsigned int i = 0; i < 4; ++i)
        {
            args[i] = itsArg[i]->evaluate(request, cache);
            result.setValueSet(i, args[i].getValueSet());
        }

        // Evaluate flags.
        FlagArray flags[4];
        for(unsigned int i = 0; i < 4; ++i)
        {
            flags[i] = args[i].flags();
        }
        result.setFlags(mergeFlags(flags, flags + 4));

        return result;
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 4;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        ASSERTSTR(i < 4, "Invalid argument index specified.");
        return itsArg[i];
    }

private:
    Expr<Scalar>::ConstPtr  itsArg[4];
};


// Adaptor class to bundle two Expr<Scalar> into a single diagonal
// Expr<JonesMatrix>.
class AsDiagonalMatrix: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<AsDiagonalMatrix>  Ptr;
    typedef shared_ptr<AsDiagonalMatrix>  ConstPtr;

    using ExprBase::connect;
    using ExprBase::disconnect;

    AsDiagonalMatrix()
    {
    }

    AsDiagonalMatrix(const Expr<Scalar>::ConstPtr &element00,
        const Expr<Scalar>::ConstPtr &element11)
    {
        itsArg[0] = element00;
        connect(itsArg[0]);
        itsArg[1] = element11;
        connect(itsArg[1]);
    }

    ~AsDiagonalMatrix()
    {
        for(unsigned int i = 0; i < 2; ++i)
        {
            disconnect(itsArg[i]);
        }
    }

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache)
        const
    {
        // Allocate result.
        JonesMatrix result;

        // Evaluate arguments (pass through).
        Scalar args[2];
        args[0] = itsArg[0]->evaluate(request, cache);
        args[1] = itsArg[1]->evaluate(request, cache);

        result.setValueSet(0, 0, args[0].getValueSet());
        result.assign(0, 1, Matrix(makedcomplex(0.0, 0.0)));
        result.assign(1, 0, Matrix(makedcomplex(0.0, 0.0)));
        result.setValueSet(1, 1, args[1].getValueSet());

        // Evaluate flags.
        FlagArray flags[2];
        flags[0] = args[0].flags();
        flags[1] = args[1].flags();
        result.setFlags(mergeFlags(flags, flags + 2));

        return result;
    }

protected:
    virtual unsigned int nArguments() const
    {
        return 2;
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        ASSERTSTR(i < 2, "Invalid argument index specified.");
        return itsArg[i];
    }

private:
    Expr<Scalar>::ConstPtr  itsArg[2];
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
        const Expr<Scalar>::ConstPtr &im)
        : BasicBinaryExpr<Scalar, Scalar, Scalar>(re, im)
    {
    }

private:
    virtual const Scalar::view evaluateImpl(const Request&,
        const Scalar::view &re, const Scalar::view &im) const
    {
        Scalar::view result;
        result.assign(tocomplex(re(), im()));
        return result;
    }
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
        const Expr<Scalar>::ConstPtr &argument)
        : BasicBinaryExpr<Scalar, Scalar, Scalar>(modulus, argument)
    {
    }

private:
    virtual const Scalar::view evaluateImpl(const Request&,
        const Scalar::view &mod, const Scalar::view &arg) const
    {
        Scalar::view result;
        result.assign(tocomplex(mod() * cos(arg()), mod() * sin(arg())));
        return result;
    }
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
