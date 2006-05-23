/*
 * OTDBParamTreeManager.java
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
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base OTDBParamTreeManager Class
 *
 * @author pompert
 * @version $Id$
 */
public class OTDBParamTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(OTDBParamTreeManager.class);
    private static OTDBParamTreeManager instance;
      
    /**
     * default constructor, protected by a singleton pattern
     */
    private OTDBParamTreeManager(UserAccount anAccount) {
        super(anAccount);
    }
    
    public static OTDBParamTreeManager getInstance(UserAccount anAccount){
        if(instance==null){
            instance = new OTDBParamTreeManager(anAccount);
        }
        return instance;
    }
    
    public String getNameForNode(TreeNode aNode){
        String name = "";
        name = ((jOTDBparam)aNode.getUserObject()).name;
        if (((jOTDBparam)aNode.getUserObject()).index > 0) {
            name +="["+String.valueOf(((jOTDBparam)aNode.getUserObject()).index)+"]";
        }
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            leaf = true;
            if (((jOTDBparam)aNode.getUserObject()).name.charAt(0) == '#' ) {
                leaf = false;
            }
        }
        return leaf;
    }
    
    public void defineChildsForNode(TreeNode aNode) {
        
        logger.trace("Entry - TreeNode jOTDBparam-defineChildNodes("+toString()+")");
        
        if (aNode.getUserObject() == null) {
            return;
        }
         // You must set the flag before defining children if you
        // use "add" for the new children. Otherwise you get an infinite
        // recursive loop, since add results in a call to getChildCount.
        // However, you could use "insert" in such a case.
        aNode.areChildrenDefined = true;
        // Only need to be done for params who's name starts with #
        if  (((jOTDBparam)aNode.getUserObject() ).name.charAt(0) != '#') {
            return;
        }
        
        String aNodeName= ((jOTDBparam)aNode.getUserObject() ).name.substring(1,((jOTDBparam)aNode.getUserObject()).name.length());
        try {
            
            
            Vector<jVICnodeDef> nodes = SharedVars.getOTDBrmi().getRemoteMaintenance().getComponentList(aNodeName,false);
            
            if (nodes.size() > 0) {
                logger.debug("Found "+ nodes.size()+ " nr of matches for node "+aNodeName);
            } else {
                logger.debug("No matches for "+ aNodeName);
                return;
            }
            
            Vector<jOTDBparam> params = SharedVars.getOTDBrmi().getRemoteMaintenance().getComponentParams(((jVICnodeDef)nodes.elementAt(0)).nodeID());
            
            Enumeration e = params.elements();
            while( e.hasMoreElements()  ) {
                jOTDBparam item = (jOTDBparam)e.nextElement();
                TreeNode newNode = new TreeNode(this.instance,item,item.name);
                aNode.add(newNode);
                //testcode to add parmdb
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                
                fireTreeInsertionPerformed(evt);
            }
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager jOTDBparam-defineChildNodes: " + e);
        }
        
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
    }
    public TreeNode getRootNode(Object arguments){
        jOTDBparam aParam =null;
        jVICnodeDef aVICnodeDef=null;
        int itsComponentID = 0;
        try {
            itsComponentID = Integer.parseInt(arguments.toString());
        } catch (NumberFormatException ex) {
            logger.error("The OTDBParamTreeManager received an incorrect itsComponentID! ",ex);
        }
        if (itsComponentID == 0 ) {
            // create a sample component param
            aParam = new jOTDBparam(0,0,0);
            aParam.name = "No ParamSelection";
        } else {
            try {
                aVICnodeDef = SharedVars.getOTDBrmi().getRemoteMaintenance().getComponentNode(itsComponentID);
            } catch (RemoteException ex) {
                logger.fatal("The OTDBParamTreeManager could not build a root node! ",ex);
            }
            if (aVICnodeDef != null) {
                // create a fake param to pass to componentTree, to simulate a node param
                aParam = new jOTDBparam(0,itsComponentID,0);
                aParam.name="#"+aVICnodeDef.name;
                aParam.index=0;
                aParam.limits="";
                aParam.type=-1;
                aParam.unit=-1;
                aParam.description="";
            } else {
                logger.debug("failed to get ComponentNode");
            }
        }
        TreeNode newNode = new TreeNode(this.instance,aParam,aParam.name);
        return newNode;
    }
}
