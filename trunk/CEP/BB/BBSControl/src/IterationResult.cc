//# IterationResult.cc: Result of a single non-linear Levenberg-Marquardt
//#                     iteration sent from solver to kernel.
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

#include <BBSControl/IterationResult.h>
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

    const string IterationResult::theirClassType = "IterationResult";
    
    // Register IterationResult with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
        bool dummy = BlobStreamableFactory::instance().registerClass<IterationResult>("IterationResult");
        bool dummy_vector = BlobStreamableFactory::instance().registerClass<BlobStreamableVector<IterationResult> >("BlobStreamableVector<IterationResult>");
    }
    
    void IterationResult::write(BlobOStream& bos) const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bos << itsDomainIndex
            << itsResultCode
            << itsResultText
            << itsUnknowns
            << itsRank
            << itsChiSquared
            << itsLMFactor;
    }


    void IterationResult::read(BlobIStream& bis)
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        
        bis >> itsDomainIndex
            >> itsResultCode
            >> itsResultText
            >> itsUnknowns
            >> itsRank
            >> itsChiSquared
            >> itsLMFactor;
    }


    const string& IterationResult::classType() const
    {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_RTTI, "");
        return IterationResult::theirClassType;
    }
} // namespace BBS
} // namespace LOFAR
