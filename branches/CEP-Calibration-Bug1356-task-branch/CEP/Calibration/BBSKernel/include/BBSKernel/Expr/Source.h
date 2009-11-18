//# PointSource.h: Abstract base class for holding a source
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSKERNEL_EXPR_SOURCE_H
#define LOFAR_BBSKERNEL_EXPR_SOURCE_H

// \file
// Abstract base class for holding a source

#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Source
{
public:
    typedef shared_ptr<Source>          Ptr;
    typedef shared_ptr<const Source>    ConstPtr;

    Source();
    Source(const string &name, const Expr<Vector<2> >::ConstPtr &position);
    virtual ~Source();

    const string &getName() const;
    Expr<Vector<2> >::ConstPtr getPosition() const;

protected:
    string                      itsName;
    Expr<Vector<2> >::ConstPtr  itsPosition;
};

// @}

// -------------------------------------------------------------------------- //
// - Source implementation                                                  - //
// -------------------------------------------------------------------------- //

inline const string &Source::getName() const
{
    return itsName;
}

inline Expr<Vector<2> >::ConstPtr Source::getPosition() const
{
    return itsPosition;
}

} // namespace BBS
} // namespace LOFAR

#endif
