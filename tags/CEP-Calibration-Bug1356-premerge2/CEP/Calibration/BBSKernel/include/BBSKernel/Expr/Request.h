//# Request.h: Request grid on which to evaluate an expression.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_EXPR_REQUEST_H
#define LOFAR_BBSKERNEL_EXPR_REQUEST_H

// \file
// Request grid on which to evaluate an expression.

#include <BBSKernel/Types.h>
#include <ParmDB/Grid.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

typedef size_t RequestId;

class Request
{
public:
    Request();
    Request(const Grid &grid);

    RequestId id() const;

    Box getBoundingBox() const;

    const Axis::ShPtr &operator[](unsigned int i) const;

    const Grid &getGrid() const;

private:
    RequestId           itsId;
    Grid                itsGrid;

    static RequestId    theirId;
};

// @}


// -------------------------------------------------------------------------- //
// - Implementation: RefCountable                                           - //
// -------------------------------------------------------------------------- //

inline RequestId Request::id() const
{
    return itsId;
}

inline Box Request::getBoundingBox() const
{
    return itsGrid.getBoundingBox();
}

inline const Axis::ShPtr &Request::operator[](unsigned int i) const
{
    return itsGrid[i];
}

inline const Grid &Request::getGrid() const
{
    return itsGrid;
}

//bool Request::getPValueFlag() const
//{
//    return itsPValueFlag;
//}

} //# namespace BBS
} //# namespace LOFAR

#endif
