/*
 * ParmDBTreeManager.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
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
 * @author pompert
 * @version $Id$
 */
public class ParmDBTreeManager extends GenericTreeManager implements ITreeManager{
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(OTDBParamTreeManager.class);
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
        String name = ((jParmDBnode)aNode.getUserObject()).name;
        return name;
    }
    
    public boolean isNodeLeaf(TreeNode aNode){
        boolean leaf = false;
        if (aNode.getUserObject() != null) {
            leaf = ((jParmDBnode)aNode.getUserObject()).leaf;
        }
        return leaf;
    }
    
    public void defineChildsForNode(TreeNode aNode) {
        
        logger.trace("Entry - TreeManager jParmDBnode-defineChildNodes("+toString()+")");
        try {
             // You must set the flag before defining children if you
            // use "add" for the new children. Otherwise you get an infinite
            // recursive loop, since add results in a call to getChildCount.
            // However, you could use "insert" in such a case.
            aNode.areChildrenDefined = true;
            Vector childs;
            if(((jParmDBnode)aNode.getUserObject()).name.equalsIgnoreCase("ParmDB")){
                
                logger.trace("Working with DB: ");
                SharedVars.getJParmFacade().setParmFacadeDB("/home/pompert/transfer/tParmFacade.in_mep");
                               
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
                TreeNode newNode = new TreeNode(this.instance,item,item.nodeID());
                aNode.add(newNode);
                TreeModelEvent evt = new TreeModelEvent(newNode,newNode.getPath());
                
                fireTreeInsertionPerformed(evt);
                
                
            }
            if(uniqueNames.size() == 0){
                ((jParmDBnode)aNode.getUserObject()).leaf=true;
            }
            
        } catch(Exception e) {
            logger.fatal("Exception during TreeManager jParmDBnode-defineChildNodes: " + e);
        }
        logger.trace("Exit - TreeManager defineChildNodes("+toString()+")");
    }
    
    public TreeNode getRootNode(Object arguments){
        String[] argumentArray = (String[])arguments;
        jParmDBnode newPNode = new jParmDBnode(argumentArray[0],argumentArray[1]);
        newPNode.name=argumentArray[0];
        TreeNode parmDBnode = new TreeNode(this.instance,newPNode,newPNode.name);
        
        return parmDBnode;
    }
       
}
