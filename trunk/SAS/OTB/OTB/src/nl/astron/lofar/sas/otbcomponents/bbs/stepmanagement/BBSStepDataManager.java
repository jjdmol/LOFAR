/*
 * BBSStepDataManager.java
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
 *
 */

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Vector;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import org.apache.log4j.Logger;

/**
 * @version $Id$
 * @created July 26, 2006, 10:04 AM
 * @author pompert
 */
public class BBSStepDataManager{
    
    private static BBSStepDataManager instance;
    private static Logger logger = Logger.getLogger(BBSStepDataManager.class);
    private static jOTDBnode stepContainerNode = null;
    private static Vector OTDBcomponentCache = null;
    
    /** Creates a new instance of BBSStepDataManager */
    private BBSStepDataManager() {
        
    }
    
    public static synchronized BBSStepDataManager getInstance(){
        if(instance==null){
            instance = new BBSStepDataManager();
        }
        return instance;
    }
    
    public Vector<BBSStepNode> buildStepTree(BBSStepNode stepContainerNode, boolean buildStrategyStepTree) throws RemoteException{
        Vector<BBSStepNode> returnNode = new Vector<BBSStepNode>();
        
        //Fetch the step names that are mentioned in the strategy tree (Strategy.Steps)
        
        //The following otdbnode should be the Step container node (Step)
        jOTDBnode rootNode = stepContainerNode.getOTDBNode();
        this.setStepContainerNode(rootNode);
        jOTDBnode strategyStepsParameter = this.getStrategyStepsNode(rootNode);
        
        if(strategyStepsParameter!=null){
            //retrieve the step names mentioned in the strategy steps parameter (Strategy.Steps)
            Vector<String> strategySteps = this.getVectorFromString(strategyStepsParameter.limits,true);
            
            if(strategySteps.size()>0){
                //Get all the steps present in the BBS Step Container
                Vector stepsVector = new Vector();
                if(buildStrategyStepTree){
                    //retrieve steps from location Step
                    stepsVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.nodeID(), 1);
                }else{
                    //retrieve steps from location Step.XXX
                    stepsVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.parentID(), 1);
                }
                Enumeration se = stepsVector.elements();
                //loop through steps
                while( se.hasMoreElements()  ) {
                    jOTDBnode aHWNode = (jOTDBnode)se.nextElement();
                    
                    //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
                    if (strategySteps.contains(aHWNode.name)) {
                        //Create a new step and build it (with its substeps as well)
                        BBSStep newStep = new BBSStep(aHWNode.name);
                        newStep.setStepContainerPointer(rootNode);
                        
                        buildStep(newStep,aHWNode);
                        BBSStepNode newChildStepNode = new BBSStepNode(newStep);
                        
                        newChildStepNode.setOTDBNode(aHWNode);
                        returnNode.add(newChildStepNode);
                        logger.trace("Strategy Step defined : "+newStep.getName());
                    }
                }
            }
        }
        return returnNode;
    }
    
    public void buildStep(BBSStep parentNode, jOTDBnode parentOTDBnode) throws RemoteException{
        
        //
        //TODO:add variables to step
        //
        
        //Add substeps
        jOTDBnode strategyStepsParameter=null;
        //retrieve 1 levels of parameters to locate Step.XXX.Steps
        
        Vector HWchilds = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.nodeID(), 1);
        // get all the params per child
        Enumeration e1 = HWchilds.elements();
        while( e1.hasMoreElements()  ) {
            
            jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
            strategyStepsParameter=null;
            //retrieving Steps
            if (aHWNode.leaf && aHWNode.name.equals("Steps")) {
                strategyStepsParameter = aHWNode;
                logger.trace("Strategy Steps defined :"+strategyStepsParameter.limits);
            } else if (aHWNode.leaf && aHWNode.name.equals("Sources")) {
                parentNode.setSources(this.getVectorFromString(aHWNode.limits,true));
                
            } else if (aHWNode.leaf && aHWNode.name.equals("ExtraSources")) {
                parentNode.setExtraSources(this.getVectorFromString(aHWNode.limits,true));
                
            } else if (aHWNode.leaf && aHWNode.name.equals("InstrumentModel")) {
                parentNode.setInstrumentModel(this.getVectorFromString(aHWNode.limits,true));
                
            } else if (aHWNode.leaf && aHWNode.name.equals("OutputData")) {
                parentNode.setOutputDataColumn(aHWNode.limits);
            }
            //Set the following values
            else if (aHWNode.leaf && aHWNode.name.equals("Operation")) {
                
                //SET OPERATION!!
                
            } else if (!aHWNode.leaf && aHWNode.name.equals("Correlation")) {
                Vector correlationParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = correlationParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aHWNode.leaf && aHWNode.name.equals("Selection")) {
                        parentNode.setCorrelationSelection(aHWNode.limits);
                    } else if (aHWNode.leaf && aHWNode.name.equals("Type")) {
                        parentNode.setCorrelationType(this.getVectorFromString(aHWNode.limits,true));
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Baselines")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aHWNode.leaf && aHWNode.name.equals("Station1")) {
                        parentNode.setStation1Selection(this.getVectorFromString(aHWNode.limits,true));
                    } else if (aHWNode.leaf && aHWNode.name.equals("Station2")) {
                        parentNode.setStation2Selection(this.getVectorFromString(aHWNode.limits,true));
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Integration")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aHWNode.leaf && aHWNode.name.equals("Time")) {
                        parentNode.setIntegrationTime(Double.parseDouble(aHWNode.limits));
                    } else if (aHWNode.leaf && aHWNode.name.equals("Freq")) {
                        parentNode.setIntegrationFrequency(Double.parseDouble(aHWNode.limits));
                    }
                }
            }
        }
        
        if(strategyStepsParameter!=null){
            //retrieve the step names mentioned in the strategy steps parameter (Strategy.Steps)
            Vector<String> strategySteps = this.getVectorFromString(strategyStepsParameter.limits,true);
            
            if(strategySteps.size()>0){
                //Get all the steps present in the BBS Step Container
                Vector stepsVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.parentID(), 1);
                Enumeration se = stepsVector.elements();
                //loop through steps
                while( se.hasMoreElements()  ) {
                    jOTDBnode aHWNode = (jOTDBnode)se.nextElement();
                    
                    //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
                    if (!aHWNode.leaf && strategySteps.contains(aHWNode.name)) {
                        //Create a new step and build it (with its substeps as well)
                        BBSStep newStep = new BBSStep(aHWNode.name);
                        newStep.setStepContainerPointer(parentNode.getStepContainerPointer());
                        
                        //build its childsteps recursively
                        buildStep(newStep,aHWNode);
                        parentNode.addChildStep(newStep);
                        logger.trace("Step defined : "+newStep.getName());
                    }
                }
            }
        }
    }
    public void persistStep(BBSStep aBBSStep){
        jOTDBnode stepsNode = aBBSStep.getStepContainerPointer();
        if(stepsNode == null){
            stepsNode = this.getStepContainerNode();
        }
        String name = aBBSStep.getName();
        boolean isStrategyStep = !aBBSStep.hasParentStep();
        jOTDBnode existingStepNode = null;
        
        //check if the step is present in the Step Container
        Vector stepsVector =  retrieveChildDataForNode(stepsNode);
        Enumeration se = stepsVector.elements();
        //loop through steps
        
        
        while( se.hasMoreElements()  ) {
            jOTDBnode aHWNode = (jOTDBnode)se.nextElement();
            /* ENABLE WHEN RENAMING COMPONENTS WORKS
             if(aHWNode.name.equals("DefaultBBSStep")){
                try {
                    SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                } catch (RemoteException ex) {
                     logger.debug("PersistStep(): Unable to delete the default Template Step from the Step Container: "+aBBSStep.getName());
                }
            }*/
            //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
            if (!aHWNode.leaf && aHWNode.name.equals(aBBSStep.getName())) {
                existingStepNode = aHWNode;
                logger.trace("PersistStep(): Step found in Step Container: "+aBBSStep.getName());
            }
        }
        
        //Update the existing step node
        if(existingStepNode != null){
            
            HashMap<String,String> unprocessedParms = new HashMap<String,String>();
            
            unprocessedParms.put("OutputData",aBBSStep.getOutputDataColumn());
            unprocessedParms.put("Sources",aBBSStep.getOutputDataColumn());
            unprocessedParms.put("ExtraSources",aBBSStep.getOutputDataColumn());
            unprocessedParms.put("InstrumentModel",aBBSStep.getOutputDataColumn());
            //do other parms
            
            //retrieve all existing step parameters
            Vector stepParametersVector = retrieveChildDataForNode(existingStepNode);
            Enumeration spe = stepParametersVector.elements();
            
            while( spe.hasMoreElements()  ) {
                jOTDBnode aHWNode = (jOTDBnode)spe.nextElement();
                if (aHWNode.leaf && aHWNode.name.equals("OutputData")){
                    if(!aHWNode.limits.equals(aBBSStep.getOutputDataColumn())){
                        aHWNode.limits=aBBSStep.getOutputDataColumn();
                        try {
                            if(aBBSStep.getParentStep() != null &&
                                    aBBSStep.getOutputDataColumn().equals(aBBSStep.getParentStep().getOutputDataColumn())){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }
                        } catch (RemoteException ex) {
                            logger.error("PersistStep(): Step-OutData could not be updated ",ex);
                        }
                    }
                    unprocessedParms.remove("OutputData");
                }
                //do other parms that are left over
            }
            for(String key : unprocessedParms.keySet()){
                try {
                    int newNodeID = SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(this.getComponentForNode(key),existingStepNode.treeID(),existingStepNode.nodeID());
                    jOTDBnode newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(existingStepNode.treeID(),newNodeID);
                    newStepNode.limits = unprocessedParms.get(key);
                    SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(newStepNode);
                } catch (RemoteException ex) {
                    logger.error("PersistStep(): Step "+existingStepNode.name+"could not be filled with missing parameters!",ex);
                }
                
                
            }
            
            //Insert a new step node
        }else{
            try {
                //new step has to be generated and persisted.
                
                //fetch list of components that contains the BBS Step tree items
                int stepTemplateNodeId = this.getComponentForNode("DefaultBBSStep");
                int stepTemplateCorrelationNodeId = this.getComponentForNode("Correlation");
                int stepTemplateBaselinesNodeId = this.getComponentForNode("Baselines");
                int stepTemplateIntegrationNodeId = this.getComponentForNode("Integration");
                
                if(stepTemplateNodeId!=0){
                    //copy the template step tree
                    int newStepNodeID = SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateNodeId,stepContainerNode.treeID(),stepContainerNode.nodeID());
                    
                    //fetch the generated node as the step node
                    jOTDBnode newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    
                    //add the subcomponents for the step as well (correlation, integration and baselines)
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateBaselinesNodeId,stepContainerNode.treeID(),newStepNode.nodeID());
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateCorrelationNodeId,stepContainerNode.treeID(),newStepNode.nodeID());
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateIntegrationNodeId,stepContainerNode.treeID(),newStepNode.nodeID());
                    
                    //update the node name
                    newStepNode.name = aBBSStep.getName();
                    SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(newStepNode);
                    newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    Vector stepParametersVector = retrieveChildDataForNode(newStepNode);
                    Enumeration spe = stepParametersVector.elements();
                    
                    while( spe.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)spe.nextElement();
                        
                        if(aHWNode.name.equals("OutputData")){
                            aHWNode.limits = aBBSStep.getOutputDataColumn();
                            if(aBBSStep.getParentStep() != null &&
                                    aBBSStep.getOutputDataColumn().equals(aBBSStep.getParentStep().getOutputDataColumn())){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }
                        }
                        
                    }
                    
                }
            } catch (RemoteException ex) {
                logger.error("PersistStep(): New step could not be generated using the DefaultBBSStep template as a component",ex);
            }
            
        }
        
        //update references to this step
        jOTDBnode parentStepsNode = null;
        
        if(isStrategyStep){
            parentStepsNode = this.getStrategyStepsNode(this.getStepContainerNode());
        }else{
            //update the references in the parent step to contain the new step.
            BBSStep parentBBSStep = aBBSStep.getParentStep();
            
            try {
                //Get all the steps present in the BBS Step Container and limit to the step defined as the step parent.
                Vector parentParmVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(this.getStepContainerNode().treeID(), this.getStepContainerNode().nodeID(), 1);
                Enumeration ppe = parentParmVector.elements();
                jOTDBnode parentStepNode = null;
                //loop through steps
                while( ppe.hasMoreElements()  ) {
                    jOTDBnode aHWNode = (jOTDBnode)ppe.nextElement();
                    if(aHWNode.name.equals(parentBBSStep.getName())){
                        parentStepNode = aHWNode;
                    }
                }
                if(parentStepNode!=null){
                    Vector parentParmsVector = this.retrieveChildDataForNode(parentStepNode);
                    Enumeration ppse = parentParmsVector.elements();
                    //loop through steps
                    while( ppse.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)ppse.nextElement();
                        if(aHWNode.name.equals("Steps")){
                            parentStepsNode = aHWNode;
                        }
                    }
                }
                
                
            } catch (RemoteException ex) {
                logger.error("persistStep() : Step could not be linked to its Parent Step!",ex);
            }
            
        }
        if(parentStepsNode!=null){
            
            Vector<String> oldList = getVectorFromString(parentStepsNode.limits,true);
            boolean exists = false;
            for(String aStrategyStepString : oldList){
                if(aStrategyStepString.equalsIgnoreCase(aBBSStep.getName())){
                    exists=true;
                }
            }
            if(!exists){
                oldList.add(aBBSStep.getName());
            }
            String newList = this.getStringFromVector(oldList,true);
            if(!parentStepsNode.limits.equals(newList)){
                parentStepsNode.limits=newList;
                try{
                    SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(parentStepsNode);
                } catch (RemoteException ex) {
                    logger.error("persistStep() : Step could not be linked to its Parent Step/Strategy!",ex);
                }
            }
        }
        
    }
    
    private void deleteStep(BBSStep aStep){
        
    }
    
    public boolean stepExists(BBSStep aStep){
        return false;
    }
    
    private Vector<String> getVectorFromString(String theList,boolean removeQuotes) {
        Vector<String> listItems = new Vector<String>();
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1,aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0,aList.length()-1);
        }
        if(!aList.equals("")){
            String[] aS=aList.split(",");
            for (int i=0; i< aS.length;i++) {
                if(removeQuotes){
                    listItems.add(aS[i].substring(1,aS[i].length()-1));
                }else{
                    listItems.add(aS[i]);
                }
            }
        }
        return listItems;
    }
    private String getStringFromVector(Vector<String> aStringVector,boolean createQuotes) {
        String aList="[";
        if (aStringVector.size() > 0) {
            int i = 0;
            for (String aString : aStringVector){
                if(i>0) aList+= ",";
                if(createQuotes){
                    aList += "\"";
                }
                aList += aString;
                if(createQuotes){
                    aList += "\"";
                }
                i++;
            }
        }
        aList+="]";
        return aList;
    }
    private Vector retrieveChildDataForNode(jOTDBnode aNode){
        Vector HWchilds = new Vector();
        try {
            HWchilds = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
        } catch (RemoteException ex) {
            logger.error("Error during retrieveChildDataForNode!", ex);
        }
        return HWchilds;
    }
    private jOTDBnode getStrategyStepsNode(jOTDBnode stepContainerNode){
        jOTDBnode strategyStepsParameter=null;
        try {
            Vector HWchilds = null;
            HWchilds = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(stepContainerNode.treeID(), stepContainerNode.parentID(), 2);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                strategyStepsParameter=null;
                //retrieving Strategy.Steps
                if (aHWNode.leaf && aHWNode.name.equals("Steps")) {
                    strategyStepsParameter = aHWNode;
                    logger.trace("Strategy Steps defined :"+strategyStepsParameter.limits);
                    break;
                }
            }
        } catch (RemoteException ex) {
            logger.error("Error during getStrategyStepsNode()!", ex);
        }
        return strategyStepsParameter;
    }
    private synchronized jOTDBnode getStepContainerNode(){
        return this.stepContainerNode;
    }
    
    private void setStepContainerNode(jOTDBnode rootNode){
        this.stepContainerNode = rootNode;
    }
    
    private int getComponentForNode(String nodeName){
        
        int returnId = 0;
        try {
            if(this.OTDBcomponentCache==null){
                OTDBcomponentCache = SharedVars.getOTDBrmi().getRemoteMaintenance().getComponentList("%",false);
            }
            Enumeration ce = OTDBcomponentCache.elements();
            while (ce.hasMoreElements()){
                jVICnodeDef aDef = (jVICnodeDef)ce.nextElement();
                if(aDef.name.equals(nodeName)){
                    returnId = aDef.nodeID();
                }
            }
        } catch (RemoteException ex) {
            logger.error("Could not load component node for node "+nodeName+" !",ex);
        }
        return returnId;
    }
}
