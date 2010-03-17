//# Scope.cc: A list of ExprParm instances that are used by a model expression.
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

#include <lofar_config.h>
#include <BBSKernel/Expr/Scope.h>

#include <BBSKernel/ParmManager.h>

namespace LOFAR
{
namespace BBS
{

void Scope::clear()
{
    itsParms.clear();
}

ExprParm::Ptr Scope::operator()(unsigned int category, const string &name)
{
    ParmProxy::ConstPtr proxy(ParmManager::instance().get(category, name));

    pair<Scope::const_iterator, bool> status =
        itsParms.insert(make_pair(proxy->getId(),
            ExprParm::Ptr(new ExprParm(proxy))));

    return status.first->second;
}

Scope::iterator Scope::begin()
{
    return itsParms.begin();
}

Scope::iterator Scope::end()
{
    return itsParms.end();
}

Scope::const_iterator Scope::begin() const
{
    return itsParms.begin();
}

Scope::const_iterator Scope::end() const
{
    return itsParms.end();
}

Scope::iterator Scope::find(unsigned int id)
{
    return itsParms.find(id);
}

Scope::const_iterator Scope::find(unsigned int id) const
{
    return itsParms.find(id);
}

} //# namespace BBS
} //# namespace LOFAR
