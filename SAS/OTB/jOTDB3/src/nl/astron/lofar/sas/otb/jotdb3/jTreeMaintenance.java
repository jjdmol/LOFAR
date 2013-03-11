//#  jOTDBtreeMaintenance.java: Maintenance on complete trees.
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

import java.rmi.RemoteException;
import java.util.Vector;

public class jTreeMaintenance implements jTreeMaintenanceInterface
{
    private String itsName = "";

    public jTreeMaintenance (String ext)
    {
        try {
            itsName=ext;
            initTreeMaintenance ();
        } catch (Exception ex) {
            System.out.println("Error during jTreeMaintenance init : " +ex);
        }
    }

    private native void initTreeMaintenance () throws RemoteException;

    //# --- PIC maintenance ---
    // Once in a while a new PIC tree will be loaded from PVSS which manages
    // the master PIC. The master PIC will be in a exported ASCII file, with
    // loadMasterFile this file can be added.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    @Override
    public native int loadMasterFile (String  filename) throws RemoteException;
    
    //# --- VIC maintenance : Components ---
    // A VIC tree is build up from single components. The definitions of the
    // components can loaded from a file with this call
    @Override
    public native int loadComponentFile (String filename,String forcedVersionNr,String forcedQualifier) throws RemoteException;
    @Override
    public native int loadComponentFile (String filename,String forcedVersionNr) throws RemoteException;
    @Override
    public native int loadComponentFile (String filename) throws RemoteException;

    // Find the top-components in the components table.
    @Override
    public native Vector<jVICnodeDef> getComponentList (String name , boolean topOnly) throws RemoteException;
    @Override
    public native Vector<jVICnodeDef> getComponentList (String name) throws RemoteException;
    @Override
    public native Vector<jVICnodeDef> getComponentList () throws RemoteException;
    
    // Get the node definition of a VC node
    @Override
    public native jVICnodeDef getComponentNode(int aNodeID) throws RemoteException;

    // Get parameterlist of the component
    @Override
    public native Vector<jOTDBparam> getComponentParams(int aNodeID) throws RemoteException;

    // Save new node or update the limits and description fields of the node
    @Override
    public native boolean saveComponentNode(jVICnodeDef  aNode) throws RemoteException;

    // test if component is a top-component
    @Override
    public native boolean isTopComponent(int  aNodeID) throws RemoteException;

    // delete a component node
    @Override
    public native boolean deleteComponentNode(int  aNodeID) throws RemoteException;
    
    // get full component name (including version in name)
    @Override
    public native String getFullComponentName(jVICnodeDef aNode) throws RemoteException;

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    // ABOUT TO BECOME OBSOLETE WHEN OTB IS READY
    @Override
    public native int buildTemplateTree (int topNodeID, short aClassif) throws RemoteException;


    // Create a new OTDBtree record for an Template tree in the database 
    // and return its treeID.
    @Override
    public native int newTemplateTree() throws RemoteException;

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    @Override
    public native int copyTemplateTree (int aTreeID) throws RemoteException;

    // Assign a name to a template tree making it a default-template
    // when an empty name is assigned the default-template becomes a normal template again
    @Override
    public native boolean assignTemplateName(int aTreeID, String aName) throws RemoteException;

    // Assign processType and others to a (default) template.
    // Doubles are not allowed by the database.
    @Override
    public native boolean assignProcessType(int aTreeID,String aProcessType, String aProcessSubtype, String aStrategy) throws RemoteException;


    // Get a single node from the VIC template tree
    @Override
    public native jOTDBnode getNode (int aTreeID, int aNodeID) throws RemoteException;

    // Get the parameter definition of a node
    @Override
    public native jOTDBparam getParam(int aTreeID,int aParamID) throws RemoteException;
    
    // Get the parameter definition of a node. Will recursively follow the
    // references in the limits field.
    @Override
    public native jOTDBparam getParam(jOTDBnode aNode) throws RemoteException;

    // Save the parameter definition
    @Override
    public native boolean saveParam(jOTDBparam aParam) throws RemoteException;

    // Get a number of levels of children.
    @Override
    public native Vector<jOTDBnode> getItemList (int aTreeID, int topNode, int depth) throws RemoteException;

    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public native Vector<jOTDBnode> getItemList (int aTreeID, String aNameFragment) throws RemoteException;

    // Get a list of nodes based on a namefragment. aNameFragment can be a regex.
    public native Vector<jOTDBnode> getItemList (int aTreeID, String aNameFragment, boolean isRegex) throws RemoteException;

    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    @Override
    public native int dupNode (int aTreeID, int orgNodeID, short newIndex) throws RemoteException;

    // Adds the given VIC Component under the given parent of a
    // template tree.
    @Override
    public native int addComponent (int compID,int treeID,int parentID, String newName) throws RemoteException;
    @Override
    public native int addComponent (int compID,int treeID,int parentID) throws RemoteException;
 
    // Updates the (vector of) OTDBnodes to the database.
    @Override
    public native boolean saveNode (jOTDBnode aNode) throws RemoteException;
    @Override
    public native boolean saveNodeList (Vector aNodeList) throws RemoteException;

    // Updates the (vector of) OTDBnodes to the database.
    @Override
    public native boolean deleteNode (jOTDBnode	aNode) throws RemoteException;
    @Override
    public native boolean deleteNodeList (Vector aNodeList) throws RemoteException;

    // Evaluate the constraints from a (sub)tree.
    @Override
    public native boolean checkTreeConstraints (int aTreeID, int topNode) throws RemoteException;
    @Override
    public native boolean checkTreeConstraints (int aTreeID) throws RemoteException;


    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    @Override
    public native int instanciateTree (int baseTree) throws RemoteException;

    // Prune an instanciated tree to get loss of depricated values.
    @Override
    public native boolean pruneTree (int aTreeID, short pruningLevel) throws RemoteException;

    // Export a VIC (sub)tree to a file. The user may choose in which format
    // the tree is exported: HTML, KeyValue List.
    @Override
    public native boolean exportTree (int aTreeID,int topItem,String filename) throws RemoteException;

    // Export a VIC (sub)tree with reported metadata to a file.
    @Override
    public native boolean exportResultTree (int aTreeID,int topItem,String filename) throws RemoteException;

    // Export all reported metadata from the given VIC tree
    @Override
    public native boolean exportMetadata (int	aTreeID,String filename) throws RemoteException;

    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    @Override
    public native boolean deleteTree (int aTreeID) throws RemoteException;

    // Retrieve the topNode of any tree
    @Override
    public native jOTDBnode getTopNode (int aTreeID) throws RemoteException;

    // save modified OTDBtree information
    @Override
    public native boolean setMomInfo(int aTreeID, int momID, int aGroupID, String campaign) throws RemoteException;

    // for backwards compatibility
    @Override
    public boolean setMomInfo(int aTreeID, int momID, String campaign) throws RemoteException {
        return setMomInfo(aTreeID,momID,0,campaign);
    }

    // Set the classification of any tree.
    @Override
    public native boolean setClassification (int aTreeID, short aClassification) throws RemoteException;

    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    @Override
    public native boolean setTreeState (int aTreeID, short aState) throws RemoteException;


    // Update the description of a tree.
    @Override
    public native boolean setDescription(int  aTreeID,String aDescription) throws RemoteException;

    // Set the scheduling times of the tree
    @Override
    public native boolean setSchedule(int aTreeID, String aStartTime,String aStopTime) throws RemoteException;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    @Override
    public native String errorMsg() throws RemoteException;


}
