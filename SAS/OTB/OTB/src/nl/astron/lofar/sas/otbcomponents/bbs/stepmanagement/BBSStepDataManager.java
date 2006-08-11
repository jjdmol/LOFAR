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
import java.util.HashSet;
import java.util.Vector;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
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
    private BBSStrategy theStrategy = null;
    private static HashMap<String,BBSStepData> stepsCollection = null;
    private static HashSet<BBSStep> stepStructureCollection = null;
    
    /** Creates a new instance of BBSStepDataManager */
    private BBSStepDataManager() {
        stepsCollection = new HashMap<String,BBSStepData> ();
        stepStructureCollection = new HashSet<BBSStep>();
    }
    
    public static synchronized BBSStepDataManager getInstance(){
        if(instance==null){
            instance = new BBSStepDataManager();
        }
        return instance;
    }
    
    
    public void setStepContainerNode(jOTDBnode rootNode){
        this.stepContainerNode = rootNode;
    }
    
    public Vector<String> getStepNames(){
        Vector<String> returnVector = new Vector<String>();
        for(String aStep : stepsCollection.keySet()){
            returnVector.add(aStep);
        }
        return returnVector;
    }
    
    public BBSStep getBBSStep(BBSStep newParent,String name){
        BBSStep returnStep = null;
        for(BBSStep aStep : stepStructureCollection){
            if(aStep.getName().equals(name)){
                returnStep = aStep.clone();
                returnStep.setParentStep(newParent);
            }
        }
        return returnStep;
    }
    
    public void assureBBSStepIsInCollection(BBSStep aStep){
        boolean exists = false;
        for(BBSStep anExistingStep : stepStructureCollection){
            if(anExistingStep.getName().equals(aStep.getName())){
                exists = true;
            }
        }
        if(!exists){            
            stepStructureCollection.add(aStep);
        }else{
            BBSStep currentStep = null;
            for(BBSStep anExistingStep : stepStructureCollection){
                if(anExistingStep.getName().equals(aStep.getName())){
                    currentStep = anExistingStep;
                }
            }
            if(currentStep!= null){
                stepStructureCollection.remove(currentStep);
                stepStructureCollection.add(aStep);
            }
        }
    }    
    public boolean stepExists(String name){
        boolean exists = false;
        for(String existingStep : this.stepsCollection.keySet()){
            if(existingStep.equalsIgnoreCase(name)){
                exists=true;
            }
        }
        return exists;
    }
    public BBSStepData getStepData(String name){
        BBSStepData returnStep = null;
        for(String aStep : stepsCollection.keySet()){
            if(aStep.equalsIgnoreCase(name)){
                returnStep = stepsCollection.get(name);
            }
        }
        if(returnStep == null){
            returnStep = new BBSStepData();
            stepsCollection.put(name,returnStep);
        }
        return returnStep;
    }
    
    public BBSStrategy getStrategy(){
        if(theStrategy ==null){
            generateStrategyFromOTDB();
        }
        return theStrategy;
    }
    
    public void generateStrategyFromOTDB(){
        
        //clear the steps collection to make sure only steps are in there that are present in the OTDB
        this.stepsCollection.clear();
        
        theStrategy = new BBSStrategy();
        
        //Fetch the step names that are mentioned in the strategy tree (Strategy.Steps)
        jOTDBnode rootNode = this.getStepContainerNode();
        jOTDBnode strategyStepsParameter = this.getStrategyStepsNode(rootNode);
        
        if(strategyStepsParameter!=null){
            //retrieve the step names mentioned in the strategy steps parameter (Strategy.Steps)
            Vector<String> strategySteps = this.getVectorFromString(strategyStepsParameter.limits,true);
            
            if(strategySteps.size()>0){
                Vector stepsVector;
                try {
                    stepsVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.nodeID(), 1);
                    //loop through steps
                    for(String aStep : strategySteps){
                        Enumeration se = stepsVector.elements();
                        while( se.hasMoreElements()  ) {
                            jOTDBnode aHWNode = (jOTDBnode)se.nextElement();
                            //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
                            if (aHWNode.name.equals(aStep)) {
                                //Create a new step and build it (with its substeps as well)
                                BBSStep newStep = new BBSStep(aHWNode.name);
                                newStep.setParentStep(null);
                                buildStep(newStep,aHWNode);
                                theStrategy.addChildStep(newStep);
                                this.assureBBSStepIsInCollection(newStep);
                                logger.trace("Strategy Step defined : "+newStep.getName());
                            }
                        }
                    }
                } catch (RemoteException ex) {
                    logger.error("Strategy Steps could not be defined! ",ex);
                }
            }
        }
    }
    
    public void persistStrategy(){
        deleteAllSteps();
        
        for(BBSStep aStep : theStrategy.getChildSteps()){
            persistStep(aStep);
        }
        
    }
    
    private void buildStep(BBSStep parentNode, jOTDBnode parentOTDBnode) throws RemoteException{
        
        //create a StepData object to fill the variables
        BBSStepData stepDataObject = getStepData(parentNode.getName());
        
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
                if(!aHWNode.limits.equals("")){
                    stepDataObject.setSources(this.getVectorFromString(aHWNode.limits,true));
                }
            } else if (aHWNode.leaf && aHWNode.name.equals("ExtraSources")) {
                if(!aHWNode.limits.equals("")){
                    stepDataObject.setExtraSources(this.getVectorFromString(aHWNode.limits,true));
                }
            } else if (aHWNode.leaf && aHWNode.name.equals("InstrumentModel")) {
                if(!aHWNode.limits.equals("")){
                    stepDataObject.setInstrumentModel(this.getVectorFromString(aHWNode.limits,true));
                }
            } else if (aHWNode.leaf && aHWNode.name.equals("OutputData")) {
                if(!aHWNode.limits.equals("")){
                    stepDataObject.setOutputDataColumn(aHWNode.limits);
                }
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
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setCorrelationSelection(aHWNode.limits);
                        }
                    } else if (aHWNode.leaf && aHWNode.name.equals("Type")) {
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setCorrelationType(this.getVectorFromString(aHWNode.limits,true));
                        }
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Baselines")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aHWNode.leaf && aHWNode.name.equals("Station1")) {
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setStation1Selection(this.getVectorFromString(aHWNode.limits,true));
                        }
                    } else if (aHWNode.leaf && aHWNode.name.equals("Station2")) {
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setStation2Selection(this.getVectorFromString(aHWNode.limits,true));
                        }
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Integration")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aHWNode.leaf && aHWNode.name.equals("Time")) {
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setIntegrationTime(Double.parseDouble(aHWNode.limits));
                        }
                    } else if (aHWNode.leaf && aHWNode.name.equals("Freq")) {
                        if(!aHWNode.limits.equals("")){
                            stepDataObject.setIntegrationFrequency(Double.parseDouble(aHWNode.limits));
                        }
                    }
                }
            }
        }
        
        if(strategyStepsParameter!=null){
            //retrieve the step names mentioned in the steps parameter (XXX.Steps)
            Vector<String> strategySteps = this.getVectorFromString(strategyStepsParameter.limits,true);
            
            if(strategySteps.size()>0){
                //Get all the steps present in the BBS Step Container
                Vector stepsVector = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.parentID(), 1);
                
                for(String aStep : strategySteps){
                    Enumeration se = stepsVector.elements();
                    //loop through steps
                    while( se.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)se.nextElement();
                        //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
                        if (!aHWNode.leaf && aHWNode.name.equals(aStep)) {
                            //Create a new step and build it (with its substeps as well)
                            BBSStep newStep = new BBSStep(aHWNode.name);
                            //build its childsteps recursively
                            buildStep(newStep,aHWNode);
                            parentNode.addChildStep(newStep);
                            this.assureBBSStepIsInCollection(newStep);
                            logger.trace("Step defined : "+newStep.getName());
                        }
                    }
                }
            }
        }
    }
    
    private void persistStep(BBSStep aBBSStep){
        jOTDBnode stepsNode = this.getStepContainerNode();
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
            
            if(aHWNode.name.equals("DefaultBBSStep")){
                try {
                    SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                } catch (RemoteException ex) {
                    logger.debug("PersistStep(): Unable to delete the default Template Step from the Step Container: "+aBBSStep.getName());
                }
            }
            //limiting the search for steps that are mentioned in the strategy steps parameter (Strategy.Steps)
            if (!aHWNode.leaf && aHWNode.name.equals(aBBSStep.getName())) {
                existingStepNode = aHWNode;
                logger.trace("PersistStep(): Step found in Step Container: "+aBBSStep.getName());
            }
        }
        
        if(existingStepNode == null){
            try {
                //new step has to be generated and persisted.
                
                //fetch list of components that contains the BBS Step tree items
                int stepTemplateNodeId = this.getComponentForNode("DefaultBBSStep");
                int stepTemplateCorrelationNodeId = this.getComponentForNode("Correlation");
                int stepTemplateBaselinesNodeId = this.getComponentForNode("Baselines");
                int stepTemplateIntegrationNodeId = this.getComponentForNode("Integration");
                
                if(stepTemplateNodeId!=0){
                    //copy the template step tree
                    int newStepNodeID = SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateNodeId,stepContainerNode.treeID(),stepContainerNode.nodeID(),aBBSStep.getName());
                    
                    //fetch the generated node as the step node
                    jOTDBnode newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    
                    //add the subcomponents for the step as well (correlation, integration and baselines)
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateBaselinesNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateCorrelationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateIntegrationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    
                    //update the node name
                    newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    Vector stepParametersVector = retrieveChildDataForNode(newStepNode);
                    Enumeration spe = stepParametersVector.elements();
                    
                    BBSStepData currentDataForStep = getStepData(aBBSStep.getName());
                    
                    
                    while( spe.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)spe.nextElement();
                        
                        //do all BBS Step parameters
                        
                        //sources
                        if(aHWNode.name.equals("Sources")){
                            if(aBBSStep.getParentStep() != null){
                                BBSStepData currentDataForParentStep = this.getStepData(aBBSStep.getParentStep().getName());
                                Vector<String> newString = currentDataForStep.getSources();
                                if(currentDataForStep.getSources() != null && currentDataForParentStep.getSources() != null){
                                    if(currentDataForStep.getSources().equals(currentDataForParentStep.getSources())) {
                                        newString = null;
                                    }
                                }
                                currentDataForStep.setSources(newString);
                            }
                            if ( currentDataForStep.getSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getSources(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //extra sources
                        else if(aHWNode.name.equals("ExtraSources")){
                            if(aBBSStep.getParentStep() != null){
                                BBSStepData currentDataForParentStep = this.getStepData(aBBSStep.getParentStep().getName());
                                Vector<String> newString = currentDataForStep.getExtraSources();
                                if(currentDataForStep.getExtraSources() != null && currentDataForParentStep.getExtraSources() != null){
                                    if(currentDataForStep.getExtraSources().equals(currentDataForParentStep.getExtraSources())) {
                                        newString = null;
                                    }
                                }
                                currentDataForStep.setExtraSources(newString);
                            }
                            if ( currentDataForStep.getExtraSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getExtraSources(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        
                        //output data column
                        else if(aHWNode.name.equals("OutputData")){
                            if(aBBSStep.getParentStep() != null){
                                BBSStepData currentDataForParentStep = this.getStepData(aBBSStep.getParentStep().getName());
                                String newString = currentDataForStep.getOutputDataColumn();
                                if(currentDataForStep.getOutputDataColumn() != null && currentDataForParentStep.getOutputDataColumn() != null){
                                    if(currentDataForStep.getOutputDataColumn().equals(currentDataForParentStep.getOutputDataColumn())) {
                                        newString = null;
                                    }
                                }
                                currentDataForStep.setOutputDataColumn(newString);
                            }
                            if ( currentDataForStep.getOutputDataColumn() != null){
                                aHWNode.limits = currentDataForStep.getOutputDataColumn();
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //instrument data model
                        else if(aHWNode.name.equals("InstrumentModel")){
                            if(aBBSStep.getParentStep() != null){
                                BBSStepData currentDataForParentStep = this.getStepData(aBBSStep.getParentStep().getName());
                                Vector<String> newString = currentDataForStep.getInstrumentModel();
                                if(currentDataForStep.getInstrumentModel() != null && currentDataForParentStep.getInstrumentModel() != null){
                                    if(currentDataForStep.getInstrumentModel().equals(currentDataForParentStep.getInstrumentModel())) {
                                        newString = null;
                                    }
                                }
                                currentDataForStep.setInstrumentModel(newString);
                            }
                            if ( currentDataForStep.getInstrumentModel() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getInstrumentModel(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
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
            
            //determine the Parent children as defined by the BBS Step Parent object
            Vector<BBSStep> currentParentChildren = new Vector<BBSStep>();
            if(isStrategyStep){
                currentParentChildren = BBSStepDataManager.getInstance().getStrategy().getChildSteps();
            }else{
                currentParentChildren = aBBSStep.getParentStep().getChildSteps();
            }
            Vector<String> currentParentChildrenList = new Vector<String>();
            for(BBSStep someStep : currentParentChildren){
                currentParentChildrenList.add(someStep.getName());
            }
            
            Vector<String> oldList = getVectorFromString(parentStepsNode.limits,true);
            
            if(!oldList.equals(currentParentChildrenList)){
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
        
        //do the child steps
        
        for(BBSStep childStep : aBBSStep.getChildSteps()){
            persistStep(childStep);
        }
        
    }
    
    private void deleteAllSteps(){
        
        jOTDBnode parentStepsNode = this.getStrategyStepsNode(this.getStepContainerNode());
        
        //remove all references to steps from the strategy
        try{
            parentStepsNode.limits="[]";
            SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(parentStepsNode);
        } catch (RemoteException ex) {
            logger.error("deleteAllSteps() : Strategy steps could not be cleaned!",ex);
        }
        
        //remove all steps
        Vector parentParmVector = this.retrieveChildDataForNode(this.getStepContainerNode());
        Enumeration ppe = parentParmVector.elements();
        //loop through steps and delete the step that matches with the step provided
        while( ppe.hasMoreElements()  ) {
            jOTDBnode aHWNode = (jOTDBnode)ppe.nextElement();
            try {
                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
            } catch (RemoteException ex) {
                logger.error("deleteAllSteps() : Step could not be deleted!",ex);
            }
        }
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
    
    private synchronized jOTDBnode getStepContainerNode(){
        return this.stepContainerNode;
    }
}
