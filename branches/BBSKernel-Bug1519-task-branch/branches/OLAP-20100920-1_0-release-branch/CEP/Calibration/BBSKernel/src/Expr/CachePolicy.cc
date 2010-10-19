//# CachePolicy.cc: Assign sensible cache behaviour settings to all Expr
//# instances of an ExprSet.
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
#include <BBSKernel/Expr/CachePolicy.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

void DefaultCachePolicy::applyImpl(const ExprBase::ConstPtr &root) const
{
    cacheSharedExpr(root);
}

void DefaultCachePolicy::cacheSharedExpr(const ExprBase::ConstPtr &root) const
{
    root->setCachePolicy(root->nConsumers() > 1 ? Cache::VOLATILE
        : Cache::NONE);

    for(unsigned int i = 0, last = root->nArguments(); i < last; ++i)
    {
        cacheSharedExpr(root->argument(i));
    }
}

void ExperimentalCachePolicy::applyImpl(const ExprBase::ConstPtr &root) const
{
    cacheSharedExpr(root);
    cacheIndepExpr(root);
}

void ExperimentalCachePolicy::cacheIndepExpr(const ExprBase::ConstPtr &root)
    const
{
    if(!root->isDependent())
    {
        root->setCachePolicy(Cache::PERMANENT);
    }
    else
    {
        for(unsigned int i = 0, last = root->nArguments(); i < last; ++i)
        {
            cacheIndepExpr(root->argument(i));
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR
