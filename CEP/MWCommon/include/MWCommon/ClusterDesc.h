//# ClusterDesc.h:  Description of a cluster and the nodes in it
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

#ifndef LOFAR_MWCOMMON_CLUSTERDESC_H
#define LOFAR_MWCOMMON_CLUSTERDESC_H

// @file
// @brief Description of a cluster and the nodes in it.

//# Includes
#include <MWCommon/NodeDesc.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Description of a cluster and the nodes in it.

  // This class holds the basic description of a cluster.
  // It defines which nodes are part of the cluster and which file systems
  // each node has access to.
  // If a data set is distributed over many file systems, the cluster
  // description tells which node can handle a data set part on a particular
  // file system.
  //
  // Currently the information is made persistent in a LOFAR .parset file.
  // In the future it needs to use the Centrol Processor Resource Manager.

  class ClusterDesc
  {
  public:
    // Construct an empty object.
    ClusterDesc()
      {}

    // Construct from the given parameterset.
    // @{
    explicit ClusterDesc (const std::string& parsetName);
    explicit ClusterDesc (const ParameterSet& parset)
      { init (parset); }
    // @}

    // Set cluster name.
    void setName (const std::string& name)
      { itsName = name; }

    // Set head node name.
    void setHeadNode (const std::string& headNode)
      { itsHeadNode = headNode; }

    // Add a file system it has access to.
    void addNode (const NodeDesc& node);

    // Write it in parset format.
    void write (std::ostream& os) const;

    // Get the name.
    const std::string& getName() const
      { return itsName; }

    // Get the head node.
    const std::string& getHeadNode() const
      { return itsHeadNode; }

    // Get all nodes.
    const std::vector<NodeDesc>& getNodes() const
      { return itsNodes; }

    // Get the map of file system to node.
    const std::map<std::string, std::vector<std::string> >& getMap() const
      { return itsFS2Nodes; }

  private:
    // Fill the object from the given parset file.
    void init (const ParameterSet& parset);

    // Fill the object from the subcluster definitions.
    void getSubClusters (const vector<string>& parsetNames);

    // Add entries to the mapping of FileSys to Nodes.
    void add2Map (const NodeDesc& node);

    std::string itsName;
    std::string itsHeadNode;
    std::vector<NodeDesc> itsNodes;
    std::map<std::string, std::vector<std::string> > itsFS2Nodes;
  };
    
}} // end namespaces

#endif
