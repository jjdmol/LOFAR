//#  jTreeMaintenanceInterface.java: The RMI interface to the OTDB database.
//#
//#  Copyright (C) 2002-2005
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
  
package jOTDB;

import jOTDB.jOTDBnode;
import jOTDB.jVICnodeDef;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Vector;

public interface jTreeMaintenanceInterface extends Remote 
{
   // Constants
   public static final String SERVICENAME = "jTreeMaintenance";

    // Get the node definition of a VC node
    public jVICnodeDef getNode (int aNodeID) throws RemoteException;

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int buildTemplateTree (int topNodeID, short aClassif) throws RemoteException;

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int copyTemplateTree (int aTreeID) throws RemoteException;

    // Get a single node from the VIC template tree
    public jOTDBnode getNode (int aTreeID, int aNodeID) throws RemoteException;

    // Get a number of levels of children.
    public Vector getItemList (int aTreeID, int topNode, int depth) throws RemoteException;

    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public Vector getItemList (int aTreeID, String aNameFragment) throws RemoteException;

    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    public int dupNode (int aTreeID, int orgNodeID, short newIndex) throws RemoteException;

    // Updates the (vector of) OTDBnodes to the database.
    public boolean saveNode (jOTDBnode aNode) throws RemoteException;
    public boolean saveNodeList (Vector aNodeList) throws RemoteException;

    // Updates the (vector of) OTDBnodes to the database.
    public boolean deleteNode (jOTDBnode	aNode) throws RemoteException;
    public boolean deleteNodeList (Vector aNodeList) throws RemoteException;

    // Evaluate the constraints from a (sub)tree.
    public boolean checkTreeConstraints (int aTreeID, int topNode) throws RemoteException;


    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int instanciateTree (int baseTree) throws RemoteException;

    // Prune an instanciated tree to get loss of depricated values.
    public boolean pruneTree (int aTreeID, short pruningLevel) throws RemoteException;

    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    public boolean deleteTree (int aTreeID) throws RemoteException;

	// Retrieve the topNode of any tree
    public jOTDBnode getTopNode (int aTreeID) throws RemoteException;

    // Set the classification of any tree.
    public boolean setClassification (int aTreeID, short aClassification) throws RemoteException;

    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    public boolean setTreeState (int aTreeID, short aState) throws RemoteException;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public String errorMsg() throws RemoteException;
}
