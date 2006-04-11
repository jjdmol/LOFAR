/*
 * ComponentTreeNode.java
 *
 * Created on April, 04, 2006, 16:29
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;
import javax.swing.tree.*;
import java.util.Vector;
import java.util.Enumeration;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import org.apache.log4j.Logger;


/**
 * Class that wraps an ComponentNode into a DefaultMutableTreeNode so that it
 * can be used to build a tree *
 *
 * @author coolen
 */
public class ComponentTreeNode extends DefaultMutableTreeNode {
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(ComponentTreeNode.class);
    
    private boolean   areChildrenDefined = false;
    private jOTDBparam itsParam;
    private OtdbRmi otdbRmi;
    
    
    /**
     * default constructor - creates an empty node.
     */
    public ComponentTreeNode() {
        logger.trace("Entry - ComponentTreeNode()");
        
        itsParam = new jOTDBparam(0,0,0);
        itsParam.name = "No param defined";
        otdbRmi = null;
        
        logger.trace("Exit - ComponentTreeNode()");
    }
    
    /**
     * constructor
     *
     * @param node jOTDBnode to be wrapped
     * @param otdbRmi Reference to the OTDB database
     */
    public ComponentTreeNode(jOTDBparam aParam, OtdbRmi otdbRmi) {
        logger.trace("Entry - ComponentTreeNode("+aParam.name+") paramID: "+ aParam.paramID());
        
        this.itsParam = aParam;
        this.otdbRmi = otdbRmi;
        
        logger.trace("Exit - ComponentTreeNode("+aParam.name+")");
    }
    
    /**
     * jOTDBparam accessor
     */
    public jOTDBparam getOTDBparam() {
        return itsParam;
    }
    
    /**
     * jOTDBparam accessor
     */
    public void setOTDBparam(jOTDBparam aParam) {
        this.itsParam = null;
        this.itsParam = aParam;
    }
    
    /**
     * OtdbRmi accessor
     */
    public void setOTDBrmi(OtdbRmi otdbRmi) {
        this.otdbRmi = otdbRmi;
    }
    
    /**
     * Called by the JTree
     * Determines if the node has childs or not
     */
    public boolean isLeaf() {
        logger.trace("Entry - isLeaf("+toString()+")");
        
        if (itsParam == null || otdbRmi == null) {
            return false;
        }
        
        // if the name of the param doesn't start with # it's a leaf
        if (itsParam.name.charAt(0) == '#' ) {
            return false;
        }
        
        return true;
    }
    
    /**
     * Called by the JTree
     * Adds childs if they are not defined and returns the number of childs
     */
    public int getChildCount() {
        logger.trace("Entry - getChildCount("+toString()+")");
        
        if (!areChildrenDefined)
            defineChildNodes();
        int childCount = super.getChildCount();
        
        logger.trace("Exit - getChildCount("+toString()+"): " + childCount);
        return(childCount);
    }
    
    /**
     * Retrieves a list of child nodes from the OTDB and adds them
     */
    private void defineChildNodes() {
        logger.trace("Entry - defineChildNodes("+toString()+")");
        
        if (itsParam == null || otdbRmi == null) {
            return;
        }
        
        // You must set the flag before defining children if you
        // use "add" for the new children. Otherwise you get an infinite
        // recursive loop, since add results in a call to getChildCount.
        // However, you could use "insert" in such a case.
        areChildrenDefined = true;
        
        // Only need to be done for params who's name starts with #
        if  (itsParam.name.charAt(0) != '#') {
            return;
        }
        
        String aNodeName= itsParam.name.substring(1,itsParam.name.length());
        try {
            
            
            Vector<jVICnodeDef> nodes = otdbRmi.getRemoteMaintenance().getComponentList(aNodeName,false);
            
            if (nodes.size() > 0) {
                logger.debug("Found "+ nodes.size()+ " nr of matches for node "+aNodeName);            
            } else {
                logger.debug("No matches for "+ aNodeName);
                return;
            }
            
            Vector<jOTDBparam> params = otdbRmi.getRemoteMaintenance().getComponentParams(((jVICnodeDef)nodes.elementAt(0)).nodeID());
            
            Enumeration e = params.elements();
            while( e.hasMoreElements()  ) {
                jOTDBparam item = (jOTDBparam)e.nextElement();
                add(new ComponentTreeNode(item, otdbRmi));
            }
        } catch(Exception e) {
            logger.fatal("Exception during defineChildNodes: " + e);
        }
        logger.trace("Exit - defineChildNodes("+toString()+")");
    }
    
    /**
     * Called by the JTree
     * @return the human readable name of the node
     */
    public String toString() {
        String name = "";
        if(itsParam.name != null) {
            name = itsParam.name;
            if (itsParam.index > 0) {
                name +="["+String.valueOf(itsParam.index)+"]";
            }
        }
        return name;
    }
    
    

}
