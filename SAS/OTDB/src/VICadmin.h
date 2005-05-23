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
//# class ...;


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

	// Before any components can be loaded into a component tree a new
	// (empty) tree must be created.
	treeID	createNewTree ();

	// a VIC tree is build up from single components. The definition of a
	// component can loaded from a file with this call
	nodeID	loadComponentFile (treeID			aTreeID,
							   const string&	filename);

	// From a component tree a (folded) tree can be constructed. In a folded
	// tree only the structure of the tree is created, there is no replication
	// of nodes on the same level.
	treeID	buildFoldedTree (treeID		baseTree);

	// From a foldedTree a fully instanciated tree can be build.
	treeID	instanciateTree (treeID		baseTree);

	// Set the classification of the tree.
	bool	setClassification(treeID		aTreeID,
							  treeClassif	aClassification);

	// Set the type/stage of the tree. When changing the type of a tree all
	// constraints/validations for the current type must be fulfilled.
	// When errors occur these can be retrieved with the errorMsg function.
	bool	setTreeType(treeID			aTreeID,
						treeType		aType);

	// Export a VIC (sub)tree to a file. The user may choose in which format
	// the tree is exported: HTML, KeyValue List.
	bool	exportTree (treeID				aTreeID,
						nodeID				topItem,
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
