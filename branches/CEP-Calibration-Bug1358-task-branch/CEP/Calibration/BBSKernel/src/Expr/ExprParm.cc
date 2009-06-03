//# ExprParm.cc: Parameter that can be used in an expression.
//#
//# Copyright (C) 2008
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
#include <BBSKernel/Expr/ExprParm.h>
//#include <BBSKernel/Expr/Cache.h>

#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS 
{

ExprParm::ExprParm(const ParmProxy::ConstPointer &parm)
    :   ExprTerminus(),
        itsParm(parm),
        itsPValueFlag(false)
{
}

void ExprParm::setPValueFlag()
{
    itsPValueFlag = true;
}

void ExprParm::clearPValueFlag()
{
    itsPValueFlag = false;
}

void ExprParm::updateSolvables(set<PValueKey> &solvables) const
{
    if(itsPValueFlag)
    {
        // TODO: The following is incorrect; need to know the id's of all the
        // _solvable_ coefficients (get the mask?).
        const size_t nCoeff = itsParm->getCoeffCount();
        for(size_t i = 0; i < nCoeff; ++i)
        {
            solvables.insert(PValueKey(itsParm->getId(), i));
        }            
    }
}

ValueSet::ConstPtr ExprParm::evaluateImpl(const Request&) const
{
    ASSERT(false);
}

ExprResult::ConstPtr ExprParm::evaluate(const Request &request, Cache &cache,
    const PValueKey &key) const
{
    // Query the cache.
    if(!key.valid() || itsParm->getId() != key.parmId)
    {
        // Have to return main value.
        // Check if there is a cached result available for this request.
        ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
            PValueKey());

        if(cached)
        {
            return cached;
        }
    }
    else
    {
        // Have to return perturbed value.
        // Check if there is a cached result available for this request.
        ExprResult::ConstPtr cached = cache.query(getId(), request.getId(),
            key);

        if(cached)
        {
            return cached;
        }
    }

    // Get the result from the Parm.
    vector<casa::Array<double> > buffers;
    itsParm->getResult(buffers, request.getGrid(), key.valid());
    ASSERT(buffers.size() > 0);

    // Transform into an ExprResult.
    ExprResult::Ptr result(new ExprResult());

    bool deleteStorage = false;
    const double *storage = 0;
        
    if(key.valid() && itsParm->getId() == key.parmId)
    {
        ASSERT(buffers.size() > key.coeffId + 1);
        const size_t bufIdx = key.coeffId + 1;

        // Copy the perturbed value.
        storage = buffers[bufIdx].getStorage(deleteStorage);
        ASSERT(storage);
        if(buffers[bufIdx].nelements() == 1)
        {
            ValueSet::Ptr tmp(new ValueSet());
            tmp->assign(Matrix(*storage));
            result->setValue(tmp);
        }
        else
        {
            const casa::IPosition &shape = buffers[bufIdx].shape();
            DBGASSERT(static_cast<unsigned int>(shape(0)) == request[FREQ]->size()
                && static_cast<unsigned int>(shape(1)) == request[TIME]->size());
            ValueSet::Ptr tmp(new ValueSet());
            tmp->assign(Matrix(storage, shape(0), shape(1)));
            result->setValue(tmp);
        }
        buffers[bufIdx].freeStorage(storage, deleteStorage);
    }
    else
    {
        // Copy the main value.
        storage = buffers[0].getStorage(deleteStorage);
        ASSERT(storage);
        if(buffers[0].nelements() == 1)
        {
            ValueSet::Ptr tmp(new ValueSet());
            tmp->assign(Matrix(*storage));
            result->setValue(tmp);
        }
        else
        {
            const casa::IPosition &shape = buffers[0].shape();
            DBGASSERT(static_cast<unsigned int>(shape(0)) == request[FREQ]->size()
                && static_cast<unsigned int>(shape(1)) == request[TIME]->size());
            ValueSet::Ptr tmp(new ValueSet());
            tmp->assign(Matrix(storage, shape(0), shape(1)));
            result->setValue(tmp);
        }
        buffers[0].freeStorage(storage, deleteStorage);
    }

    if(!key.valid() || getConsumerCount() > 1)
    {
        cache.insert(getId(), request.getId(), key, result);
    }
    
    return result;
}

//ValueSet::ConstPtr ExprParm::evaluateImpl(const Request &request) const
//{
//    // Get the result from the Parm.
//    vector<casa::Array<double> > buffers;
//    itsParm->getResult(buffers, request.getGrid(), itsPValueFlag
//        && request.getPValueFlag());
//    ASSERT(buffers.size() > 0);

//    // Transform into a Result.
//    ValueSet::Ptr result(new ValueSet());

//    bool deleteStorage;
//    const double *storage = 0;
//    
//    // Copy the main value.
//    storage = buffers[0].getStorage(deleteStorage);
//    ASSERT(storage);
//    if(buffers[0].nelements() == 1)
//    {
//        result->assign(Matrix(storage[0]));
//    }
//    else
//    {
//        const casa::IPosition &shape = buffers[0].shape();
//        result->assign(Matrix(storage, shape(0), shape(1)));
//    }
//    buffers[0].freeStorage(storage, deleteStorage);

//    // Copy the perturbed values if necessary.
////    if(itsPValueFlag && request.getPValueFlag())
////    {
////        const size_t nCoeff = itsParm->getCoeffCount();
////        ASSERT(buffers.size() == nCoeff + 1);
////        
////        for(size_t i = 0; i < nCoeff; ++i)
////        {
////            storage = buffers[i + 1].getStorage(deleteStorage);
////            ASSERT(storage);
////            if(buffers[i + 1].nelements() == 1)
////            {
////                result.setPerturbedValue(PValueKey(itsParm->getId(), i),
////                    Matrix(storage[0]));
////            }
////            else
////            {
////                const casa::IPosition &shape = buffers[i + 1].shape();
////                result.setPerturbedValue(PValueKey(itsParm->getId(), i),
////                    Matrix(storage, shape(0), shape(1)));
////            }
////            buffers[i + 1].freeStorage(storage, deleteStorage);
////        }
////    }
//    
//    return result;
//}

} //# namespace BBS
} //# namespace LOFAR
