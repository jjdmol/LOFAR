//# WorkerDesc.h: Description of all workers
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_LMWCOMMON_WORKERSDESC_H
#define LOFAR_LMWCOMMON_WORKERSDESC_H

// @file
// @brief Description of all workers.
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <LMWCommon/ClusterDesc.h>
#include <string>
#include <vector>

namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Description of all workers.

  // This class holds the description of the workers in an MW run.
  // For each worker it describes on which node it runs and which types
  // of work it can perform.
  // Combined with the cluster description, this information is used
  // to determine which worker can be used to perform a given type of work
  // on data on a given file system. In determining this, it keeps track of
  // the workload to avoid that the same worker is selected again and again.

  class WorkersDesc
  {
  public:
    // Construct from the given cluster description.
    WorkersDesc (const ClusterDesc&);

    // Add a worker with the given id.
    // It can do the work types given in the vector on the given node.
    void addWorker (unsigned id, const std::string& node,
		    const std::vector<int>& workTypes);

    // Increase or decrease the load for the given worker.
    // Return the new load.
    // @{
    int incrLoad (unsigned worker)
      { return ++itsLoad[worker]; }
    int decrLoad (unsigned worker)
      { return --itsLoad[worker]; }
    // @}

    // Find the worker with the lowest load that can perform the given
    // work type for data on the given file system on a given type of node.
    // By default it looks for Compute Nodes.
    // The given file system can be empty indicating that any worker can do it.
    // It returns -1 if no suitable worker could be found.
    // Otherwise the worker id is returned.
    int findWorker (int workType, const std::string& fileSystem,
                    NodeDesc::NodeType type = NodeDesc::Compute) const;

  private:
    // Map giving the workers on each node.
    typedef std::map<std::string, std::vector<unsigned> > MapN2W;
    // Map given the nodes with access to a file system.
    typedef std::map<std::string, std::vector<int> > MapF2N;

    // Find worker with lowest load.
    // @{
    int findLowest (const MapN2W& workMap,
                    NodeDesc::NodeType type) const;
    int findLowest (const MapN2W& workMap,
		    const std::string& fileSystem,
                    NodeDesc::NodeType type) const;
    // @}

    const ClusterDesc&   itsClusterDesc;
    std::map<int,MapN2W> itsMap;      //# map worktype to nodename/workerid
    std::vector<int>     itsLoad;     //# load of each worker (#times used)
  };
    
}} //# end namespaces

#endif
