//# Cache.h: Cache for ExprResult instances
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

#ifndef LOFAR_BBSKERNEL_EXPR_CACHE_H
#define LOFAR_BBSKERNEL_EXPR_CACHE_H

#include <Common/lofar_map.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <limits>

// \file
// Cache for ExprResult instances

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// TODO: fix include conflict with Expr.h.
typedef size_t ExprId;

struct CacheRecord
{
    CacheRecord()
        :   request(0),
            value(0)
    {
    }

    CacheRecord(RequestId request, const ExprValueSet *value)
        :   request(request),
            value(value)
    {
    }

    RequestId           request;
    const ExprValueSet  *value;
};


class Cache
{
public:
    Cache();

    ~Cache()
    {
        clear();
    }

    void insert(ExprId expr, RequestId request, const ExprValueSet &value)
    {
        CacheRecord &record = itsCache[expr];

        // If the cache already contains an old result, replace it.
        if(record.value == 0 || record.request < request)
        {
            delete record.value;
            record.request = request;
            record.value = new ExprValueSet(value);
        }

//        itsMemSize += result->memory();
        itsMaxSize = std::max(itsMaxSize, itsCache.size());
    }

    bool query(ExprId expr, RequestId request, ExprValueSet &value) const
    {
        ++itsQueryCount;

        map<ExprId, CacheRecord>::const_iterator it = itsCache.find(expr);
        if(it != itsCache.end() && it->second.request == request)
        {
            ++itsHitCount;
            ASSERT(it->second.value != 0);
            value = *(it->second.value);
            return true;
        }

        return false;
    }

    void clear()
    {
        map<ExprId, CacheRecord>::iterator it = itsCache.begin();
        while(it != itsCache.end())
        {
            delete it->second.value;
            ++it;
        }

        itsCache.clear();
    }

    void clearStats()
    {
        itsQueryCount = itsHitCount = itsMaxSize = itsMemSize = 0;
    }

    void printStats()
    {
        cout << "Cache statistics:" << endl;
        cout << "  max. size: " << itsMaxSize << endl;
        cout << "  mem. size: " << (double) itsMemSize / (1024*1024) << " MB"
            << endl;
        cout << "  #queries: " << itsQueryCount << endl;
        cout << "  #hits: " << itsHitCount << endl;
    }

private:
    mutable size_t  itsQueryCount, itsHitCount, itsMaxSize, itsMemSize;

    // TODO: Use hashtable?
    map<ExprId, CacheRecord>    itsCache;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
