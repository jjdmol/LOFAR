//# Scope.h: A list of ExprParm instances that are used by a model expression.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_SCOPE_H
#define LOFAR_BBSKERNEL_EXPR_SCOPE_H

// \file
// A list of ExprParm instances that are used by a model expression.

#include <Common/lofar_map.h>
#include <Common/lofar_string.h>

#include <BBSKernel/Expr/ExprParm.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Scope
{
public:
    typedef map<unsigned int, ExprParm::Ptr>::iterator          iterator;
    typedef map<unsigned int, ExprParm::Ptr>::const_iterator    const_iterator;

    void clear();

    ExprParm::Ptr operator()(unsigned int category, const string &name);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    iterator find(unsigned int id);
    const_iterator find(unsigned int id) const;

private:
    map<unsigned int, ExprParm::Ptr>    itsParms;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
