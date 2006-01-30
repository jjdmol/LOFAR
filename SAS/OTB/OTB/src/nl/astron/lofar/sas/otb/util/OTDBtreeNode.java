/*
 * OTDBtreeNode.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.awt.*;
import java.rmi.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.util.Vector;
import java.util.Enumeration;
import java.util.Random;
import org.apache.log4j.Logger;
import jOTDB.*;

/**
 * Class that wraps an jOTDBnode into a DefaultMutableTreeNode so that it 
 * can be used to build a tree * 
 *
 * @author blaakmeer
 */
public class OTDBtreeNode extends DefaultMutableTreeNode {

    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(OTDBtreeNode.class);

    private boolean   areChildrenDefined = false;
    private jOTDBnode node;
    private OtdbRmi otdbRmi;
    
    // TEST ONLY. In the future, childs are retrieved from the OTDB
    private Vector childs;

    /**
     * default constructor - creates an empty node.
     */
    public OTDBtreeNode() {
        logger.trace("Entry - OTDBtreeNode()");

        node = new jOTDBnode(0,0,0,0);
        node.name = "No node defined";
        otdbRmi = null;
        
        logger.trace("Exit - OTDBtreeNode()");
    }
    
    /**
     * constructor
     * 
     * @param node jOTDBnode to be wrapped
     * @param otdbRmi Reference to the OTDB database 
     */
    public OTDBtreeNode(jOTDBnode node, OtdbRmi otdbRmi) {
        logger.trace("Entry - OTDBtreeNode("+toString()+")");

        this.node = node;
        this.otdbRmi = otdbRmi;
        
        // TEST ONLY
        childs = defineTestChilds(node.nodeID(),node.treeID());

        logger.trace("Exit - OTDBtreeNode("+toString()+")");
    }
    
    /**
     * jOTDBnode accessor
     */
    public jOTDBnode getOTDBnode() {
        return node;
    }

    /**
     * jOTDBnode accessor
     */
    public void setOTDBnode(jOTDBnode node) {
        this.node = null;
        this.node = node;
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
        
        boolean isLeaf = false;
        try {
            //TODO Vector childs = otdbRmi.getRemoteMaintenance().getItemList(node.nodeID(), node.treeID(), 1);

            if(childs.size() == 0)
                isLeaf = true;
        }
        catch(Exception e) {
            logger.fatal("Exception during defineChildNodes: " + e);
            isLeaf = true;
        }
        logger.trace("Exit - isLeaf("+toString()+"): " + isLeaf);
        return isLeaf;
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

        // You must set the flag before defining children if you
        // use "add" for the new children. Otherwise you get an infinite
        // recursive loop, since add results in a call to getChildCount.
        // However, you could use "insert" in such a case.
        areChildrenDefined = true;
        
        try {
            //Vector childs = otdbRmi.getRemoteMaintenance().getItemList(node.nodeID(), node.treeID(), 1);

            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                jOTDBnode item = (jOTDBnode)e.nextElement();        
                add(new OTDBtreeNode(item, otdbRmi));
            }
        }
        catch(Exception e) {
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
        if(node != null) {
            name = node.name;
        }
        return name;
    }
    
    
    /**
    * TEST ONLY. Defines some child nodes
    */
    private Vector defineTestChilds(int aTreeID, int topNode) {
        logger.trace("Entry - defineChildNodes("+toString()+")(" + aTreeID + "," + topNode + ")");

        // using Java Generics to get a typesafe container
        Vector<jOTDBnode> v = new Vector<jOTDBnode>();
        Random rand = new Random();
        
        int childs = rand.nextInt(10);
        for(int i = 0; i < childs; i++) {
            int randomNumber = rand.nextInt();
            jOTDBnode jotdbnode = new jOTDBnode(aTreeID, randomNumber, topNode, 0);
            jotdbnode.name = "Node_"+randomNumber;
            v.add(jotdbnode);
        }

        logger.trace("Exit - defineChildNodes("+toString()+")(" + aTreeID + "," + topNode + ")");
        return v;
    }
}
