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

package nl.astron.lofar.sas.otb.jotdb2;

import java.util.Vector;

public class jTreeMaintenance
{

    public jTreeMaintenance ()
    {
        try {
            initTreeMaintenance ();
        } catch (Exception ex) {
            System.out.println("Error during jTreeMaintenance init : " +ex);
        }
    }

    private native void initTreeMaintenance () throws Exception;

    //# --- PIC maintenance ---
    // Once in a while a new PIC tree will be loaded from PVSS which manages
    // the master PIC. The master PIC will be in a exported ASCII file, with
    // loadMasterFile this file can be added.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int loadMasterFile (String  filename) throws Exception;
    
    //# --- VIC maintenance : Components ---
    // A VIC tree is build up from single components. The definitions of the
    // components can loaded from a file with this call
    public native int loadComponentFile (String filename,String forcedVersionNr,String forcedQualifier) throws Exception;

    // Find the top-components in the components table.
    public native Vector<jVICnodeDef> getComponentList (String name , boolean topOnly) throws Exception;
    
    // Get the node definition of a VC node
    public native jVICnodeDef getComponentNode(int aNodeID) throws Exception;

    // Get parameterlist of the component
    public native Vector<jOTDBparam> getComponentParams(int aNodeID) throws Exception;

    // Save new node or update the limits and description fields of the node
    public native boolean saveComponentNode(jVICnodeDef  aNode) throws Exception;

    // test if component is a top-component
    public native boolean isTopComponent(int  aNodeID) throws Exception;

    // delete a component node
    public native boolean deleteComponentNode(int  aNodeID) throws Exception;
    
    // get full component name (including version in name)
    public native String getFullComponentName(jVICnodeDef aNode) throws Exception;

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    // ABOUT TO BECOME OBSOLETE WHEN OTB IS READY
    public native int buildTemplateTree (int topNodeID, short aClassif) throws Exception;


    // Create a new OTDBtree record for an Template tree in the database 
    // and return its treeID.
    public native int newTemplateTree() throws Exception;

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int copyTemplateTree (int aTreeID) throws Exception;

    // Get a single node from the VIC template tree
    public native jOTDBnode getNode (int aTreeID, int aNodeID) throws Exception;

    // Get the parameter definition of a node
    public native jOTDBparam getParam(int aTreeID,int aParamID) throws Exception;
    
    // Get the parameter definition of a node. Will recursively follow the
    // references in the limits field.
    public native jOTDBparam getParam(jOTDBnode aNode) throws Exception;

    // Save the parameter definition
    public native boolean saveParam(jOTDBparam aParam) throws Exception;

    // Get a number of levels of children.
    public native Vector<jOTDBnode> getItemList (int aTreeID, int topNode, int depth) throws Exception;

    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public native Vector<jOTDBnode> getItemList (int aTreeID, String aNameFragment) throws Exception;

    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    public native int dupNode (int aTreeID, int orgNodeID, short newIndex) throws Exception;

    // Adds the given VIC Component under the given parent of a
    // template tree.
    public native int addComponent (int compID,int treeID,int parentID, String newName) throws Exception;
 
    // Updates the (vector of) OTDBnodes to the database.
    public native boolean saveNode (jOTDBnode aNode) throws Exception;
    public native boolean saveNodeList (Vector aNodeList) throws Exception;

    // Updates the (vector of) OTDBnodes to the database.
    public native boolean deleteNode (jOTDBnode	aNode) throws Exception;
    public native boolean deleteNodeList (Vector aNodeList) throws Exception;

    // Evaluate the constraints from a (sub)tree.
    public native boolean checkTreeConstraints (int aTreeID, int topNode) throws Exception;


    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int instanciateTree (int baseTree) throws Exception;

    // Prune an instanciated tree to get loss of depricated values.
    public native boolean pruneTree (int aTreeID, short pruningLevel) throws Exception;

    // Export a VIC (sub)tree to a file. The user may choose in which format
    // the tree is exported: HTML, KeyValue List.
    public native boolean exportTree (int aTreeID,int topItem,String filename,
                                      int outputFormat,boolean folded) throws Exception;


    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    public native boolean deleteTree (int aTreeID) throws Exception;

    // Retrieve the topNode of any tree
    public native jOTDBnode getTopNode (int aTreeID) throws Exception;

    // save modified OTDBtree information
    public native boolean setMomInfo(int aTreeID, int momID, String campaign) throws Exception;
    
    // Set the classification of any tree.
    public native boolean setClassification (int aTreeID, short aClassification) throws Exception;

    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    public native boolean setTreeState (int aTreeID, short aState) throws Exception;


    // Update the description of a tree.
    public native boolean setDescription(int  aTreeID,String aDescription) throws Exception;

    // Set the scheduling times of the tree
    public native boolean setSchedule(int aTreeID, String aStartTime,String aStopTime) throws Exception;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public native String errorMsg() throws Exception;


}
