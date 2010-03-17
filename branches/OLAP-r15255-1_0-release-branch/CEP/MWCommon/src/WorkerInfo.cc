//# WorkerInfo.cc: 
//#
//# Copyright (c) 2007
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

#include <MWCommon/WorkerInfo.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;

namespace LOFAR { namespace CEP {

  WorkerInfo::WorkerInfo()
  {}

  WorkerInfo::WorkerInfo (const std::string& hostName,
                          const std::vector<int>& workTypes)
    : itsHostName  (hostName),
      itsWorkTypes (workTypes)
  {}

  WorkerInfo::~WorkerInfo()
  {}

  int WorkerInfo::getWorkType() const
  {
    return (itsWorkTypes.size() == 0  ?  0 : itsWorkTypes[0]);
  }

  BlobOStream& operator<< (BlobOStream& bs, const WorkerInfo& info)
  {
    bs.putStart ("info", 1);
    bs << info.itsHostName << info.itsWorkTypes;
    bs.putEnd();
    return bs;
  }

  BlobIStream& operator>> (BlobIStream& bs, WorkerInfo& info)
  {
    int version = bs.getStart ("info");
    ASSERT (version == 1);
    bs >> info.itsHostName >> info.itsWorkTypes;
    bs.getEnd();
    return bs;
  }

}} // end namespaces
