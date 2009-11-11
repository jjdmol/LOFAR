//# Cache.h: Cache for expression values.
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

#include <BBSKernel/Expr/ExprId.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ExprResult.h>

#include <Common/lofar_map.h>

// \file
// Cache for expression values.

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Cache
{
public:
    Cache();
    ~Cache();

    template <typename T_EXPR_VALUE>
    void insert(ExprId expr, RequestId request, const T_EXPR_VALUE &result);

    template <typename T_EXPR_VALUE>
    bool query(ExprId expr, RequestId request, T_EXPR_VALUE &value);

    void clear();
    void clearStats();

private:
    class CacheRecord
    {
    public:
        CacheRecord();

        RequestId   request;
        ExprValue   *result;
    };

    friend ostream &operator<<(ostream &out, const Cache &obj);

    size_t                      itsQueryCount;
    size_t                      itsHitCount;
    size_t                      itsMaxSize;

    // TODO: Use hashtable?
    map<ExprId, CacheRecord>    itsCache;
};

// iostream I/O.
ostream &operator<<(ostream &out, const Cache &obj);

// @}


// -------------------------------------------------------------------------- //
// - Implementation: Cache                                                  - //
// -------------------------------------------------------------------------- //

template <typename T_EXPR_VALUE>
void Cache::insert(ExprId expr, RequestId request, const T_EXPR_VALUE &result)
{
    CacheRecord &record = itsCache[expr];
    itsMaxSize = std::max(itsMaxSize, itsCache.size());

    if(record.result == 0 || record.request < request)
    {
        ExprValue* tmp = record.result;
        record.result = new T_EXPR_VALUE(result);
        delete tmp;
        record.request = request;
    }
}

template <typename T_EXPR_VALUE>
bool Cache::query(ExprId expr, RequestId request, T_EXPR_VALUE &value)
{
    ++itsQueryCount;

    map<ExprId, CacheRecord>::const_iterator it = itsCache.find(expr);
    if(it != itsCache.end() && it->second.request == request)
    {
        ASSERT(it->second.result != 0);
//        DBGASSERT(dynamic_cast<const T_EXPR_VALUE*>(it->second.result));
        value = *(static_cast<T_EXPR_VALUE*>(it->second.result));
        ++itsHitCount;
        return true;
    }

    return false;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
