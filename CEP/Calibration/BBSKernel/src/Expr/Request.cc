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

Request::Request(const Grid &grid, bool evalPValues)
    :   itsId(theirId++),
        itsGrid(grid),
        itsPValueFlag(evalPValues)
{
}        

Request::~Request()
{
}

} //# namespace BBS
} //# namespace LOFAR
