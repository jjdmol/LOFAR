//#  VICadmin.h: For managing VIC trees.
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

#ifndef LOFAR_OTDB_VICADMIN_H
#define LOFAR_OTDB_VICADMIN_H

// \file VICadmin.h
// For managing VIC trees.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBconnection.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBnode;
class OTDBparam;
class VICnodeDef;


// The VICadmin class is the interface to the VIC trees. The VIC trees 
// differ from the PIC tree in that there is always one operational PIC tree 
// and there can be many operational VIC trees.
// Another difference is that the VIC tree contains references to the PIC tree.
// In other word: the software tree(VIC) is mapped on the hardware tree(PIC).
class VICadmin {
public:
	enum formatType { FtKVList, FtHTML };

	// Connect the VIC interface to an OTDB database.
	explicit VICadmin (OTDBconnection*		aConn);

	~VICadmin();

	// a VIC tree is build up from single components. The definition of a
	// component can loaded from a file with this call
	nodeIDType	loadComponentFile (const string&	filename);
	bool		saveNode	(VICnodeDef&	aNode);
	bool		saveParam	(OTDBparam&		aParam);
	bool		deleteNode	(VICnodeDef&	aNode);
	VICnodeDef	getNode		(nodeIDType		aNodeID);
	VICnodeDef	getNode		(const string&		aNameFragment,
							 uint32				aVersion = 0,
							 treeClassifType	aClassification = 0);
	vector<VICnodeDef> getTopComponentList (const string&	name = "%");

	// From a component tree a (folded) tree can be constructed. In a folded
	// tree only the structure of the tree is created, there is no replication
	// of nodes on the same level.
	treeIDType buildFoldedTree (treeIDType			baseTree,
							    nodeIDType			topNodeID,
							    treeClassifType		aClassif);

	// Get a single node from the VIC template tree
	OTDBnode getNode (treeIDType	aTreeID,
					  nodeIDType	aNodeID);

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

	// Updates the (vector of) OTDBnodes to the database.
	bool	saveNode    (OTDBnode&			aNode);
	bool	saveNodeList(vector<OTDBnode>&	aNodeList);

	// Updates the (vector of) OTDBnodes to the database.
	bool	deleteNode    (OTDBnode&			aNode);
	bool	deleteNodeList(vector<OTDBnode>&	aNodeList);

	// From a foldedTree a fully instanciated tree can be build.
	treeIDType instanciateTree (treeIDType	baseTree);

	// Set the classification of the tree.
	bool	setClassification(treeIDType		aTreeID,
							  treeClassifType	aClassification);

	// Set the type/stage of the tree. When changing the type of a tree all
	// constraints/validations for the current type must be fulfilled.
	// When errors occur these can be retrieved with the errorMsg function.
	bool	setTreeType(treeIDType		aTreeID,
						treeType		aType);

	// Export a VIC (sub)tree to a file. The user may choose in which format
	// the tree is exported: HTML, KeyValue List.
	bool	exportTree (treeIDType			aTreeID,
						nodeIDType			topItem,
						const string&		filename,
						const formatType	outputFormat,
						bool				folded = false);

	// Whenever an error occurs in one the OTDB functions the message can
	// be retrieved with this function.
	string	errorMsg();

private:
	// Default construction and copying is not allowed
	VICadmin();
	VICadmin(const VICadmin&	that);
	VICadmin& operator=(const VICadmin& that);

	//# --- Datamembers ---
	OTDBconnection*		itsConn;
	string				itsError;
};

//# --- Inline functions ---
inline string VICadmin::errorMsg() {
	return (itsError);
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
