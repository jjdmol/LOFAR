//# MeasurementExpr.h: A measurement equation (expression) for a set of
//# interferometers (baselines).
//#
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSKERNEL_MEASUREMENTEXPR_H
#define LOFAR_BBSKERNEL_MEASUREMENTEXPR_H

// \file
// A measurement equation (expression) for a set of interferometers (baselines).

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/VisDimensions.h>
#include <BBSKernel/Expr/ExprValue.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class MeasurementExpr: public ExprSet<JonesMatrix>
{
public:
    typedef shared_ptr<MeasurementExpr>         Ptr;
    typedef shared_ptr<const MeasurementExpr>   ConstPtr;

    virtual const BaselineSeq &baselines() const = 0;
    virtual const CorrelationSeq &correlations() const = 0;
};

//Expr<JonesMatrix>::Ptr compose(const Expr<JonesMatrix>::Ptr &accumulator,
//    const Expr<JonesMatrix>::Ptr &effect);

//Expr<JonesMatrix>::Ptr corrupt(const Expr<JonesMatrix>::Ptr &lhs,
//    const Expr<JonesMatrix>::Ptr &coherence,
//    const Expr<JonesMatrix>::Ptr &rhs);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
