//# FlagArray.cc: An array of flags that can transparently handle rank 0
//# (scalar) rank 2 (matrix) flag data.
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

#include <Common/LofarLogger.h>
#include <BBSKernel/Expr/FlagArray.h>

namespace LOFAR
{
namespace BBS
{

// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayImplScalar                                    - //
// -------------------------------------------------------------------------- //

FlagArrayImpl *FlagArrayImplScalar::opBitWiseOr(const FlagArrayImplScalar &lhs,
    bool lhsTmp, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseOr FlagArrayImplScalar" << (lhsTmp ? " (temp)" : "")
        << " FlagArrayImplScalar" << (rhsTmp ? " (temp)" : ""));

    FlagArrayImplScalar *result =
        lhsTmp ? const_cast<FlagArrayImplScalar*>(&lhs)
        : (rhsTmp ? const_cast<FlagArrayImplScalar*>(this)
        :  new FlagArrayImplScalar());
        
    result->value() = lhs.value() | value();
    return result;
}

FlagArrayImpl *FlagArrayImplScalar::opBitWiseOr(const FlagArrayImplMatrix &lhs,
    bool lhsTmp, bool) const
{
    LOG_DEBUG_STR("opBitWiseOr FlagArrayImplMatrix" << (lhsTmp ? " (temp)" : "")
        << " FlagArrayImplScalar");

    FlagArrayImplMatrix *result =
        lhsTmp ? const_cast<FlagArrayImplMatrix*>(&lhs)
        : new FlagArrayImplMatrix(lhs.shape(1), lhs.shape(0));

    FlagArrayImplMatrix::const_iterator lhs_it = lhs.begin();
    FlagArrayImplMatrix::const_iterator lhs_end = lhs.end();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(lhs_it != lhs_end)
    {
        *result_it = *lhs_it | value();
        ++lhs_it;
        ++result_it;
    }

    return result;
}

FlagArrayImpl *FlagArrayImplScalar::opBitWiseAnd(const FlagArrayImplScalar &lhs,
    bool lhsTmp, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseAnd FlagArrayImplScalar"
        << (lhsTmp ? " (temp)" : "") << " FlagArrayImplScalar"
        << (rhsTmp ? " (temp)" : ""));

    FlagArrayImplScalar *result =
        lhsTmp ? const_cast<FlagArrayImplScalar*>(&lhs)
        : (rhsTmp ? const_cast<FlagArrayImplScalar*>(this)
        :  new FlagArrayImplScalar());
        
    result->value() = lhs.value() & value();
    return result;
}

FlagArrayImpl *FlagArrayImplScalar::opBitWiseAnd(const FlagArrayImplMatrix &lhs,
    bool lhsTmp, bool) const
{
    LOG_DEBUG_STR("opBitWiseAnd FlagArrayImplMatrix"
        << (lhsTmp ? " (temp)" : "") << " FlagArrayImplScalar");

    FlagArrayImplMatrix *result =
        lhsTmp ? const_cast<FlagArrayImplMatrix*>(&lhs)
        : new FlagArrayImplMatrix(lhs.shape(1), lhs.shape(0));

    FlagArrayImplMatrix::const_iterator lhs_it = lhs.begin();
    FlagArrayImplMatrix::const_iterator lhs_end = lhs.end();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(lhs_it != lhs_end)
    {
        *result_it = *lhs_it & value();
        ++lhs_it;
        ++result_it;
    }

    return result;
}

// -------------------------------------------------------------------------- //
// - Implementation: FlagArrayImplScalar                                    - //
// -------------------------------------------------------------------------- //

FlagArrayImpl *FlagArrayImplMatrix::opBitWiseOr(const FlagArrayImplScalar &lhs,
    bool, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseOr FlagArrayImplScalar FlagArrayImplMatrix"
        << (rhsTmp ? " (temp)" : ""));

    FlagArrayImplMatrix *result =
        rhsTmp ? const_cast<FlagArrayImplMatrix*>(this)
        : new FlagArrayImplMatrix(shape(1), shape(0));

    FlagArrayImplMatrix::const_iterator rhs_it = begin();
    FlagArrayImplMatrix::const_iterator rhs_end = end();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(rhs_it != rhs_end)
    {
        *result_it = lhs.value() | *rhs_it;
        ++rhs_it;
        ++result_it;
    }

    return result;
}

FlagArrayImpl *FlagArrayImplMatrix::opBitWiseOr(const FlagArrayImplMatrix &lhs,
    bool lhsTmp, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseOr FlagArrayImplMatrix" << (lhsTmp ? " (temp)" : "")
        << " FlagArrayImplMatrix" << (rhsTmp ? " (temp)" : ""));
    DBGASSERT(lhs.size() == size());

    FlagArrayImplMatrix *result =
        lhsTmp ? const_cast<FlagArrayImplMatrix*>(&lhs)
        : (rhsTmp ? const_cast<FlagArrayImplMatrix*>(this)
        : new FlagArrayImplMatrix(shape(1), shape(0)));
        
    // TODO: Optimization opportunity: result_it could be the same as lhs_it
    // or rhs_it.
    FlagArrayImplMatrix::const_iterator lhs_it = lhs.begin();
    FlagArrayImplMatrix::const_iterator lhs_end = lhs.end();
    FlagArrayImplMatrix::const_iterator rhs_it = begin();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(lhs_it != lhs_end)
    {
        *result_it = *lhs_it | *rhs_it;
        ++lhs_it;
        ++rhs_it;
        ++result_it;
    }

    return result;
}

FlagArrayImpl *FlagArrayImplMatrix::opBitWiseAnd(const FlagArrayImplScalar &lhs,
    bool, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseAnd FlagArrayImplScalar FlagArrayImplMatrix"
        << (rhsTmp ? " (temp)" : ""));

    FlagArrayImplMatrix *result =
        rhsTmp ? const_cast<FlagArrayImplMatrix*>(this)
        : new FlagArrayImplMatrix(shape(1), shape(0));

    FlagArrayImplMatrix::const_iterator rhs_it = begin();
    FlagArrayImplMatrix::const_iterator rhs_end = end();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(rhs_it != rhs_end)
    {
        *result_it = lhs.value() & *rhs_it;
        ++rhs_it;
        ++result_it;
    }

    return result;
}

FlagArrayImpl *FlagArrayImplMatrix::opBitWiseAnd(const FlagArrayImplMatrix &lhs,
    bool lhsTmp, bool rhsTmp) const
{
    LOG_DEBUG_STR("opBitWiseAnd FlagArrayImplMatrix"
        << (lhsTmp ? " (temp)" : "")
        << " FlagArrayImplMatrix" << (rhsTmp ? " (temp)" : ""));
    DBGASSERT(lhs.size() == size());

    FlagArrayImplMatrix *result =
        lhsTmp ? const_cast<FlagArrayImplMatrix*>(&lhs)
        : (rhsTmp ? const_cast<FlagArrayImplMatrix*>(this)
        : new FlagArrayImplMatrix(shape(1), shape(0)));
        
    // TODO: Optimization opportunity: result_it could be the same as lhs_it
    // or rhs_it.
    FlagArrayImplMatrix::const_iterator lhs_it = lhs.begin();
    FlagArrayImplMatrix::const_iterator lhs_end = lhs.end();
    FlagArrayImplMatrix::const_iterator rhs_it = begin();
    FlagArrayImplMatrix::iterator result_it = result->begin();
    while(lhs_it != lhs_end)
    {
        *result_it = *lhs_it & *rhs_it;
        ++lhs_it;
        ++rhs_it;
        ++result_it;
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
