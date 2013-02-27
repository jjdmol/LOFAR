//# IonPhaseShift.h: Compute the ionospheric phase shift for a particular pierce
//# point using a single layer ionospheric model.
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

#ifndef LOFAR_BBSKERNEL_EXPR_IONPHASESHIFT_H
#define LOFAR_BBSKERNEL_EXPR_IONPHASESHIFT_H

// \file
// Compute the ionospheric phase shift for a particular pierce point using a
// single layer ionospheric model.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class IonPhaseShift: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<IonPhaseShift>       Ptr;
    typedef shared_ptr<const IonPhaseShift> ConstPtr;

    IonPhaseShift(const Expr<Vector<4> >::ConstPtr &piercePoint,
        const Expr<Scalar>::ConstPtr &r0,
        const Expr<Scalar>::ConstPtr &beta);

    virtual ~IonPhaseShift();

    template <typename T>
    void setCalibratorPiercePoints(T first, T last);

    template <typename T>
    void setTECWhite(T first, T last);

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    Expr<Vector<4> >::ConstPtr          itsPiercePoint;
    Expr<Scalar>::ConstPtr              itsR0;
    Expr<Scalar>::ConstPtr              itsBeta;

    vector<Expr<Vector<3> >::ConstPtr>  itsCalPiercePoint;
    vector<Expr<Vector<2> >::ConstPtr>  itsTECWhite;
};

// @}

// -------------------------------------------------------------------------- //
// - IonPhaseShift implementation                                           - //
// -------------------------------------------------------------------------- //

template <typename T>
void IonPhaseShift::setCalibratorPiercePoints(T first, T last)
{
    itsCalPiercePoint = vector<Expr<Vector<3> >::ConstPtr>(first, last);
    for(; first != last; ++first)
    {
        connect(*first);
    }
}

template <typename T>
void IonPhaseShift::setTECWhite(T first, T last)
{
    itsTECWhite = vector<Expr<Vector<2> >::ConstPtr>(first, last);
    for(; first != last; ++first)
    {
        connect(*first);
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
