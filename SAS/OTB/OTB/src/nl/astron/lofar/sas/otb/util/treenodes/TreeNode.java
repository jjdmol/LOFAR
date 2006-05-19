/*
 * TreeNode.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util.treenodes;

import javax.swing.tree.DefaultMutableTreeNode;
import nl.astron.lofar.sas.otb.util.treemanagers.ITreeManager;
import org.apache.log4j.Logger;

/**
 * Base TreeNode Interface
 *
 * @author pompert
 * @version $Id$
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
    
    public boolean isLeaf() {
        return treeManager.isNodeLeaf(this);
    }

    /**
     * Called by the JTree
     * Adds childs if they are not defined and returns the number of childs
     */
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
