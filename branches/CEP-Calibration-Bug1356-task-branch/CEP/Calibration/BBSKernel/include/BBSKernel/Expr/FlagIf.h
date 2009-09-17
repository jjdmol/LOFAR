//# FlagIf.h: Flag the argument by setting the flags for all samples for which
//# the predicate evaluates to true.
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

#ifndef LOFAR_BBSKERNEL_EXPR_FLAGIF_H
#define LOFAR_BBSKERNEL_EXPR_FLAGIF_H

// \file
// Flag the argument by setting the flags for all samples for which the
// predicate evaluates to true.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// TODO: Generalize to other ExprValue types?
template <typename T_PREDICATE>
class FlagIf: public UnaryExpr<Scalar, Scalar>
{
public:
    typedef shared_ptr<FlagIf>          Ptr;
    typedef shared_ptr<const FlagIf>    ConstPtr;

    using UnaryExpr<Scalar, Scalar>::argument0;

    FlagIf(const Expr<Scalar>::ConstPtr &arg0, T_PREDICATE predicate,
        FlagType mask);

protected:
    virtual const Scalar evaluateExpr(const Request &request, Cache &cache)
        const;

private:
    T_PREDICATE itsPredicate;
    FlagType    itsFlagMask;
};

// Helper function to create a FlagIf instance.
template <typename T_PREDICATE>
typename FlagIf<T_PREDICATE>::Ptr makeFlagIf(const Expr<Scalar>::ConstPtr &arg0,
    T_PREDICATE predicate, FlagType mask = FlagType(1))
{
    return typename FlagIf<T_PREDICATE>::Ptr(new FlagIf<T_PREDICATE>(arg0,
        predicate, mask));
}

// @}


// -------------------------------------------------------------------------- //
// - Implementation: FlagIf                                                 - //
// -------------------------------------------------------------------------- //

template <typename T_PREDICATE>
FlagIf<T_PREDICATE>::FlagIf(const Expr<Scalar>::ConstPtr &arg0,
    T_PREDICATE predicate, FlagType mask)
        :   UnaryExpr<Scalar, Scalar>(arg0),
            itsPredicate(predicate),
            itsFlagMask(mask),
{
}

template <typename T_PREDICATE>
const Scalar FlagIf<T_PREDICATE>::evaluateExpr(const Request &request,
    Cache &cache) const
{
    // Evaluate argument.
    const Scalar arg0 = argument0()->evaluate(request, cache);

    // Create result.
    Scalar result;
    // Pass through value.
    result.setValueSet(arg0.getValueSet());

    // Compute flags.
    const Matrix value = arg0.value();
    ASSERTSTR(!value.isComplex(), "Only real valued input supported.");

    if(value.isArray())
    {
        const size_t nFreq = request[FREQ]->size();
        const size_t nTime = request[TIME]->size();

        FlagArray flags(nFreq, nTime);
        FlagArray::iterator flagIt = flags.begin();
        FlagArray::iterator flagItEnd = flags.end();

        const double *valueIt = value.doubleStorage();
        ASSERT(static_cast<size_t>(value.nelements()) == nFreq * nTime);

        while(flagIt != flagItEnd)
        {
            *flagIt++ = itsPredicate(*valueIt++) ? itsFlagMask : 0;
        }

        result.setFlags(arg0.hasFlags() ? arg0.flags() | flags : flags);
    }
    else if(itsPredicate(value.getDouble()))
    {
        FlagArray flags(itsFlagMask);
        result.setFlags(arg0.hasFlags() ? arg0.flags() | flags : flags);
    }

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
