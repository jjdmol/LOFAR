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

import java.util.Enumeration;
import java.util.Vector;
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
 * @updated
 */
public class BBSStepTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(BBSStepTreeManager.class);
    private static BBSStepTreeManager instance;
    private static String BBSSTEP_TREENODE_SEPARATOR_CHAR = ".";
    
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
        /*
        logger.trace("Entry - TreeManager BBSStepNode-defineChildNodes("+aNode.getName()+" ("+((BBSStepNode)aNode.getUserObject()).getNodeID()+"))");
        try {
            // You must set the flag before defining children if you
            // use "add" for the new children. Otherwise you get an infinite
            // recursive loop, since add results in a call to getChildCount.
            // However, you could use "insert" in such a case.
            aNode.areChildrenDefined = true;
            Vector childs = null;
            BBSStepNode containedNode = (BBSStepNode)aNode.getUserObject();
            
            if(((BBSStepNode)aNode.getUserObject()).isRootNode()){
                //childs = SharedVars.getJParmFacade().getNames(""+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length())+"*");
            }else{
                //childs = SharedVars.getJParmFacade().getNames("*"+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length()+1)+this.PARMDB_TREENODE_SEPARATOR_CHAR+"*");
            }
            Vector<String> uniqueNames = new Vector<String>();
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                String aValue = (String)e.nextElement();
                
                
                BBSStepNode item = new BBSStepNode(containedNode.getNodeID()+this.BBSSTEP_TREENODE_SEPARATOR_CHAR+childName,((BBSStepNode)aNode.getUserObject()).getNodeID());
                //item.leaf=true;
                item.setName(childName);
                logger.trace("Node name selected : "+item.getName());
                ((jParmDBnode)aNode.getUserObject()).setLeaf(false);
                TreeNode newNode = new TreeNode(this.instance,item,item.getNodeID());
                aNode.add(newNode);
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                
                fireTreeInsertionPerformed(evt);
                
                
            }
            if(uniqueNames.size() == 0){
                ((BBSStepNode)aNode.getUserObject()).setLeaf(true);
            }
            
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager BBSStepNode-defineChildNodes: " + e);
        }
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
        */
    }
    
    public TreeNode getRootNode(Object arguments){
        String[] argumentArray = (String[])arguments;
        BBSStepNode newPNode = new BBSStepNode(argumentArray[0],argumentArray[1]);
        if(argumentArray.length==3){
            if(!argumentArray[2].equals("")){
                newPNode.setRootNode(true);
            }
        }
        newPNode.setName(argumentArray[0]);
        TreeNode bbsNode = new TreeNode(this.instance,newPNode,newPNode.getName());
        
        return bbsNode;
    }
    
}
