//# WorkerInfo.h: Information about a worker
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_MWCOMMON_WORKERINFO_H
#define LOFAR_MWCOMMON_WORKERINFO_H

// @file
// @brief Information about a worker.
// @author Ger van Diepen (diepen AT astron nl)

#include <string>
#include <vector>

//# Forward Declarations.
namespace LOFAR {
  class BlobOStream;
  class BlobIStream;
}


namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Information about a worker.

  // This class contains the information describing a worker.
  // It contains the name of the host it is running on and a vector
  // with the types of work in can perform. Currently only the first
  // work type is taken into account.
  //
  // @todo Take all work types into account.

  class WorkerInfo
  {
  public:
    // Creatye empty object.
    WorkerInfo();

    // Construct the object from the given info.
    WorkerInfo (const std::string& hostName,
                const std::vector<int>& workTypes);

    ~WorkerInfo();

    // Get the host name.
    const std::string& getHostName() const
      { return itsHostName; }

    // Get the work types.
    const std::vector<int>& getWorkTypes() const
      { return itsWorkTypes; }

    // Get the first work type. Returns 0 if no work types.
    int getWorkType() const;

    // Read or write the info from/into a blob.
    // @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
                                           const WorkerInfo& info);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
                                           WorkerInfo& info);
    // @}

  private:
    std::string      itsHostName;
    std::vector<int> itsWorkTypes;
  };

}} //# end namespaces

#endif
