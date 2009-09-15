//# WorkersDesc.cc: Description of a workers
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite LOFARsoft.
//# LOFARsoft is free software: you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# LOFARsoft is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with LOFARsoft. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen <diepen AT astron nl>

#include <lofar_config.h>

#include <MWCommon/WorkersDesc.h>
#include <algorithm>

using namespace std;

namespace LOFAR { namespace CEP {

  WorkersDesc::WorkersDesc (const ClusterDesc& cd)
    : itsClusterDesc (cd)
  {
    // Reserve for 128 workers; might get more.
    itsLoad.reserve (128);
  }

  void WorkersDesc::addWorker (unsigned workerId, const string& nodeName,
			       const vector<int>& workTypes)
  {
    // Resize load vector if needed.
    // Initialize load for this worker.
    if (workerId >= itsLoad.size()) {
      itsLoad.resize (workerId+1);
    }
    itsLoad[workerId] = 0;
    // Add the worker to the list of nodes per worker type.
    for (vector<int>::const_iterator iter=workTypes.begin();
	 iter != workTypes.end();
	 ++iter) {
      // Get worker map for this worker type; gets created if not existing.
      MapN2W& mnw = itsMap[*iter];
      // Append to list of workers for this node.
      vector<unsigned>& vec = mnw[nodeName];
      vec.push_back (workerId);
    }
  }

  int WorkersDesc::findWorker (int workType, const string& fileSystem,
                               NodeDesc::NodeType type) const
  {
    // Find the worker type.
    map<int,MapN2W>::const_iterator workMap = itsMap.find(workType);
    if (workMap == itsMap.end()) {
      return -1;
    }
    // The worker has to operate on a given file system, so only nodes
    // with access to it will be considered.
    // If the file system is empty, no specific file system is needed and
    // any node may do.
    if (fileSystem.empty()) {
      return findLowest (workMap->second, type);
    }
    return findLowest (workMap->second, fileSystem, type);
  }

  int WorkersDesc::findLowest (const MapN2W& workMap,
                               NodeDesc::NodeType type) const
  {
    // Find worker with lowest load.
    int worker = -1;
    int load   = 1000000;
    // Loop over all nodes and find worker with lowest load
    // for which the node type matches.
    for (MapN2W::const_iterator workers = workMap.begin();
	 workers != workMap.end();
	 ++workers) {
      // Only use node if its type matches.
      bool use = true;
      if (type != NodeDesc::Any) {
        const vector<NodeDesc>& nodes = itsClusterDesc.getNodes();
        for (vector<NodeDesc>::const_iterator niter = nodes.begin();
             niter != nodes.end(); ++niter) {
          if (workers->first == niter->getName()) {
            use = niter->getType() == NodeDesc::Any || niter->getType() == type;
            break;
          }
        }
      }
      if (use) {
        const vector<unsigned>& workTypes = workers->second;
        for (vector<unsigned>::const_iterator witer=workTypes.begin();
             witer != workTypes.end();
             ++witer) {
          // We can stop if a worker with load 0 is found.
          if (itsLoad[*witer] < load) {
            worker = *witer;
            load   = itsLoad[*witer];
            if (load == 0) break;
          }
        }
        if (load == 0) break;
      }
    }
    return worker;
  }

  int WorkersDesc::findLowest (const MapN2W& workMap,
			       const string& fileSystem,
                               NodeDesc::NodeType type) const
  {
    // Find worker with lowest load.
    int worker = -1;
    int load   = 1000000;
    // Get all nodes with access to the file system.
    const MapF2N& fs2Nodes = itsClusterDesc.getMap();
    MapF2N::const_iterator nodes = fs2Nodes.find(fileSystem);
    if (nodes == fs2Nodes.end()) {
      return -1;
    }
    const vector<int>& nodesVec = nodes->second;
    for (vector<int>::const_iterator iter=nodesVec.begin();
	 iter != nodesVec.end();
	 ++iter) {
      // Loop over all suitable nodes and find worker with lowest load.
      const NodeDesc& node = itsClusterDesc.getNodes()[*iter];
      if (node.getType() == type
      ||  node.getType() == NodeDesc::Any  ||  type == NodeDesc::Any) {
        MapN2W::const_iterator workers = workMap.find(node.getName());
        if (workers != workMap.end()) {
          const vector<unsigned>& workTypes = workers->second;
          for (vector<unsigned>::const_iterator witer=workTypes.begin();
               witer != workTypes.end();
               ++witer) {
            // We can stop if a worker with load 0 is found.
            if (itsLoad[*witer] < load) {
              worker = *witer;
              load   = itsLoad[*witer];
              if (load == 0) break;
            }
          }
          if (load == 0) break;
        }
      }
    }
    return worker;
  }

}} // end namespaces
