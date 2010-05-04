/*
 * BBSStepTreeManager.java
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

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import javax.swing.event.TreeModelEvent;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.GenericTreeManager;
import nl.astron.lofar.sas.otb.util.treemanagers.ITreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base BBSStepTreeManager Class
 *
 * @author pompert
 * @version $Id$
 * @created 25-7-2006, 13:56
 */
public class BBSStepTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(BBSStepTreeManager.class);
    private static BBSStepTreeManager instance;
    
    /**
     * default constructor, protected by a singleton pattern
     */
    private BBSStepTreeManager(UserAccount anAccount) {
        super(anAccount);
    }
        
    public static BBSStepTreeManager getInstance(UserAccount anAccount){
        if(instance==null){
            instance = new BBSStepTreeManager(anAccount);
        }
        return instance;
    }
    
    public String getNameForNode(TreeNode aNode){
        //add operation type to the name
        String name = ((BBSStepNode)aNode.getUserObject()).getName();
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            leaf = ((BBSStepNode)aNode.getUserObject()).isLeaf();
        }
        return leaf;
    }
    
    public void defineChildsForNode(TreeNode aNode) {
        
        if (aNode.getUserObject() == null) {
            logger.debug("Error - TreeManager BBSStepNode-defineChildNodes("+aNode.getName()+" does not contain a user object)");
            return;
        }
        //get instance to the bbs step tree helper class
        BBSStepDataManager BBSsdm = BBSStepDataManager.getInstance();
        
        //BBS Node contained in the TreeNode
        BBSStepNode containedBBSNode = (BBSStepNode)aNode.getUserObject();
        //jOTDB Node contained that is needed to fetch child nodes from OTDB
        
        
        logger.trace("Entry - TreeManager BBSStepNode-defineChildNodes("+aNode.getName()+" ("+containedBBSNode.getName()+"))");
        
        // You must set the flag before defining children if you
        // use "add" for the new children. Otherwise you get an infinite
        // recursive loop, since add results in a call to getChildCount.
        // However, you could use "insert" in such a case.
        aNode.areChildrenDefined = true;
        if(containedBBSNode.isRootNode()){
            jOTDBnode rootNode = containedBBSNode.getOTDBNode();
            BBSsdm.setStepContainerNode(rootNode);
            
            //retrieve the strategy steps
            BBSStrategy theStrategy = BBSsdm.getStrategy();
            
            for(BBSStep childStep : theStrategy.getChildSteps()){
                BBSStepNode newChildStepNode = new BBSStepNode(childStep);
                TreeNode newNode = new TreeNode(BBSStepTreeManager.instance,newChildStepNode,newChildStepNode.getName());
                aNode.add(newNode);
            }
        }else{
            //expand the first steps in the tree
            BBSStep containedBBSStep = containedBBSNode.getContainedStep();
            
            for(BBSStep aStep : containedBBSStep.getChildSteps()){
                logger.trace("Child Node found for BBS Step Tree :"+aStep.getName());
                aStep.setParentStep(containedBBSStep);
                BBSStepNode newPNode = new BBSStepNode(aStep);
                newPNode.setName(aStep.getName());
                newPNode.setRootNode(false);
                TreeNode newNode = new TreeNode(BBSStepTreeManager.instance,newPNode,newPNode.getName());
                aNode.add(newNode);
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                fireTreeInsertionPerformed(evt);
                
            }
        }
        
        
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
        
    }
    
    public TreeNode getRootNode(Object arguments){
        Object[] argumentArray = (Object[])arguments;
        jOTDBnode userObject = (jOTDBnode)argumentArray[1];
        String title = (String)argumentArray[0];
        BBSStepNode newPNode = new BBSStepNode(null);
        newPNode.setRootNode(true);
        newPNode.setName(title);
        newPNode.setOTDBNode(userObject);
        newPNode.leaf=false;
        TreeNode bbsNode = new TreeNode(BBSStepTreeManager.instance,newPNode,newPNode.getName());
        
        return bbsNode;
    }
    
}
