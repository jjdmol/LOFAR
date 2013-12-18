//# AntennaElementHBA.h: Model of an idealized LOFAR HBA dual dipole antenna
//# element.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_EXPR_ANTENNAELEMENTHBA_H
#define LOFAR_BBSKERNEL_EXPR_ANTENNAELEMENTHBA_H

// \file
// Model of an idealized LOFAR HBA dual dipole antenna element.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class AntennaElementHBA: public BasicUnaryExpr<Vector<2>, JonesMatrix>
{
public:
    typedef shared_ptr<AntennaElementHBA>       Ptr;
    typedef shared_ptr<const AntennaElementHBA> ConstPtr;

    AntennaElementHBA(const Expr<Vector<2> >::ConstPtr &target);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &target) const;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
