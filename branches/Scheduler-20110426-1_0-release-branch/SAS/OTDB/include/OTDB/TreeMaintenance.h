//#  TreeMaintenance.h: Maintenance on complete trees.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_OTDB_TREEMAINTENANCE_H
#define LOFAR_OTDB_TREEMAINTENANCE_H

// \file
// Maintenance on complete trees.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconnection.h>
#include <Common/lofar_vector.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBnode;
class OTDBparam;
class VICnodeDef;


// The TreeMaintenance class is the interface to manipulate the VIC and
// PIC trees in the OTDB.
class TreeMaintenance
{
public:
	enum formatType	{ FtKVList, FtHTML };

	// Connect the TreeMaintenance interface to the OTDB database.
	explicit TreeMaintenance (OTDBconnection*	aConn);
	~TreeMaintenance();


	//# --- PIC maintenance ---
	// Once in a while a new PIC tree will be loaded from PVSS which manages
	// the master PIC. The master PIC will be in a exported ASCII file, with
	// loadMasterFile this file can be added.
	// Returns 0 on failure, otherwise the ID of the new tree is returned.
	treeIDType	loadMasterFile (const string&	filename);


	//# --- VIC maintenance : Components ---
	// A VIC tree is build up from single components. The definitions of the
	// components can loaded from a file with this call
	nodeIDType	loadComponentFile (const string&	filename,
								   const string&	forcedVersionNr = "",
								   const string&	forcedQualifier = "");

	// Find the top-components in the components table.
	vector<VICnodeDef>	getComponentList (const string&	name = "%", bool topOnly = false);

	// Get the node definition of a VC node
	VICnodeDef			getComponentNode  (nodeIDType	aNodeID);
	
	// Get parameterlist of the component
	vector<OTDBparam>	getComponentParams(nodeIDType	aNodeID);

	// Save new node or update the limits and description fields of the node.
	bool	saveComponentNode	(VICnodeDef&	aNode);

	// test if component is a top-component
	bool	isTopComponent		(nodeIDType		aNodeID);

	// delete a component node
	bool	deleteComponentNode	(nodeIDType		aNodeID);

	// Return the full component name including the versionnumber.
	string	getFullComponentName (VICnodeDef&	aNode);

	//# --- VIC maintenance : Templates ---
	// From a component tree a template tree can be constructed. In a template
	// tree only the structure of the tree is created, there is no replication
	// of nodes on the same level.
	// Returns 0 on failure, otherwise the ID of the new tree is returned.
	// ABOUT TO BECOME OBSOLETE WHEN OTB IS READY
	treeIDType buildTemplateTree (nodeIDType	topNodeID,
								  classifType	aClassif);

	// Create a new OTDBtree record for an Template tree in the database and return its
	// treeID.
	treeIDType newTemplateTree();

	// Make a copy of an existing template tree.
	// Returns 0 on failure, otherwise the ID of the new tree is returned.
	treeIDType	copyTemplateTree(treeIDType		aTreeID);

	// Assign a name to a template tree making it a default-template.
	// When an empty name is assigned the default-template becomes a normal template again.
	bool	assignTemplateName (treeIDType		treeID,
							    const string&	name);

	// Get a single node from any tree
	OTDBnode getNode (treeIDType	aTreeID,
					  nodeIDType	aNodeID);

	// Get the parameter definition of a node
	OTDBparam	getParam	(treeIDType		aTreeID,
							 nodeIDType		aParamID);

	// Get the parameter definition of a node. Will recursively follow the
	// references in the limits field.
	OTDBparam	getParam	(const OTDBnode&		aNode);

	// Save the parameter definition
	bool	saveParam	(OTDBparam&		aParam);

	// Get a number of levels of children.
	vector<OTDBnode> getItemList (treeIDType	aTreeID,
								  nodeIDType	topNode,
								  uint32		depth);
	// Get a list of nodes based on a namefragment. Use '%' as wildchar.
	vector<OTDBnode> getItemList (treeIDType	aTreeID,
								  const string&	aNameFragment);

