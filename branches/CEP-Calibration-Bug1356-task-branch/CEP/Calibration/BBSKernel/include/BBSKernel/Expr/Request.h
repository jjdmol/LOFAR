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

#ifndef EXPR_REQUEST_H
#define EXPR_REQUEST_H

// \file
// Request grid on which to evaluate an expression.

#include <ParmDB/Grid.h>
#include <BBSKernel/Types.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

typedef int RequestId;
const RequestId InitRequestId = -1;

class Request
{
public:
    Request();

    Request(const Grid &grid, bool evalPValues = false);
    ~Request();

    RequestId id() const
    { return itsId; }

    Box getBoundingBox() const
    { return itsGrid.getBoundingBox(); }

    const Axis::ShPtr &operator[](size_t n) const
    { return itsGrid[n]; }

    const Grid &getGrid() const
    { return itsGrid; }

    bool getPValueFlag() const
    { return itsPValueFlag; }

private:
    size_t              itsId;
    Grid                itsGrid;
    bool                itsPValueFlag;

    static RequestId    theirId;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
