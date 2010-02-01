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

class Scope;
class SourceInfo;

// \addtogroup Expr
// @{

class Source
{
public:
    typedef shared_ptr<Source>          Ptr;
    typedef shared_ptr<const Source>    ConstPtr;

    virtual ~Source();

    static Source::Ptr create(const SourceInfo &source, Scope &scope);

    const string &name() const;
    Expr<Vector<2> >::ConstPtr position() const;

    virtual Expr<JonesMatrix>::Ptr
        coherence(const Expr<Vector<3> >::ConstPtr &uvwLHS,
            const Expr<Vector<3> >::ConstPtr &uvwRHS) const = 0;

protected:
    Source(const SourceInfo &source, Scope &scope);

    string                  itsName;
    Expr<Vector<2> >::Ptr   itsPosition;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
