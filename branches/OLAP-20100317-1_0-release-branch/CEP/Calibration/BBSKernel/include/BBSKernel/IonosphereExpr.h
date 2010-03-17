//# IonosphereExpr.h: Wrapper class that constructs ionosphere expressions based
//# on the IonosphereConfig instance provided to the constructor. The class also
//# stores a reference to any shared auxilliary data.
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

#ifndef LOFAR_BBSKERNEL_IONOSPHEREEXPR_H
#define LOFAR_BBSKERNEL_IONOSPHEREEXPR_H

// \file
// Wrapper class that constructs ionosphere expressions based on the
// IonosphereConfig instance provided to the constructor. The class also stores
// a reference to any shared auxilliary data.

#include <BBSKernel/Expr/Expr.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>

namespace casa
{
    class MPosition;
}

namespace LOFAR
{
namespace BBS
{

class IonosphereConfig;
class Scope;

// \addtogroup BBSKernel
// @{

class IonosphereExpr
{
public:
    typedef shared_ptr<IonosphereExpr>          Ptr;
    typedef shared_ptr<const IonosphereExpr>    ConstPtr;

    static IonosphereExpr::Ptr create(const IonosphereConfig &config,
        Scope &scope);

    virtual ~IonosphereExpr();

    virtual Expr<JonesMatrix>::Ptr construct(const casa::MPosition &position,
        const Expr<Vector<4> >::ConstPtr &piercePoint) const = 0;
};

class PolynomialLayerExpr: public IonosphereExpr
{
public:
    typedef shared_ptr<PolynomialLayerExpr>         Ptr;
    typedef shared_ptr<const PolynomialLayerExpr>   ConstPtr;

    PolynomialLayerExpr(const IonosphereConfig &config, Scope &scope);

    virtual Expr<JonesMatrix>::Ptr construct(const casa::MPosition &position,
        const Expr<Vector<4> >::ConstPtr &piercePoint) const;

private:
    vector<Expr<Scalar>::Ptr>   itsCoeff;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
