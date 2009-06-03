//# Expr.cc: Expression base class
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
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprResult.h>
//#include <BBSKernel/Expr/Cache.h>

namespace LOFAR
{
namespace BBS 
{

NodeId Expr::theirId = 0;

//ARRAY(flag_t) ExprBase::evaluateFlags(const Request &request,
//    const vector<ConstResultPtr> &inputs) const
//{
//    ARRAY(flag_t) flags;

//    vector<ConstResultPtr>::const_iterator inputIt = inputs.begin();
//    while(inputIt != inputs.end())
//    {
//        if((*inputIt)->hasFlags())
//        {
//            flags = (*inputIt)->getFlags();
//        }
//        ++inputIt;
//    }
//    
//    while(inputIt != inputs.end())
//    {
//        if((*inputIt)->hasFlags())
//        {
//            flags |= (*inputIt)->getFlags();
//        }
//        ++inputIt;
//    }
//    
//    return flags;
//}

//ExprBase::ConstResultPtr ExprBase::evaluateBase(const Request &request,
//    const PValueKey &key, bool perturbed, Cache &cache) const
//{
////    ASSERT(perturbed == key.valid());
//    
//    if(!perturbed || itsSolvables.find(key) == itsSolvables.end())
//    {
//        // Have to return main value.
//        // Check if there is a cached result available for this request.
//        ConstResultPtr cached = cache.query(getId(), request.getId(),
//            PValueKey());

//        if(cached)
//        {
////            if(perturbed)
////            {
////                LOG_DEBUG("Using cached main value for perturbed value.");
////            }
////            
//            return cached;
//        }
//    }
//    else
//    {
//        // Have to return perturbed value.
//        // Check if there is a cached result available for this request.
//        ConstResultPtr cached = cache.query(getId(), request.getId(), key);

//        if(cached)
//        {
////            LOG_DEBUG("Found cached perturbed value.");
//            return cached;
//        }
//    }
//        
//    // TODO: Check for NULL inputs.
//    vector<ConstResultPtr> inputs(itsInputs.size());
//    for(size_t i = 0; i < itsInputs.size(); ++i)
//    {
//        inputs[i] = itsInputs[i]->evaluateBase(request, key, perturbed, cache);
//    }

//    // TODO: Should we need the flags during processing, we can compute
//    // then beforehand and pass them to evaluateImpl().
//    ResultPtr result = const_pointer_cast<ExprResult>(evaluateImpl(request, inputs));
//    result->setFlags(evaluateFlags(request, inputs));
//    
//    if(!perturbed && itsConsumerCount > 1)
//    {
//        cache.insert(getId(), request.getId(), PValueKey(), result);
//    }
//    else if(itsConsumerCount > 1)
//    {
////        LOG_DEBUG_STR("Consumer count: " << itsConsumerCount);
////        if(perturbed)
////        {
//        cache.insert(getId(), request.getId(), key, result);
////        }
////        else
////        {
////        }
//    }        
//    
//    // No cached result, so compute the value and store it in the buffer
//    // that was passed in.
//    return result;
//}
    
} //# namespace BBS
} //# namespace LOFAR
