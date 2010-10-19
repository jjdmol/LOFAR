//# Request.h: Request grid(s) on which to evaluate an expression.
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

#ifndef LOFAR_BBSKERNEL_EXPR_REQUEST_H
#define LOFAR_BBSKERNEL_EXPR_REQUEST_H

// \file
// Request grid(s) on which to evaluate an expression.

// This include defines names for the first axis (FREQ) and second axis (TIME),
// which are more readable than plain 0 and 1.
#include <BBSKernel/Types.h>

#include <Common/LofarLogger.h>
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
    const Box &domain() const;
    const Grid &operator[](unsigned int i) const;
    void append(const Grid &grid);

private:
    RequestId           itsId;
    Box                 itsDomain;
    vector<Grid>        itsGrid;

    static RequestId    theirId;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: Request                                                - //
// -------------------------------------------------------------------------- //

inline RequestId Request::id() const
{
    return itsId;
}

inline const Box &Request::domain() const
{
    return itsDomain;
}

inline const Grid &Request::operator[](unsigned int i) const
{
    DBGASSERT(i < itsGrid.size());
    return itsGrid[i];
}

} //# namespace BBS
} //# namespace LOFAR

#endif
