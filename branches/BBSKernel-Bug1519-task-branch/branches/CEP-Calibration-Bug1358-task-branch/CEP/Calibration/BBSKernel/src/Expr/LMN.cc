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

LMN::LMN(const PhaseRef::ConstPointer &ref)
    :   ExprStatic<LMN::N_Arguments>(),
        itsRef(ref)
{
}

Shape LMN::shape(const ExprValueSet (&arguments)[LMN::N_Arguments]) const
{
    DBGASSERT(arguments[0].shape() == Shape(2));
    return Shape(3);
}

void LMN::evaluateImpl(const Request&,
    const ExprValue (&arguments)[LMN::N_Arguments], ExprValue &result) const
{
//    cout << "REF: " << itsRef->getRa() << " " << itsRef->getDec() << endl;
//    cout << "SOURCE: " << ra->value()(0, 0) << " " << dec->value()(0, 0) << endl;

    Matrix cosDec(cos(arguments[POSITION](1)));
    Matrix deltaRa(arguments[POSITION](0) - itsRef->getRa());

//    ValueSet::Ptr result(new ValueSet(3));
    result.assign(0, cosDec * sin(deltaRa));
    result.assign(1, sin(arguments[POSITION](1)) * itsRef->getCosDec() - cosDec
        * itsRef->getSinDec() * cos(deltaRa));
    Matrix n = 1.0 - sqr(result(0)) - sqr(result(1));
    ASSERT(min(n).getDouble() >= 0.0);

    result.assign(2, sqrt(n));

//    LOG_DEBUG_STR("LMN: " << result(0) << " " << result(1) << " " << result(2));
}

} // namespace BBS
} // namespace LOFAR
