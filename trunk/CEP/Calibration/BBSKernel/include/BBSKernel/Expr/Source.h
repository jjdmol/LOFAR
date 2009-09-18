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

#ifndef EXPR_SOURCE_H
#define EXPR_SOURCE_H

// \file
// Abstract base class for holding a source

#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class Source
{
public:
    typedef shared_ptr<Source>       Pointer;
    typedef shared_ptr<const Source> ConstPointer;

    Source(const string &name, const Expr &ra, const Expr &dec);
    virtual ~Source();

    const string &getName() const
    { return itsName; }

    const Expr &getRa() const
    { return itsRa; }
    const Expr &getDec() const
    { return itsDec; }

protected:
    string  itsName;
    Expr    itsRa;
    Expr    itsDec;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
