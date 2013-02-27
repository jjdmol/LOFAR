//# TileArrayFactor.h: Compute the array factor of a LOFAR HBA tile.
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

#ifndef LOFAR_BBSKERNEL_EXPR_TILEARRAYFACTOR_H
#define LOFAR_BBSKERNEL_EXPR_TILEARRAYFACTOR_H

// \file
// Compute the array factor of a LOFAR HBA tile.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class TileArrayFactor: public BasicBinaryExpr<Vector<3>, Vector<3>, Scalar>
{
public:
    TileArrayFactor(const Expr<Vector<3> >::ConstPtr &direction,
        const Expr<Vector<3> >::ConstPtr &reference,
        const AntennaField::ConstPtr &field,
        bool conjugate = false);

protected:
    virtual const Scalar::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &direction, const Vector<3>::View &reference)
        const;

private:
    AntennaField::ConstPtr  itsField;
    bool                    itsConjugateFlag;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
