//# PointSource.h: Abstract base class for holding a source
//#
//# Copyright (C) 2006
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

    const string &getName() const
    {
        return itsName;
    }

    const Expr<Vector<2> >::ConstPtr &getPosition() const
    {
        return itsPosition;
    }

protected:
    string                      itsName;
    Expr<Vector<2> >::ConstPtr  itsPosition;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
