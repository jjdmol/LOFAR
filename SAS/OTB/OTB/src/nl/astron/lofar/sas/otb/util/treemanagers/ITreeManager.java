/*
 * ITreeManager.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util.treemanagers;

import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;

/**
 * Base TreeManager Interface
 *
 * @author pompert
 * @version $Id$
 */
public interface ITreeManager {
    
    public String getNameForNode(TreeNode aNode);
    
    public boolean isNodeLeaf(TreeNode aNode);
    
    public void defineChildsForNode(TreeNode aNode);
    
    public TreeNode getRootNode(Object arguments);
}
