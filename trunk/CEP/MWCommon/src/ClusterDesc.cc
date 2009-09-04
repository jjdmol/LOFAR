//# ClusterDesc.cc: Description of a cluster
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/ClusterDesc.h>

using namespace std;

namespace LOFAR { namespace CEP {

  ClusterDesc::ClusterDesc (const string& parsetName)
  {
    init (ParameterSet (parsetName));
  }

  void ClusterDesc::init (const ParameterSet& parset)
  {
    itsName     = parset.getString ("ClusterName");
    itsHeadNode = parset.getString ("HeadNode", string());
    int nnode   = parset.getInt    ("NNodes", 0);
    if (nnode > 0) {
      // The cluster can be heterogeneous and is described in detail.
      for (int i=0; i<nnode; ++i) {
        ostringstream prefix;
        prefix << "Node" << i << '.';
        ParameterSet subset = parset.makeSubset (prefix.str());
        NodeDesc node(subset);
        addNode (node);
      }
    } else if (parset.isDefined ("SubClusters")) {
      getSubClusters (parset.getStringVector ("SubClusters", true));
    } else {
      // The cluster is homogeneous and is described like that.
      // All nodes share the same global mount points and have the same
      // local disks names.
      vector<string> computeNodes =
        parset.get("ComputeNodes").expand().getStringVector();
      vector<string> mountPoints  =
        parset.get("MountPoints").expand().getStringVector();
      vector<string> localDisks   =
        parset.get("LocalDisks").expand().getStringVector();
      if (!itsHeadNode.empty()) {
        computeNodes.push_back (itsHeadNode);
      }
      for (uint i=0; i<computeNodes.size(); ++i) {
        NodeDesc node;
        node.setName (computeNodes[i]);
        for (uint j=0; j<mountPoints.size(); ++j) {
          node.addFileSys (mountPoints[j], mountPoints[j]);
        }
        // Add node name to local filesys to make it unique.
        for (uint j=0; j<localDisks.size(); ++j) {
          node.addFileSys (computeNodes[i] + ':' + localDisks[j],
                           localDisks[j]);
        }
        addNode (node);
      }
    }
  }

  void ClusterDesc::getSubClusters (const vector<string>& parsetNames)
  {
    for (uint i=0; i<parsetNames.size(); ++i) {
      ClusterDesc cdesc(parsetNames[i]);
      const vector<NodeDesc>& nodes =cdesc.getNodes();
      for (uint j=0; j<nodes.size(); ++j) {
        addNode (nodes[j]);
      }
    }
  }

  void ClusterDesc::write (ostream& os) const
  { 
    os << "ClusterName = " << itsName << endl;
    os << "HeadNode    = " << itsHeadNode << endl;
    os << "NNodes = " << itsNodes.size() << endl;
    for (unsigned i=0; i<itsNodes.size(); ++i) {
      ostringstream prefix;
      prefix << "Node" << i << '.';
      itsNodes[i].write (os, prefix.str());
    }
  }

  void ClusterDesc::addNode (const NodeDesc& node)
  {
    itsNodes.push_back (node);
    add2Map (node);
  }

  void ClusterDesc::add2Map (const NodeDesc& node)
  {
    // The head node is not added as a resource to the map.
    if (node.getName() != itsHeadNode) {
      for (vector<string>::const_iterator iter = node.getFileSys().begin();
           iter != node.getFileSys().end();
           ++iter) {
        vector<string>& vec = itsFS2Nodes[*iter];
        vec.push_back (node.getName());
      }
    }
  }

//   string ClusterDesc::findNode (const string& fileSystem,
// 				const map<string,int>& done) const
//   {
//     map<string,vector<string> >::const_iterator iter =
//                                              itsFS2Nodes.find(fileSystem);
//     if (iter == itsFS2Nodes.end()) {
//       return "";
//     }
//     const vector<string>& nodes = iter->second;
//     for (unsigned i=0; i<nodes.size(); ++i) {
//       if (done.find(nodes[i]) == done.end()) {
// 	return nodes[i];
//       }
//     }
//     return "";
//   }

}} // end namespaces
