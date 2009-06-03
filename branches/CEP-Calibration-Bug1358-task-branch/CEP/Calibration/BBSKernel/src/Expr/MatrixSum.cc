//# MatrixSum.h: Compute the (element-wise) sum of a collection of Jones
//# matrices.
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

#include <BBSKernel/Expr/MatrixSum.h>

namespace LOFAR
{
namespace BBS
{

MatrixSum::MatrixSum()
    : ExprDynamic()
{
}    

ValueSet::ConstPtr MatrixSum::evaluateImpl(const Request &request,
    const vector<ValueSet::ConstPtr> &inputs) const
{
    ASSERT(inputs.size() > 0);
    ASSERT(inputs[0]->rank() == 2 && inputs[0]->size() == 4);

    int nx = request[FREQ]->size();
    int ny = request[TIME]->size();
    
    Matrix el00(makedcomplex(0,0), nx, ny);
    Matrix el01(makedcomplex(0,0), nx, ny);
    Matrix el10(makedcomplex(0,0), nx, ny);
    Matrix el11(makedcomplex(0,0), nx, ny);

//    Matrix el00 = inputs[0]->value(0, 0).clone();
//    Matrix el01 = inputs[0]->value(0, 1).clone();
//    Matrix el10 = inputs[0]->value(1, 0).clone();
//    Matrix el11 = inputs[0]->value(1, 1).clone();

//    for(size_t i = 1; i < inputs.size(); ++i)
    for(size_t i = 0; i < inputs.size(); ++i)
    {
        ASSERT(inputs[i]->rank() == 2 && inputs[i]->size() == 4);
        el00 += inputs[i]->value(0, 0);
        el01 += inputs[i]->value(0, 1);
        el10 += inputs[i]->value(1, 0);
        el11 += inputs[i]->value(1, 1);
    }
    
    ValueSet::Ptr result(new ValueSet(2, 2));
    result->assign(0, 0, el00);
    result->assign(1, 0, el01);
    result->assign(0, 1, el10);
    result->assign(1, 1, el11);

    return result;
}

} // namespace BBS
} // namespace LOFAR