	// Duplicates the given node (and its parameters and children)
	// in the template database. The duplicate gets the new index.
	nodeIDType	dupNode (treeIDType		aTreeID,
						 nodeIDType		orgNodeID,
					 	 int16			newIndex);

	// Adds the given VIC Component under the given parent of a 
	// template tree.
	nodeIDType	addComponent (nodeIDType	compID,
							  treeIDType	treeID,
							  nodeIDType	parentID,
							  const string&	newName="");

	// Updates the (vector of) OTDBnodes to the database.
	bool	saveNode    (OTDBnode&			aNode);
	bool	saveNodeList(vector<OTDBnode>&	aNodeList);

	// Updates the (vector of) OTDBnodes to the database.
	bool	deleteNode    (OTDBnode&			aNode);
	bool	deleteNodeList(vector<OTDBnode>&	aNodeList);

	// Evaluate the constraints from a (sub)tree.
	// NOT YET IMPLEMENTED
	bool	checkTreeConstraints(treeIDType		aTreeID,
								 nodeIDType		topNode = 0);


	//# --- VIC maintenance : Hierarchical trees ---
	// From a template tree a fully instanciated tree can be build.
	// Returns 0 on failure, otherwise the ID of the new tree is returned.
	treeIDType instanciateTree (treeIDType	baseTree);

	// Prune an instanciated tree to get loss of depricated values.
	bool	pruneTree(treeIDType	aTreeID, int16	pruningLevel);

	// Export a VIC (sub)tree to a file. The user may choose in which format
	// the tree is exported: HTML, KeyValue List.
	bool	exportTree (treeIDType			aTreeID,
						nodeIDType			topItem,
						const string&		filename,
						const formatType	outputFormat = FtKVList,
						bool				folded = false);


	//# --- Finally some general tree maintenance ---
	// Delete a tree (of any kind) from the database.
	bool		deleteTree(treeIDType		aTreeID);

	// Retrieve the topNode of any tree
	OTDBnode getTopNode (treeIDType		aTreeID);

	// save modified OTDBtree information
	bool	setMomInfo (treeIDType		aTreeID,
						treeIDType		aMomID,
						string			aCampaign);

	// Set the classification of any tree.
	bool	setClassification(treeIDType	aTreeID,
							  classifType	aClassification);

	// Set the state of any tree. When changing the state of a tree all
	// constraints/validations for the current type must be fulfilled.
	// When errors occur these can be retrieved with the errorMsg function.
	bool	setTreeState(treeIDType		aTreeID,
						 treeState		aState);

	// Update the description of a tree.
	bool	setDescription(treeIDType	aTreeID,
						   string		aDescription);

	// Set the scheduling times of the tree
	bool	setSchedule(treeIDType		aTreeID,
				        const ptime&	aStartTime,
				        const ptime& 	aStopTime);

	// Whenever an error occurs in one the OTDB functions the message can
	// be retrieved with this function.
	inline string	errorMsg() const;

private:
	// Default construnction is not possible
	TreeMaintenance();
	// Copying is not necessary
	TreeMaintenance(const TreeMaintenance&	that);
	TreeMaintenance& operator=(const TreeMaintenance& that);

	// Helper routine for loading the component file
	VICnodeDef	getNodeDef	(const string&	aNameFragment,
							 uint32			aVersion = 0,
							 classifType	aClassification = 0);
	// Helper routines for getItemList
	vector<OTDBnode> getPICitemList(treeIDType, nodeIDType, uint32);
	vector<OTDBnode> getVTitemList (treeIDType, nodeIDType, uint32);
	vector<OTDBnode> getVHitemList (treeIDType, nodeIDType, uint32);

	//# --- Datamembers ---
	OTDBconnection*		itsConn;
	string				itsError;
};

//# --- Inline functions ---

//#
//# errorMsg()
//#
inline string TreeMaintenance::errorMsg() const
{
	return (itsError);
}

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
