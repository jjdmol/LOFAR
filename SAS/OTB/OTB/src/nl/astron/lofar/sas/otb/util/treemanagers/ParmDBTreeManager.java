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
import javax.swing.event.TreeModelListener;
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
            Vector childs;
            jParmDBnode containedNode = (jParmDBnode)aNode.getUserObject();
            SharedVars.getJParmFacade().setParmFacadeDB(containedNode.getParmDBLocation());
            logger.trace("Working with DB: "+containedNode.getParmDBLocation());
            if(((jParmDBnode)aNode.getUserObject()).isRootNode()){
                logger.trace("ParmDBtreeNode calling getNames("+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length())+"*)");
                childs = SharedVars.getJParmFacade().getNames(""+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length())+"*");
                logger.trace("ParmDBtreeNode gets "+childs.size()+" names");
            }else{
                logger.trace("ParmDBtreeNode calling getNames(*"+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length()+1)+this.PARMDB_TREENODE_SEPARATOR_CHAR+"*)");
                childs = SharedVars.getJParmFacade().getNames("*"+containedNode.getNodeID().substring(containedNode.getParmDBIdentifier().length()+1)+this.PARMDB_TREENODE_SEPARATOR_CHAR+"*");
                logger.trace("ParmDBtreeNode gets "+childs.size()+" names");
            }
            Vector<String> uniqueNames = new Vector<String>();
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                String aValue = (String)e.nextElement();
                String splitName[]= aValue.split("["+PARMDB_TREENODE_SEPARATOR_CHAR+"]");
                String parentLevels[] = containedNode.getNodeID().split("["+PARMDB_TREENODE_SEPARATOR_CHAR+"]");
                
                String trace = "ParmDBtreeNode gets name [";
                for(int i = 0;i<splitName.length;i++){
                    trace+=","+splitName[i];
                }
                logger.trace(trace+"]");
                if (splitName.length >=2) {
                    aValue=splitName[parentLevels.length-1];
                }
                
                if(!uniqueNames.contains(aValue)){
                    uniqueNames.addElement(aValue);
                }
            }
            e = uniqueNames.elements();
            while( e.hasMoreElements()  ) {
                
                String childName = (String)e.nextElement();
                
                jParmDBnode item = new jParmDBnode(containedNode.getNodeID()+this.PARMDB_TREENODE_SEPARATOR_CHAR+childName,((jParmDBnode)aNode.getUserObject()).getNodeID());
                //item.leaf=true;
                item.setName(childName);
                item.setParmDBLocation(((jParmDBnode)aNode.getUserObject()).getParmDBLocation());
                item.setParmDBIdentifier(containedNode.getParmDBIdentifier());
                logger.trace("Node name selected : "+item.getName());
                ((jParmDBnode)aNode.getUserObject()).setLeaf(false);
                TreeNode newNode = new TreeNode(this.instance,item,item.getNodeID());
                aNode.add(newNode);
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                
                fireTreeInsertionPerformed(evt);
                
                
            }
            if(uniqueNames.size() == 0){
                ((jParmDBnode)aNode.getUserObject()).setLeaf(true);
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
        TreeNode parmDBnode = new TreeNode(this.instance,newPNode,newPNode.getName());
        
        return parmDBnode;
    }
       
}
