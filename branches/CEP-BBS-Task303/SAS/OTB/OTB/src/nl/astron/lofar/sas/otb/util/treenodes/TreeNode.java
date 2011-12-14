/*
 * TreeNode.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

package nl.astron.lofar.sas.otb.util.treenodes;

import javax.swing.tree.DefaultMutableTreeNode;
import nl.astron.lofar.sas.otb.util.treemanagers.ITreeManager;
import org.apache.log4j.Logger;

/**
 * Base TreeNode Interface
 *
 * @created 26-01-2006, 14:56
 *
 * @author Blaakmeer
 *
 * @version $Id$
 *
 * @updated
 */
public class TreeNode extends DefaultMutableTreeNode {
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(TreeNode.class);
    
    public boolean   areChildrenDefined = false;
    private String name;
    private ITreeManager treeManager;
    
    /**
     * default constructor - creates an empty node.
     */
    public TreeNode(ITreeManager treeManager) {
        name="Default Root TreeNode";
        this.treeManager = (ITreeManager)treeManager;
    }
    
    /**
     * constructor - creates a TreeNode using the name provided
     */
    public TreeNode(ITreeManager treeManager, String name) {
        this.name=name;
        this.treeManager = (ITreeManager)treeManager;
    }
    
    /**
     * constructor - creates a TreeNode using the object provided
     */
    public TreeNode(ITreeManager treeManager, Object userObject) {
        this.userObject=userObject;
        this.treeManager = (ITreeManager)treeManager;
    }
    /**
     * constructor - creates a TreeNode using the object and name provided
     */
    public TreeNode(ITreeManager treeManager, Object userObject, String name) {
        this.userObject=userObject;
        this.name=name;
        this.treeManager = (ITreeManager)treeManager;
    }
    
    @Override
    public boolean isLeaf() {
        return treeManager.isNodeLeaf(this);
    }

    /**
     * Called by the JTree
     * Adds childs if they are not defined and returns the number of childs
     */
    @Override
    public int getChildCount() {
        logger.trace("Entry - getChildCount("+toString()+")");
        
        if (!areChildrenDefined)
            ((ITreeManager)treeManager).defineChildsForNode(this);
        int childCount = super.getChildCount();
        
        logger.trace("Exit - getChildCount("+toString()+"): " + childCount);
        return(childCount);
    }
    
    /**
     * Called by the JTree
     * @return the human readable name of the node
     */
    @Override
    public String toString() {
        return ((ITreeManager)treeManager).getNameForNode(this);
    }
    
    public String getName(){
        return name;
    }
    
    public void setName(String name){
        this.name=name;
    }
    

}
