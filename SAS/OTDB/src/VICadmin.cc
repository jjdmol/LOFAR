//#  VICadmin.cc: For managing VIC trees.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<OTDB/VICadmin.h>

namespace LOFAR {
  namespace OTDB {

//
// VICadmin()
//
VICadmin::VICadmin (OTDBconnection*		aConn) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}

//
// ~VICadmin()
//
VICadmin::~VICadmin()
{
	if (itsConn) {
		delete itsConn;
	}
}

//
// createNewTree(): treeID
//
// Before any components can be loaded into a component tree a new
// (empty) tree must be created.
treeIDType	VICadmin::createNewTree ()
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	//TODO: ...

	return (0);
}

//
// loadComponentFile(treeID, filename): nodeID
//
// a VIC tree is build up from single components. The definition of a
// component can loaded from a file with this call
nodeIDType	VICadmin::loadComponentFile (treeIDType			aTreeID,
						   			 const string&	filename)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	//TODO: ...

	return (0);

}

//
// buildFoldedTree(treeID, topNode): treeID
//
// From a component tree a (folded) tree can be constructed. In a folded
// tree only the structure of the tree is created, there is no replication
// of nodes on the same level.
treeIDType	VICadmin::buildFoldedTree (treeIDType		baseTree)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	//TODO: ...

	return (0);
}

//
// instanciateTree(treeID): treeID
//
// From a foldedTree a fully instanciated tree can be build.
treeIDType	VICadmin::instanciateTree (treeIDType		baseTree)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}

//
// setClassification(treeID, classification): bool
//
// Set the classification of the tree.
bool	VICadmin::setClassification(treeIDType			aTreeID,
								    treeClassifType		aClassification)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}

//
// setTreeType(treetype): bool
//
// Set the type/stage of the tree. When changing the type of a tree all
// constraints/validations for the current type must be fulfilled.
// When errors occur these can be retrieved with the errorMsg function.
bool	VICadmin::setTreeType(treeIDType		aTreeID,
							  treeType			aType)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}

//
// exportTree(treeID, nodeID, filename, formattype, folded): bool
//
// Export a VIC (sub)tree to a file. The user may choose in which format
// the tree is exported: HTML, KeyValue List.
bool	VICadmin::exportTree (treeIDType		aTreeID,
							  nodeIDType		topItem,
							  const string&		filename,
							  const formatType	outputFormat,
							  bool				folded)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}



  } // namespace OTDB
} // namespace LOFAR
