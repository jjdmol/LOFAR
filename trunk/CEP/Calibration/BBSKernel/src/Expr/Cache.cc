//# Cache.cc: Cache for ExprResult instances
//#
//# Copyright (C) 2009
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

#include <lofar_config.h>
#include <BBSKernel/Expr/Cache.h>

namespace LOFAR
{
namespace BBS
{

Cache::Cache()
    :   itsQueryCount(0),
        itsHitCount(0),
        itsMaxSize(0)
{
}

Cache::~Cache()
{
    clear();
}

void Cache::clear()
{
    map<ExprId, CacheRecord>::iterator it = itsCache.begin();
    while(it != itsCache.end())
    {
        delete it->second.result;
        ++it;
    }
    itsCache.clear();
}

void Cache::clear(Policy policy)
{
    unsigned int n = 0;

    map<ExprId, CacheRecord>::iterator it = itsCache.begin();
    while(it != itsCache.end())
    {
        if(it->second.policy == policy)
        {
            ++n;
            delete it->second.result;
            itsCache.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    LOG_DEBUG_STR("Cleaned " << n << " cached results for policy "
        << policyAsString(policy));
}

void Cache::clearStats()
{
    itsQueryCount = itsHitCount = itsMaxSize = 0;
}

Cache::CacheRecord::CacheRecord()
    :   request(0),
        result(0)
{
}

ostream &operator<<(ostream &out, const Cache &obj)
{
    out << "Cache statistics: queries: " << obj.itsQueryCount << " hits: "
        << obj.itsHitCount << " max. size: " << obj.itsMaxSize;
    return out;
}

const string &Cache::policyAsString(Policy policy)
{
    static const string policyNames[Cache::N_Policy] =
        {"NONE", "VOLATILE", "PERMANENT"};

    return policyNames[policy];
}

} //# namespace BBS
} //# namespace LOFAR
