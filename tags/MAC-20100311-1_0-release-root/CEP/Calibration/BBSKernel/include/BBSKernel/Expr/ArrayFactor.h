//# ArrayFactor.h: Compute the array factor of a LOFAR station.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_ARRAYFACTOR_H
#define LOFAR_BBSKERNEL_EXPR_ARRAYFACTOR_H

// \file
// Compute the array factor of a LOFAR station.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ArrayFactor: public BasicBinaryExpr<Vector<2>, Vector<2>, JonesMatrix>
{
public:
    ArrayFactor(const Expr<Vector<2> >::ConstPtr &direction,
        const Expr<Vector<2> >::ConstPtr &reference,
        const AntennaSelection &selection, double referenceFreq);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &direction, const Vector<2>::View &reference)
        const;

private:
    AntennaSelection    itsSelection;
    double              itsReferenceFreq;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
