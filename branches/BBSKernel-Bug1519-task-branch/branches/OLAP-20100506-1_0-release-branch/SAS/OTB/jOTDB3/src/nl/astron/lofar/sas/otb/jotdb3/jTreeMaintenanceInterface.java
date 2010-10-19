//#  jTreeMaintenanceInterface.java: The RMI interface to the OTDB database.
//#
//#  Copyright (C) 2002-2007
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
  
package nl.astron.lofar.sas.otb.jotdb3;


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Vector;

public interface jTreeMaintenanceInterface extends Remote 
{
   // Constants
   public static final String SERVICENAME = "jTreeMaintenance";


    //# --- PIC maintenance ---
    // Once in a while a new PIC tree will be loaded from PVSS which manages
    // the master PIC. The master PIC will be in a exported ASCII file, with
    // loadMasterFile this file can be added.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int loadMasterFile (String  filename) throws RemoteException;

    //# --- VIC maintenance : Components ---
    // A VIC tree is build up from single components. The definitions of the
    // components can loaded from a file with this call
    public  int loadComponentFile (String filename,String forcedVersionNr, String forcedQualifier) throws RemoteException;

    // Find the top-components in the components table.
    public Vector<jVICnodeDef> getComponentList (String name , boolean topOnly) throws RemoteException;

    // Get the node definition of a VC node
    public jVICnodeDef getComponentNode(int aNodeID) throws RemoteException;

    // Get parameterlist of the component
    public Vector<jOTDBparam> getComponentParams(int aNodeID) throws RemoteException;

    // Save new node or update the limits and description fields of the node
    public boolean  saveComponentNode(jVICnodeDef  aNode) throws RemoteException;

    // test if component is a top-component
    public boolean isTopComponent(int aNodeID) throws RemoteException;

    // delete component node
    public boolean deleteComponentNode(int aNodeID) throws RemoteException;
    
    // get full component name
    public String getFullComponentName(jVICnodeDef aNode) throws RemoteException;

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int buildTemplateTree (int topNodeID, short aClassif) throws RemoteException;

    // Create a new OTDBtree record for an Template tree in the database
    // and return its treeID.
    public int newTemplateTree() throws RemoteException;

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int copyTemplateTree (int aTreeID) throws RemoteException;


    // Get a single node from the VIC template tree
    public jOTDBnode getNode (int aTreeID, int aNodeID) throws RemoteException;

    // Get the parameter definition of a node
    public jOTDBparam getParam(int aTreeID,int aParamID) throws RemoteException;

    // Get the parameter definition of a node. Will recursively follow the
    // references in the limits field.
    public jOTDBparam getParam(jOTDBnode aNode) throws RemoteException;
    
// Save the parameter definition
    public boolean saveParam(jOTDBparam aParam) throws RemoteException;

    // Get a number of levels of children.
    public Vector<jOTDBnode> getItemList (int aTreeID, int topNode, int depth) throws RemoteException;

    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public Vector<jOTDBnode> getItemList (int aTreeID, String aNameFragment) throws RemoteException;

    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    public int dupNode (int aTreeID, int orgNodeID, short newIndex) throws RemoteException;

    // Adds the given VIC Component under the given parent of a
    // template tree.
    public int addComponent (int compID,int treeID,int parentID, String newName) throws RemoteException;

    // Updates the (vector of) OTDBnodes to the database.
    public boolean saveNode (jOTDBnode aNode) throws RemoteException;
    public boolean saveNodeList (Vector aNodeList) throws RemoteException;

    // Updates the (vector of) OTDBnodes to the database.
    public boolean deleteNode (jOTDBnode        aNode) throws RemoteException;
    public boolean deleteNodeList (Vector aNodeList) throws RemoteException;

    // Evaluate the constraints from a (sub)tree.
    public boolean checkTreeConstraints (int aTreeID, int topNode) throws RemoteException;

    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int instanciateTree (int baseTree) throws RemoteException;

    // Prune an instanciated tree to get loss of depricated values.
    public boolean pruneTree (int aTreeID, short pruningLevel) throws RemoteException;

    // Export a VIC (sub)tree to a file. The user may choose in which format
    // the tree is exported: HTML, KeyValue List.
    public boolean exportTree (int aTreeID,int topItem,String filename,int outputFormat,boolean folded) throws RemoteException;

    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    public boolean deleteTree (int aTreeID) throws RemoteException;

    // Retrieve the topNode of any tree
    public jOTDBnode getTopNode (int aTreeID) throws RemoteException;

    public boolean setMomInfo(int aTreeID, int momID, String campaign) throws RemoteException;


    // Set the classification of any tree.
    public boolean setClassification (int aTreeID, short aClassification) throws RemoteException;

    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    public boolean setTreeState (int aTreeID, short aState) throws RemoteException;

    // Update the description of a tree.
    public boolean setDescription(int  aTreeID,String aDescription) throws RemoteException;

    // Set the scheduling times of the tree
    public boolean setSchedule(int aTreeID, String aStartTime,String aStopTime) throws RemoteException;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public String errorMsg() throws RemoteException;


}
