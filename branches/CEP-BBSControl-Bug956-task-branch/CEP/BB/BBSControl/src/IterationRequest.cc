//# IterationRequest.h: Request sent from kernel to solver to perform one
//#                     non-linear Levenberg-Marquardt iteration. 
//#
//# Copyright (C) 2006
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

#include <BBSControl/IterationRequest.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/BlobStreamableVector.h>

#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>
#include <casa/BasicSL/String.h>

#include <boost/scoped_array.hpp>
    
#include <Common/LofarLogger.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobAipsIO.h>

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;

    const string IterationRequest::theirClassType = "IterationRequest";
        
    // Register IterationRequest with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
        bool dummy = BlobStreamableFactory::instance().registerClass<IterationRequest>("IterationRequest");
        bool dummy_vector = BlobStreamableFactory::instance().registerClass<BlobStreamableVector<IterationRequest> >("BlobStreamableVector<" + IterationRequest::theirClassType + ">");
    }
    
    void IterationRequest::write(BlobOStream& bos) const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bos << itsDomainIndex;
        
        BlobAipsIO aipsBuffer(bos);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;
        
        if(!itsNormalEquations.toRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSControlException, "Unable to serialise normal equations (" << aipsErrorMessage << ")");
        }        
        
        // Serialise LSQFit object.
        aipsStream << aipsRecord;
        
        // DEBUG DEBUG DEBUG DEBUG DEBUG
        /*
        casa::MemoryIO aipsMemoryBuffer;
        casa::AipsIO aipsMemoryStream(&aipsMemoryBuffer);
        aipsMemoryStream << aipsRecord;
        LOG_DEBUG_STR("LSQFit-object: nUnknowns: " << itsNormalEquations.nUnknowns() << ", size: " << aipsMemoryBuffer.length() << " bytes");
        */
        // DEBUG DEBUG DEBUG DEBUG DEBUG
    }


    void IterationRequest::read(BlobIStream& bis)
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bis >> itsDomainIndex;
        
        BlobAipsIO aipsBuffer(bis);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;

        aipsStream >> aipsRecord;        
        if(!itsNormalEquations.fromRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSControlException, "Unable to deserialise normal equations (" << aipsErrorMessage << ")");
        }
    }


    const string& IterationRequest::classType() const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        return IterationRequest::theirClassType;
    }
} // namespace BBS
} // namespace LOFAR
