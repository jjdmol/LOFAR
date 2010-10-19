//# PointSource.h: Class holding the expressions defining a point source.
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_BBSKERNEL_EXPR_POINTSOURCE_H
#define LOFAR_BBSKERNEL_EXPR_POINTSOURCE_H

// \file
// Class holding the expressions defining a point source.

#include <BBSKernel/Expr/Source.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class PointSource: public Source
{
public:
    typedef shared_ptr<PointSource>         Ptr;
    typedef shared_ptr<const PointSource>   ConstPtr;

    PointSource(const string &name, const Expr<Vector<2> >::ConstPtr &position,
        const Expr<Vector<4> >::ConstPtr &stokes);

    const Expr<Vector<4> >::ConstPtr &getStokesVector() const
    {
        return itsStokesVector;
    }

private:
    Expr<Vector<4> >::ConstPtr  itsStokesVector;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
