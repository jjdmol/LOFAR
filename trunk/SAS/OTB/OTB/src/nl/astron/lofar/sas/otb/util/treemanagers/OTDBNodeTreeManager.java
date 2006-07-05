/*
 * OTDBNodeTreeManager.java
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

import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base OTDBNodeTreeManager Class
 *
 * @created 26-01-2006, 14:56
 *
 * @author blaakmeer/Coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class OTDBNodeTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(OTDBNodeTreeManager.class);
    private static OTDBNodeTreeManager instance;
    
    /**
     * default constructor, protected by a singleton pattern
     */
    private OTDBNodeTreeManager(UserAccount anAccount) {
        super(anAccount);
    }
    
    public static OTDBNodeTreeManager getInstance(UserAccount anAccount){
        if(instance==null){
            instance = new OTDBNodeTreeManager(anAccount);
        }
        return instance;
    }
    
    public String getNameForNode(TreeNode aNode){
        String name = "";
        if(aNode.getUserObject()!= null && aNode.getUserObject() instanceof jOTDBnode){
            
            String splitName[]=((jOTDBnode)aNode.getUserObject()).name.split("[.]");
            if (splitName.length >=2) {
                name=splitName[splitName.length-1];
            } else {
                name=((jOTDBnode)aNode.getUserObject()).name;
            }
            if (((jOTDBnode)aNode.getUserObject()).index > 0 && !((jOTDBnode)aNode.getUserObject()).leaf) {
                if (name.contains("[") && name.contains("]")) {
                    
                } else {
                    name +="["+String.valueOf(((jOTDBnode)aNode.getUserObject()).index)+"]";
                }
            }
        }
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            if(aNode.getUserObject() instanceof jOTDBnode){
                leaf = ((jOTDBnode)aNode.getUserObject()).leaf;
            }
        }
        return leaf;
    }
    
    public void defineChildsForNode(TreeNode aNode) {
        logger.trace("Entry - TreeManager defineChildNodes("+toString()+")");
        
        if (aNode.getUserObject() == null) {
            return;
        }
        
        // You must set the flag before defining children if you
        // use "add" for the new children. Otherwise you get an infinite
        // recursive loop, since add results in a call to getChildCount.
        // However, you could use "insert" in such a case.
        aNode.areChildrenDefined = true;
        
        try {
            Vector childs =
                    SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(((jOTDBnode)aNode.getUserObject()).treeID(), ((jOTDBnode)aNode.getUserObject()).nodeID(), 1);
            
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                jOTDBnode item = (jOTDBnode)e.nextElement();
                logger.trace("Node name selected :"+item.name);
                TreeNode newNode = new TreeNode(this.instance,item,item.name);
                aNode.add(newNode);
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                fireTreeInsertionPerformed(evt);
            }
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager OTDB-defineChildNodes",e);
        }
    }
    public TreeNode getRootNode(Object arguments){
        jOTDBnode otdbNode=null;
        int itsTreeID = 0;
        try {
            itsTreeID = Integer.parseInt(arguments.toString());
        } catch (NumberFormatException ex) {
            logger.error("The OTDBNodeTreeManager received an incorrect TreeID! ",ex);
        }
        
        if (itsTreeID == 0 ) {
            // create a sample root node.
            otdbNode = new jOTDBnode(0,0,0,0);
            otdbNode.name = "No TreeSelection";
        } else {
            try {
                otdbNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getTopNode(itsTreeID);
            } catch (RemoteException ex) {
                logger.fatal("The OTDBNodeTreeManager could not build a root node! ",ex);
            }
        }
        TreeNode newNode = new TreeNode(this.instance,otdbNode,otdbNode.name);
        
        return newNode;
    }
}