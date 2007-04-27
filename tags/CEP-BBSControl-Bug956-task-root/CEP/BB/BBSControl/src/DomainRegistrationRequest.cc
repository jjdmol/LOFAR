//# DomainRegistrationRequest.cc: request sent from kernel to solver to
//#     register a solve domain and set the initial values of its
//#     parameters.
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

#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/BlobStreamableVector.h>
#include <Common/LofarLogger.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;
    using LOFAR::operator>>;

    const string DomainRegistrationRequest::theirClassType = "DomainRegistrationRequest";
    
    // Register DomainRegistrationRequest with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
        bool dummy = BlobStreamableFactory::instance().registerClass<DomainRegistrationRequest>("DomainRegistrationRequest");
        bool dummy_vector = BlobStreamableFactory::instance().registerClass<BlobStreamableVector<DomainRegistrationRequest> >("BlobStreamableVector<" + DomainRegistrationRequest::theirClassType + ">");
    }
    
    void DomainRegistrationRequest::write(BlobOStream& bos) const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bos << itsDomainIndex
            << itsEpsilon
            << itsMaxIter
            << itsUnknowns;
    }


    void DomainRegistrationRequest::read(BlobIStream& bis)
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bis >> itsDomainIndex
            >> itsEpsilon
            >> itsMaxIter
            >> itsUnknowns;
    }


    const string& DomainRegistrationRequest::classType() const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        return DomainRegistrationRequest::theirClassType;
    }
} // namespace BBS
} // namespace LOFAR
