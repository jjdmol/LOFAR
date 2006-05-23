/*
 * OTDBNodeTreeManager.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
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
 * @author pompert
 * @version $Id$
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
                name +="["+String.valueOf(((jOTDBnode)aNode.getUserObject()).index)+"]";
            }
        }else if(aNode.getUserObject()!= null && aNode.getUserObject() instanceof jParmDBnode){
            name=((jParmDBnode)aNode.getUserObject()).name;
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