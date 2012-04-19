//# Expr.cc: Expression base class
//#
//# Copyright (C) 2002
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
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

ExprId ExprBase::theirId = 0;


ExprBase::ExprBase()
    :   itsConsumerCount(0),
        itsCachePolicy(Cache::NONE),
        itsId(theirId++)
{
}

ExprBase::~ExprBase()
{
}

void ExprBase::connect(const ExprBase::ConstPtr &arg) const
{
    arg->incConsumerCount();
}

void ExprBase::disconnect(const ExprBase::ConstPtr &arg) const
{
    arg->decConsumerCount();
}

void ExprBase::incConsumerCount() const
{
    ++itsConsumerCount;
}

void ExprBase::decConsumerCount() const
{
    --itsConsumerCount;
}

bool ExprBase::isDependent() const
{
    for(unsigned int i = 0; i < nArguments(); ++i)
    {
        if(argument(i)->isDependent())
        {
            return true;
        }
    }

    return false;
}

} //# namespace BBS
} //# namespace LOFAR
