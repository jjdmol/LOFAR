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
	initTreeMaintenance ();
    }

    private native void initTreeMaintenance ();

    // Get the node definition of a VC node
    public native jVICnodeDef getNodeDef (int aNodeID);

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int buildTemplateTree (int topNodeID, short aClassif);

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int copyTemplateTree (int aTreeID);

    // Get a single node from the VIC template tree
    public native jOTDBnode getNode (int aTreeID, int aNodeID);

    public native boolean setMomInfo(int aTreeID, int momID, String campaign);

    // Get a number of levels of children.
    public native Vector getItemList (int aTreeID, int topNode, int depth);

    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public native Vector getItemList (int aTreeID, String aNameFragment);

    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    public native int dupNode (int aTreeID, int orgNodeID, short newIndex);

    // Updates the (vector of) OTDBnodes to the database.
    public native boolean saveNode (jOTDBnode aNode);
    public native boolean saveNodeList (Vector aNodeList);

    // Updates the (vector of) OTDBnodes to the database.
    public native boolean deleteNode (jOTDBnode	aNode);
    public native boolean deleteNodeList (Vector aNodeList);

    // Evaluate the constraints from a (sub)tree.
    public native boolean checkTreeConstraints (int aTreeID, int topNode);


    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public native int instanciateTree (int baseTree);

    // Prune an instanciated tree to get loss of depricated values.
    public native boolean pruneTree (int aTreeID, short pruningLevel);

    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    public native boolean deleteTree (int aTreeID);

	// Retrieve the topNode of any tree
    public native jOTDBnode getTopNode (int aTreeID);

    // Set the classification of any tree.
    public native boolean setClassification (int aTreeID, short aClassification);

    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    public native boolean setTreeState (int aTreeID, short aState);

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public native String errorMsg();

    // Get the parameter definition of a node
    public native jOTDBparam getParam (int aTreeID, int aNodeID);

    // Save the parameter definition
    public native boolean saveParam (jOTDBparam aParam);
}
