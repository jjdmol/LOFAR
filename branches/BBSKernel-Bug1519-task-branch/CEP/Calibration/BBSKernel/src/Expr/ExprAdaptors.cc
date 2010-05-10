//# ExprAdaptors.cc: Helper classes to convert between different Expr types.
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

#include <lofar_config.h>
#include <BBSKernel/Expr/ExprAdaptors.h>

namespace LOFAR
{
namespace BBS
{

// -------------------------------------------------------------------------- //
// - Implementation: AsExpr<JonesMatrix>                                    - //
// -------------------------------------------------------------------------- //

AsExpr<JonesMatrix>::AsExpr()
{
}

AsExpr<JonesMatrix>::AsExpr(const Expr<Scalar>::ConstPtr &element00,
    const Expr<Scalar>::ConstPtr &element01,
    const Expr<Scalar>::ConstPtr &element10,
    const Expr<Scalar>::ConstPtr &element11)
{
    connect(element00);
    itsArg[0] = element00;
    connect(element01);
    itsArg[1] = element01;
    connect(element10);
    itsArg[2] = element10;
    connect(element11);
    itsArg[3] = element11;
}

AsExpr<JonesMatrix>::~AsExpr()
{
    for(unsigned int i = 0; i < 4; ++i)
    {
        disconnect(itsArg[i]);
    }
}

void AsExpr<JonesMatrix>::connect(unsigned int i1, unsigned int i0,
    const Expr<Scalar>::ConstPtr &arg)
{
    DBGASSERT(i1 < 2 && i0 < 2);
    connect(arg);
    itsArg[i1 * 2 + i0] = arg;
}

const JonesMatrix AsExpr<JonesMatrix>::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Allocate result.
    JonesMatrix result;

    // Evaluate arguments (pass through).
    Scalar args[4];
    for(unsigned int i = 0; i < 4; ++i)
    {
        args[i] = itsArg[i]->evaluate(request, cache, grid);
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

unsigned int AsExpr<JonesMatrix>::nArguments() const
{
    return 4;
}

ExprBase::ConstPtr AsExpr<JonesMatrix>::argument(unsigned int i) const
{
    ASSERTSTR(i < 4, "Invalid argument index specified.");
    return itsArg[i];
}

// -------------------------------------------------------------------------- //
// - Implementation: AsDiagonalMatrix                                       - //
// -------------------------------------------------------------------------- //

AsDiagonalMatrix::AsDiagonalMatrix(const Expr<Scalar>::ConstPtr &element00,
    const Expr<Scalar>::ConstPtr &element11)
    :   BinaryExpr<Scalar, Scalar, JonesMatrix>(element00, element11)
{
}

const JonesMatrix AsDiagonalMatrix::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Allocate result.
    JonesMatrix result;

    // Evaluate arguments (pass through).
    Scalar args[2];
    args[0] = argument0()->evaluate(request, cache, grid);
    args[1] = argument1()->evaluate(request, cache, grid);

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

// -------------------------------------------------------------------------- //
// - Implementation: AsComplex                                              - //
// -------------------------------------------------------------------------- //

AsComplex::AsComplex(const Expr<Scalar>::ConstPtr &re,
        const Expr<Scalar>::ConstPtr &im)
        : BasicBinaryExpr<Scalar, Scalar, Scalar>(re, im)
{
}

const Scalar::View AsComplex::evaluateImpl(const Grid&, const Scalar::View &re,
    const Scalar::View &im) const
{
    Scalar::View result;
    result.assign(tocomplex(re(), im()));
    return result;
}

// -------------------------------------------------------------------------- //
// - Implementation: AsPolar                                                - //
// -------------------------------------------------------------------------- //

AsPolar::AsPolar(const Expr<Scalar>::ConstPtr &modulus,
    const Expr<Scalar>::ConstPtr &argument)
    : BasicBinaryExpr<Scalar, Scalar, Scalar>(modulus, argument)
{
}

const Scalar::View AsPolar::evaluateImpl(const Grid&, const Scalar::View &mod,
    const Scalar::View &arg) const
{
    Scalar::View result;
    result.assign(tocomplex(mod() * cos(arg()), mod() * sin(arg())));
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
