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
typedef size_t NodeId;

class CacheRecord
{
public:
//    CacheRecord(NodeId node, RequestId request)
//        :   node(node),
//            request(request),
//            key(std::numeric_limits<size_t>::max(),
//                std::numeric_limits<size_t>::max())
//    {
//    }            

    CacheRecord(NodeId node, RequestId request, const PValueKey &key)
        :   node(node),
            request(request),
            key(key)
    {
    }

    bool operator<(const CacheRecord &other) const
    {
        return node < other.node || (node == other.node && request < other.request)
            || (node == other.node && request == other.request && key < other.key);
    }
    
private:
    NodeId      node;
    RequestId   request;
    PValueKey   key;
};

class Cache
{
public:
    Cache();
    ~Cache();

//    void insert(NodeId node, RequestId request,
//        const ExprResult::ConstPtr &result)
//    {
//        itsCache.insert(make_pair(CacheRecord(node, request,
//            PValueKey(std::numeric_limits<size_t>::max(),
//                std::numeric_limits<size_t>::max())), result));
//        
//        itsMaxSize = std::max(itsMaxSize, itsCache.size());
//    }
            
    void insert(NodeId node, RequestId request, const PValueKey &key,
        const ExprResult::ConstPtr &result)
    {
        itsCache.insert(make_pair(CacheRecord(node, request, key), result));
//        itsMemSize += result->memory();
        itsMaxSize = std::max(itsMaxSize, itsCache.size());
    }

//    ExprResult::ConstPtr query(NodeId node, RequestId request) const
//    {
//        ++itsQueryCount;
//        
//        map<pair<size_t, RequestId>, ExprResult::ConstPtr>::const_iterator it =
//            itsCache.find(CacheRecord(node, request,
//                PValueKey(std::numeric_limits<size_t>::max(),
//                    std::numeric_limits<size_t>::max())));
//                        
//        if(it != itsCache.end())
//        {
////            LOG_DEBUG_STR("Found result in cache: " << node << ":" << request);
//            ++itsHitCount;
//            return it->second;
//        }
//                    
//        return ExprResult::ConstPtr();
//    }
    
    ExprResult::ConstPtr query(NodeId node, RequestId request,
        const PValueKey &key) const
    {
        ++itsQueryCount;
        
        map<CacheRecord, ExprResult::ConstPtr>::const_iterator it =
            itsCache.find(CacheRecord(node, request, key));
        
        if(it != itsCache.end())
        {
//            LOG_DEBUG_STR("Found result in cache: " << node << ":" << request);
            ++itsHitCount;
            return it->second;
        }
                    
        return ExprResult::ConstPtr();
    }

    void clear()
    {
        itsCache.clear();
        itsQueryCount = itsHitCount = itsMaxSize = itsMemSize = 0;
    }
    
    void printStatistics()
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
    map<CacheRecord, ExprResult::ConstPtr>  itsCache;    
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
