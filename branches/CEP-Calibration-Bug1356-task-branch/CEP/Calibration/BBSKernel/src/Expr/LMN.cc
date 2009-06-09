//# LMN.cc: LMN-coordinates of a direction on the sky.
//#
//# Copyright (C) 2005
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

#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

LMN::LMN(const PhaseRef::ConstPtr &ref,
    const Expr<Vector<2> >::ConstPtr &position)
    :   Expr1<Vector<2>, Vector<3> >(position),
        itsRef(ref)
{
}

const Vector<3>::proxy LMN::evaluateImpl(const Request &request,
    const Vector<2>::proxy &position) const
{
//    cout << "REF: " << itsRef->getRa() << " " << itsRef->getDec() << endl;
//    cout << "SOURCE: " << ra->value()(0, 0) << " " << dec->value()(0, 0) << endl;

    Matrix cosDec(cos(position(1)));
    Matrix deltaRa(position(0) - itsRef->getRa());

    Vector<3>::proxy result;
    result.assign(0, cosDec * sin(deltaRa));
    result.assign(1, sin(position(1)) * itsRef->getCosDec() - cosDec
        * itsRef->getSinDec() * cos(deltaRa));
    Matrix n = 1.0 - sqr(result(0)) - sqr(result(1));
    ASSERT(min(n).getDouble() >= 0.0);

    result.assign(2, sqrt(n));

//    cout << "LMN: " << result->value(0)(0, 0) << " " << result->value(1)(0, 0)
//        << " " << result->value(2)(0, 0) << endl;

    return result;
}

} // namespace BBS
} // namespace LOFAR
