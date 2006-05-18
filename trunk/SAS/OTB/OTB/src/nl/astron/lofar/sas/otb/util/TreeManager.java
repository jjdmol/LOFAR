/*
 * TreeManager.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.util.Enumeration;
import java.util.Vector;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base TreeManager Class
 *
 * @author pompert
 * @version $Id$
 */
public class TreeManager {
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(TreeManager.class);
    private static TreeManager instance;
    
    /**
     * default constructor, protected by a singleton pattern
     */
    private TreeManager() {
        
    }
    
    public static TreeManager getInstance(){
        if(instance==null){
            instance = new TreeManager();
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
            
        } else if(aNode.getUserObject()!= null && aNode.getUserObject() instanceof jParmDBnode){
            
            name = ((jParmDBnode)aNode.getUserObject()).name;
            
        } else if(aNode.getUserObject()!= null && aNode.getUserObject() instanceof jOTDBparam){
            
            name = ((jOTDBparam)aNode.getUserObject()).name;
            if (((jOTDBparam)aNode.getUserObject()).index > 0) {
                name +="["+String.valueOf(((jOTDBparam)aNode.getUserObject()).index)+"]";
            }
        }
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            if(aNode.getUserObject() instanceof jParmDBnode){
            leaf = ((jParmDBnode)aNode.getUserObject()).leaf;  
            }
            else if(aNode.getUserObject() instanceof jOTDBnode){
            leaf = ((jOTDBnode)aNode.getUserObject()).leaf;  
            }
            else if(aNode.getUserObject() instanceof jOTDBparam){
                leaf = true;
                if (((jOTDBparam)aNode.getUserObject()).name.charAt(0) == '#' ) {
                    leaf = false;
                } 
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
        
        if(aNode.getUserObject() instanceof jOTDBnode){
            
            defineOTDBNodeChildNode(aNode);
            
        }else if(aNode.getUserObject() instanceof jParmDBnode){
            
            defineParmDBChildNode(aNode);
            
        } else if (aNode.getUserObject() instanceof jOTDBparam){
            
            defineOTDBParamChildNode(aNode);
        }
    }
    
    private void defineOTDBNodeChildNode(TreeNode aNode){
        try {
            Vector childs =
                    SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(((jOTDBnode)aNode.getUserObject()).treeID(), ((jOTDBnode)aNode.getUserObject()).nodeID(), 1);
            
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                jOTDBnode item = (jOTDBnode)e.nextElement();
                logger.trace("Node name selected :"+item.name);
                TreeNode newNode = new TreeNode(item);
                aNode.add(newNode);
                
                //testcode to add parmdb
                
                if(item.name.equalsIgnoreCase("Observation.AO")){
                    jParmDBnode newPNode = new jParmDBnode("ParmDB","BBS");
                    newPNode.name="ParmDB";
                    TreeNode parmDBnode = new TreeNode(newPNode);
                    
                    newNode.add(parmDBnode);
                }
                
            }
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager OTDB-defineChildNodes: " + e);
            e.printStackTrace();
        }
    }
    
    private void defineOTDBParamChildNode(TreeNode aNode){
        logger.trace("Entry - TreeNode jOTDBparam-defineChildNodes("+toString()+")");
        
        if (aNode.getUserObject() == null) {
            return;
        }
        
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
                aNode.add(new TreeNode(item));
            }
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager jOTDBparam-defineChildNodes: " + e);
        }
        
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
    }
    
    private void defineParmDBChildNode(TreeNode aNode){
        logger.trace("Entry - TreeManager jParmDBnode-defineChildNodes("+toString()+")");
        try {
            Vector childs;
            if(((jParmDBnode)aNode.getUserObject()).name.equalsIgnoreCase("ParmDB")){
                logger.trace("ParmDBtreeNode calling getNames("+((jParmDBnode)aNode.getUserObject()).nodeID().substring(6)+"*)");
                childs = SharedVars.getJParmFacade().getNames(""+((jParmDBnode)aNode.getUserObject()).nodeID().substring(6)+"*");
                logger.trace("ParmDBtreeNode gets "+childs.size()+" names");
            }else{
                logger.trace("ParmDBtreeNode calling getNames(*"+((jParmDBnode)aNode.getUserObject()).nodeID().substring(7)+".*)");
                childs = SharedVars.getJParmFacade().getNames("*"+((jParmDBnode)aNode.getUserObject()).nodeID().substring(7)+".*");
                logger.trace("ParmDBtreeNode gets "+childs.size()+" names");
            }
            Vector<String> uniqueNames = new Vector<String>();
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                String aValue = (String)e.nextElement();
                String splitName[]= aValue.split("[.]");
                String parentLevels[] = ((jParmDBnode)aNode.getUserObject()).nodeID().split("[.]");
                
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
                
                jParmDBnode item = new jParmDBnode(((jParmDBnode)aNode.getUserObject()).nodeID()+"."+childName,((jParmDBnode)aNode.getUserObject()).nodeID());
                //item.leaf=true;
                item.name = childName;
                logger.trace("Node name selected : "+item.name);
                ((jParmDBnode)aNode.getUserObject()).leaf=false;
                aNode.add(new TreeNode(item));
            }
            if(uniqueNames.size() == 0){
                ((jParmDBnode)aNode.getUserObject()).leaf=true;
            }
            
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager jParmDBnode-defineChildNodes: " + e);
        }
        
    }
    
}
