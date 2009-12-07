//# Request.cc: Request grid on which to evaluate an expression.
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <BBSKernel/Expr/Request.h>

namespace LOFAR
{
namespace BBS
{

RequestId Request::theirId = 0;

Request::Request()
    :   itsId(theirId++)
{
}

Request::Request(const Grid &grid)
    :   itsId(theirId++),
//        itsBoundingBox(grid.getBoundingBox()),
        itsDomain(grid.getBoundingBox())
//        itsActiveGrid(0)
{
    append(grid);
}

void Request::append(const Grid &grid)
{
#ifdef LOFAR_DEBUG
    const Box &box = grid.getBoundingBox();
    DBGASSERT(casa::near(box.lowerX(), itsDomain.lowerX())
        && casa::near(box.lowerY(), itsDomain.lowerY())
        && casa::near(box.upperX(), itsDomain.upperX())
        && casa::near(box.upperY(), itsDomain.upperY()));
#endif

    itsGrid.push_back(grid);
}

//void Request::pushGrid() const
//{
//    ++itsActiveGrid;
//    DBGASSERT(itsActiveGrid < itsGrid.size());
//}

//void Request::popGrid() const
//{
//    DBGASSERT(itsActiveGrid > 0);
//    --itsActiveGrid;
//}

} //# namespace BBS
} //# namespace LOFAR
