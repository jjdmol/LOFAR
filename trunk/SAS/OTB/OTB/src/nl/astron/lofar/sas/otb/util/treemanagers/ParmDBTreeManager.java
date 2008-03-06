/*
 * ParmDBTreeManager.java
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

package nl.astron.lofar.sas.otb.util.treemanagers;

import java.util.Enumeration;
import java.util.Vector;
import javax.swing.event.TreeModelEvent;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base ParmDBTreeManager Class
 *
 * @created 25-0-2006, 13:56
 *
 * @author pompert
 *
 * @version $Id$
 *
 * @updated
 */
public class ParmDBTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(ParmDBTreeManager.class);
    private static final String PARMDB_TREENODE_SEPARATOR_CHAR = ":";
    private static ParmDBTreeManager instance;
   
    /**
     * default constructor, protected by a singleton pattern
     */
    private ParmDBTreeManager(UserAccount anAccount) {
        super(anAccount);
    }
    
    public static ParmDBTreeManager getInstance(UserAccount anAccount){
        if(instance==null){
            instance = new ParmDBTreeManager(anAccount);
        }
        return instance;
    }
    
    public String getNameForNode(TreeNode aNode){
        //logger.trace("Parm DB node : "+((jOTDBnode)aNode.getUserObject()).name);
        String name = ((jParmDBnode)aNode.getUserObject()).getName();
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            leaf = ((jParmDBnode)aNode.getUserObject()).isLeaf();
        }
        return leaf;
    }
    
    public void defineChildsForNode(TreeNode aNode) {
        logger.trace("Entry - TreeManager jParmDBnode-defineChildNodes("+aNode.getName()+" ("+((jParmDBnode)aNode.getUserObject()).getNodeID()+"))");
        try {
            // You must set the flag before defining children if you
            // use "add" for the new children. Otherwise you get an infinite
            // recursive loop, since add results in a call to getChildCount.
            // However, you could use "insert" in such a case.
            aNode.areChildrenDefined = true;

            jParmDBnode userNode = (jParmDBnode) aNode.getUserObject();
            SharedVars.getJParmFacade().setParmFacadeDB(userNode.getParmDBLocation());
            logger.trace("Working with DB: " + userNode.getParmDBLocation());

            // The first node a user can possibly open is the root node. Once that
            // happens, all child nodes are defined so defineChildren() is only called
            // once for the root node and never thereafter.
            if(userNode.isRootNode()){
                logger.trace("ParmDBtreeNode calling getNames("+userNode.getNodeID().substring(userNode.getParmDBIdentifier().length())+"*)");
                Vector children = SharedVars.getJParmFacade().getNames("*");
                logger.trace("ParmDBtreeNode gets "+children.size()+" names");

                if(children.size() == 0)
                {
                    userNode.setLeaf(true);
                }
                else
                {
                    Enumeration e = children.elements();
                    while( e.hasMoreElements() ) {
                        String pathString = (String) e.nextElement();
                        logger.trace("definePath: " + pathString);
                        definePath(aNode, pathString.split(PARMDB_TREENODE_SEPARATOR_CHAR), 0);
                    }
                }
            }
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager jParmDBnode-defineChildNodes: " + e);
        }
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
    }
    
    public TreeNode getRootNode(Object arguments){
        String[] argumentArray = (String[])arguments;
        jParmDBnode newPNode = new jParmDBnode(argumentArray[0],argumentArray[1]);
        newPNode.setParmDBIdentifier(argumentArray[0]);
        if(argumentArray.length==3){
            if(!argumentArray[2].equals("")){
                newPNode.setParmDBLocation(argumentArray[2]);
                
                newPNode.setRootNode(true);
            }
        }
        newPNode.setName(argumentArray[0]);
        TreeNode parmDBnode = new TreeNode(ParmDBTreeManager.instance,newPNode,newPNode.getName());
        
        return parmDBnode;
    }
    
    // Try to follow the specified path starting at the root node. If at some point
    // it is not possible to continue, then create all the remaining nodes on the path.
    // For example, if we start with an empty tree and the specified path is
    // 'solver:chi' then the node 'solver' does not exist and therefore the nodes
    // 'solver' and 'chi' will be created. If definePath is subsequently called again
    // with the path 'solver:rank' then the node 'solver' already exists. However, that
    // node only has one child called 'chi', so a child 'rank' will be added.
    private void definePath(TreeNode root, String[] path, int index)
    {
        if(index >= path.length)
        {
            // We've reached the end of the path, so we're done.
            return;
        }
        else
        {
            // Check if the children of the current root node contain
            // the current node on the path.
            TreeNode node = null;
            boolean found = false;
            Enumeration children = root.children();
            while(children.hasMoreElements())
            {
                node = (TreeNode) children.nextElement();
                if(((jParmDBnode) node.getUserObject()).getName().equals(path[index]))
                {
                    found = true;
                    break;
                }
            }
            
            if(found)
            {
                // Found. So let's continue with the next node on the path.
                definePath(node, path, index + 1);
            }
            else
            {
                // Not found. Create all the nodes on the path below and
                // including the current node.
                TreeNode pathRoot = root;
                jParmDBnode pathUserRoot = ((jParmDBnode) pathRoot.getUserObject());
                
                for(int i = index; i < path.length; i++)
                {
                    // Parent node is no longer a leaf node.
                    pathUserRoot.setLeaf(false);
                    
                    // Create a new jParmDBnode.
                    String parentID = pathUserRoot.getNodeID();
                    jParmDBnode item = new jParmDBnode(parentID + ParmDBTreeManager.PARMDB_TREENODE_SEPARATOR_CHAR + path[i], parentID);
                    item.setName(path[i]);
                    item.setLeaf(true);
                    item.setParmDBLocation(pathUserRoot.getParmDBLocation());
                    item.setParmDBIdentifier(pathUserRoot.getParmDBIdentifier());
                    
                    // Create a new TreeNode and add it to the current root.
                    TreeNode newNode = new TreeNode(ParmDBTreeManager.instance, item, item.getNodeID());
                    newNode.areChildrenDefined = true;
                    pathRoot.add(newNode);
                    
                    // Is this necessary??
                    TreeModelEvent evt = new TreeModelEvent(newNode, newNode.getPath());
                    fireTreeInsertionPerformed(evt);
                    
                    // Newly created node becomes the root node for the next
                    // iteration.
                    pathRoot = newNode;
                    pathUserRoot = ((jParmDBnode) pathRoot.getUserObject());
                }
            }
        }
    }
}
