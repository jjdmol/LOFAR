//# ElementBeamExpr.h: Wrapper class that constructs the correct beam expr based
//# on the BeamConfig instance provided in the constructor. It also caches any
//# shared auxilliary data.
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

#ifndef LOFAR_BBSKERNEL_ELEMENTBEAMEXPR_H
#define LOFAR_BBSKERNEL_ELEMENTBEAMEXPR_H

// \file
// Wrapper class that constructs the correct beam expr based on the BeamConfig
// instance provided in the constructor. It also caches any shared auxilliary
// data.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/HamakerDipole.h>

#include <Common/lofar_smartptr.h>

#include <casa/OS/Path.h>

namespace LOFAR
{
namespace BBS
{

class BeamConfig;
class Scope;

// \addtogroup BBSKernel
// @{

class ElementBeamExpr
{
public:
    typedef shared_ptr<ElementBeamExpr>         Ptr;
    typedef shared_ptr<const ElementBeamExpr>   ConstPtr;

    static ElementBeamExpr::Ptr create(const BeamConfig &config, Scope &scope);

    virtual ~ElementBeamExpr();

    virtual Expr<JonesMatrix>::Ptr
        construct(const Expr<Vector<2> >::ConstPtr &direction,
            const Expr<Scalar>::ConstPtr &orientation) const = 0;
};

class HamakerBeamExpr: public ElementBeamExpr
{
public:
    typedef shared_ptr<HamakerBeamExpr>         Ptr;
    typedef shared_ptr<const HamakerBeamExpr>   ConstPtr;

    HamakerBeamExpr(const BeamConfig &config, Scope &scope);

    virtual Expr<JonesMatrix>::Ptr
        construct(const Expr<Vector<2> >::ConstPtr &direction,
            const Expr<Scalar>::ConstPtr &orientation) const;

private:
    HamakerBeamCoeff    itsCoeff;
};

class YatawattaBeamExpr: public ElementBeamExpr
{
public:
    typedef shared_ptr<YatawattaBeamExpr>       Ptr;
    typedef shared_ptr<const YatawattaBeamExpr> ConstPtr;

    YatawattaBeamExpr(const BeamConfig &config, Scope &scope);

    virtual Expr<JonesMatrix>::Ptr
        construct(const Expr<Vector<2> >::ConstPtr &direction,
            const Expr<Scalar>::ConstPtr &orientation) const;

private:
    casa::Path  itsModulePath[2];
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
