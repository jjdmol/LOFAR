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
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 * The BBSStepDataManager manages the BBS Step tree, its persistence and its correctness.
 *
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
    private HashMap<String,BBSStepData> stepsCollection = null;
    private HashSet<BBSStep> stepStructureCollection = null;
    
    /**
     * Creates a new instance of BBSStepDataManager, protected by a singleton pattern
     */
    private BBSStepDataManager() {
        stepsCollection = new HashMap<String,BBSStepData> ();
        stepStructureCollection = new HashSet<BBSStep>();
    }
    /**
     * Returns a static instance of the BBSStepDataManager class.
     *
     * @return the BBSStepDataManager instance
     */
    public static synchronized BBSStepDataManager getInstance(){
        if(instance==null){
            instance = new BBSStepDataManager();
        }
        return instance;
    }
    /**
     * This method will allow the BBSStepDataManager to locate the proper BBS tree in the OTDB
     * to locate the BBS Strategy and Steps.
     * This method has to be used/called before any other methods will work.
     *
     * @parm rootNode the BBS.Step 'Step Container' OTDBnode.
     */
    public synchronized void setStepContainerNode(jOTDBnode rootNode){
        BBSStepDataManager.stepContainerNode = rootNode;
    }
    /**
     * Returns the unique names of every Step currently being managed.
     *
     * @return the unique names of all the steps being managed.
     */
    public synchronized Vector<String> getStepNames(){
        Vector<String> returnVector = new Vector<String>();
        for(String aStep : stepsCollection.keySet()){
            returnVector.add(aStep);
        }
        return returnVector;
    }
    /**
     * Returns a BBSStep with a given name, and attaches it to a given parent
     * or the BBS Strategy.<br><br>
     * If a BBSStep object already exists with the same name in the managed BBSStep cache,
     * a clone of that existing step is returned.
     * A new BBSStep is created should that not be the case.
     * It will also be added to the cache of BBSSteps being managed.
     *
     * @parm newParent the BBSStep parent to attach the new BBSStep to, or null if the BBSStep is a BBS Strategy step.
     * @parm name the name to be given to the BBS Step.
     * @return a BBSStep object with the given name and association with the given parent BBSStep
     */
    public synchronized BBSStep getBBSStep(BBSStep newParent,String name){
        BBSStep returnStep = null;
        //check if the step name is present in the step cache
        for(BBSStep aStep : stepStructureCollection){
            if(aStep.getName().equals(name)){
                returnStep = aStep.clone();
                returnStep.setParentStep(newParent);
            }
        }
        //add a new step as the requested step name was not found in the cache
        if(returnStep==null){
            returnStep = new BBSStep(name);
            returnStep.setParentStep(newParent);
            stepStructureCollection.add(returnStep);
        }
        return returnStep;
    }
    /**
     * This helper method assures that a BBSStep is present in the BBSStep cache.
     * This updates the cache if the given BBSStep with its Step Name does not yet exist,
     * or updates the cached BBSStep if it does already exist.
     * (Step Name is used to identify the BBSStep)
     *
     * @parm aStep the BBSStep object to check in the BBSStep cache.
     */
    public synchronized void assureBBSStepIsInCollection(BBSStep aStep){
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
    
    /**
     * Answers if a BBSStep with the given name is already available in the BBSStep cache.
     *
     * @parm name the name of the BBS Step to check for
     * @return <i>true</i> - if the step is already defined, <i>false</i> - if the step is not yet defined
     */
    public synchronized boolean stepExists(String name){
        boolean exists = false;
        for(String existingStep : this.stepsCollection.keySet()){
            if(existingStep.equalsIgnoreCase(name)){
                exists=true;
            }
        }
        return exists;
    }
    /**
     * Retrieves the BBSStepData object for a given Step name.
     * This ensures that Step data is current for all BBS Steps in the tree.
     *
     * @parm name the BBS Step name to fetch the data for
     * @return the BBSStepData object for the given name. If no step data was available, null is returned.
     */
    public synchronized BBSStepData getStepData(String name){
        BBSStepData returnStep = null;
        returnStep = stepsCollection.get(name);
        if(returnStep == null){
            returnStep = new BBSStepData();
            stepsCollection.put(name,returnStep);
        }
        return returnStep;
    }
    /**
     * Retrieves all inherited step data in a BBSStepData object for a given BBS Step.
     * The data for the step itself is ignored, so only data from parent steps is present.
     *
     * @parm aStep the BBSStep to fetch all inherited data for.
     * @return the BBSStepData object containing all inherited data for the given BBSStep, ignoring its own data.
     */
    public synchronized BBSStepData getInheritedStepData(BBSStep aStep){
        return getInheritedStepData(aStep,null);
    }
    /**
     * Returns a static BBSStrategy object that is the entry point to the entire BBS Step tree.
     * A new BBSStrategy object will be generated using the OTDB if the BBSStrategy object does not yet exist.
     *
     * @return the BBSStrategy object
     */
    public synchronized BBSStrategy getStrategy(){
        if(theStrategy ==null){
            generateStrategyFromOTDB();
        }
        return theStrategy;
    }
    /**
     * (Re)generates the BBSStrategy and all BBS Steps using data active in the OTDB BBS template tree.
     * This action reverts any changes made to the BBS Strategy and steps since the last persistStrategy() call.
     */
    public synchronized void generateStrategyFromOTDB(){
        
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
                    stepsVector = OtdbRmi.getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.nodeID(), 1);
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
    /**
     * Persists the BBS Strategy and all BBS Steps therein to the OTDB template tree.
     */
    public synchronized void persistStrategy(){
        deleteAllSteps();
        
        for(BBSStep aStep : theStrategy.getChildSteps()){
            persistStep(aStep);
        }
        
        //update references to the child steps in the strategy
        
        jOTDBnode parentStepsNode = this.getStrategyStepsNode(this.getStepContainerNode());
        
        //determine the Parent children as defined by the BBS Step Parent object
        Vector<BBSStep> currentParentChildren = theStrategy.getChildSteps();
        
        Vector<String> currentParentChildrenList = new Vector<String>();
        for(BBSStep someStep : currentParentChildren){
            currentParentChildrenList.add(someStep.getName());
        }
        String newList = this.getStringFromVector(currentParentChildrenList,true);
        parentStepsNode.limits=newList;
        try{
            OtdbRmi.getRemoteMaintenance().saveNode(parentStepsNode);
        } catch (RemoteException ex) {
            logger.error("persistStep() : Step could not be linked to its Parent Step/Strategy!",ex);
        }
    }
    /**
     * Helper method to recursively build a BBSStep object and all child BBSStep objects from a Step node in the OTDB Step container.
     *
     * @parm parentNode the BBSStep object to build.
     * @parm parentOTDBnode the OTDB node pointer to the Step node in BBS.Step.***
     */
    private void buildStep(BBSStep parentNode, jOTDBnode parentOTDBnode) throws RemoteException{
        
        //create a StepData object to fill the variables
        BBSStepData stepDataObject = getStepData(parentNode.getName());
        
        //Add substeps
        jOTDBnode strategyStepsParameter=null;
        //retrieve 1 levels of parameters to locate Step.XXX.Steps
        
        Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.nodeID(), 1);
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
                stepDataObject.setOutputDataColumn(aHWNode.limits);
            }
            //Set the following values
            else if (aHWNode.leaf && aHWNode.name.equals("Operation")) {
                if(!aHWNode.limits.equals("")){
                    String value = String.valueOf(aHWNode.limits).toLowerCase();
                    String firstChar = value.substring(0,1);
                    firstChar = firstChar.toUpperCase();
                    value = firstChar + value.substring(1,value.length());
                    stepDataObject.setOperationName(value);
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Correlation")) {
                Vector correlationParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = correlationParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aCENode.leaf && aCENode.name.equals("Selection")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setCorrelationSelection(aCENode.limits);
                        }
                    } else if (aCENode.leaf && aCENode.name.equals("Type")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setCorrelationType(this.getVectorFromString(aCENode.limits,true));
                        }
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Baselines")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aCENode.leaf && aCENode.name.equals("Station1")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setStation1Selection(this.getVectorFromString(aCENode.limits,true));
                        }
                    } else if (aCENode.leaf && aCENode.name.equals("Station2")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setStation2Selection(this.getVectorFromString(aCENode.limits,true));
                        }
                    }
                }
            } else if (!aHWNode.leaf && aHWNode.name.equals("Integration")) {
                Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                
                Enumeration ce = baselinesParms.elements();
                while( ce.hasMoreElements()  ) {
                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                    
                    if (aCENode.leaf && aCENode.name.equals("Time")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setIntegrationTime(Double.parseDouble(aCENode.limits));
                        }
                    } else if (aCENode.leaf && aCENode.name.equals("Freq")) {
                        if(!aCENode.limits.equals("")){
                            stepDataObject.setIntegrationFrequency(Double.parseDouble(aCENode.limits));
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
                Vector stepsVector = OtdbRmi.getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.parentID(), 1);
                
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
        //another iteration to collect operation type attributes
        if(stepDataObject.getOperationName() !=null){
            
            Vector HWchilds2 = OtdbRmi.getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.nodeID(), 1);
            // get all the params per child
            Enumeration e2 = HWchilds2.elements();
            while( e2.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e2.nextElement();
                /*
                 * This operation only supports operation node attributes of up to two levels deep.
                 *
                 * For instance: Solve.DomainSize.Freq is supported
                 *               Predict.DomainSize.Integration.Freq is NOT supported...
                 */
                if (!aHWNode.leaf && aHWNode.name.equals(stepDataObject.getOperationName())){
                    Vector operationParms = this.retrieveChildDataForNode(aHWNode);
                    
                    Enumeration ce = operationParms.elements();
                    while( ce.hasMoreElements()  ) {
                        jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                        
                        if (aCENode.leaf){
                            if(!aCENode.limits.equals("")){
                                stepDataObject.addOperationAttribute(LofarUtils.keyName(aCENode.name),aCENode.limits);
                            }
                        } else {
                            Vector operationSubParms = this.retrieveChildDataForNode(aCENode);
                            
                            Enumeration cse = operationSubParms.elements();
                            while( cse.hasMoreElements()  ) {
                                jOTDBnode aCSENode = (jOTDBnode)cse.nextElement();
                                if (aCSENode.leaf){
                                    if(!aCSENode.limits.equals("")){
                                        stepDataObject.addOperationAttribute(LofarUtils.keyName(aCENode.name)+"."+LofarUtils.keyName(aCSENode.name),aCSENode.limits);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    /**
     * Helper method that persists a BBSStep object with its data to the OTDB template tree.
     *
     * @parm aBBSStep the BBSStep object to persist.
     */
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
            
            //delete the standard bbs step found in the template tree
            if(aHWNode.name.equals("DefaultBBSStep")){
                try {
                    OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
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
        //if the step to be persisted was already present in the OTDB,
        //it is assumed it was persisted by a previous persistStep() call...
        if(existingStepNode == null){
            try {
                //new step has to be generated and persisted.
                BBSStepData currentDataForStep = getStepData(aBBSStep.getName());
                
                //fetch list of components that contains the BBS Step tree items
                int stepTemplateNodeId = this.getComponentForNode("DefaultBBSStep");
                int stepTemplateCorrelationNodeId = this.getComponentForNode("Correlation");
                int stepTemplateBaselinesNodeId = this.getComponentForNode("Baselines");
                int stepTemplateIntegrationNodeId = this.getComponentForNode("Integration");
                int stepTemplateStepOperationNodeId = -1;
                if(currentDataForStep.getOperationName() != null && currentDataForStep.getOperationAttributes() != null &&
                        !currentDataForStep.getOperationName().equals("")){
                    stepTemplateStepOperationNodeId = this.getComponentForNode(currentDataForStep.getOperationName());
                }
                
                if(stepTemplateNodeId!=0){
                    //copy the template step tree
                    int newStepNodeID = OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateNodeId,stepContainerNode.treeID(),stepContainerNode.nodeID(),aBBSStep.getName());
                    
                    //fetch the generated node as the step node
                    jOTDBnode newStepNode = OtdbRmi.getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    
                    //add the subcomponents for the step as well (correlation, integration, baselines, and operation if needed)
                    OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateBaselinesNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateCorrelationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateIntegrationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    if(stepTemplateStepOperationNodeId != -1){
                        
                        int stepOperationNodeId = OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateStepOperationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                        
                        //collect components that are part of the operation type...
                        if(currentDataForStep.getOperationAttributes() != null){
                            Vector<String> toBeAddedSubComponents = new Vector<String>();
                            for(String someOperationAttribute : currentDataForStep.getOperationAttributes().keySet()){
                                String[] splitter = someOperationAttribute.split("[.]");
                                if(splitter.length>1){
                                    if(!toBeAddedSubComponents.contains(splitter[0])){
                                        toBeAddedSubComponents.add(splitter[0]);
                                    }
                                }
                            }
                            for(String someOperationAttribute : toBeAddedSubComponents){
                                int stepTemplateStepOperationTypeNodeId = this.getComponentForNode(someOperationAttribute);
                                OtdbRmi.getRemoteMaintenance().addComponent(stepTemplateStepOperationTypeNodeId,stepContainerNode.treeID(),stepOperationNodeId,"");
                            }
                        }
                    }
                    
                    newStepNode = OtdbRmi.getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    Vector stepParametersVector = retrieveChildDataForNode(newStepNode);
                    Enumeration spe = stepParametersVector.elements();
                    
                    while( spe.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)spe.nextElement();
                        
                        //do all BBS Step parameters
                        
                        //sources
                        if(aHWNode.name.equals("Sources")){
                            if ( currentDataForStep.getSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getSources(),true);
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //extra sources
                        else if(aHWNode.name.equals("ExtraSources")){
                            if ( currentDataForStep.getExtraSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getExtraSources(),true);
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        
                        //output data column
                        else if(aHWNode.name.equals("OutputData")){
                            if ( currentDataForStep.getOutputDataColumn() != null){
                                aHWNode.limits = currentDataForStep.getOutputDataColumn();
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //instrument data model
                        else if(aHWNode.name.equals("InstrumentModel")){
                            if ( currentDataForStep.getInstrumentModel() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getInstrumentModel(),true);
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        
                        //Integration
                        
                        else if (!aHWNode.leaf && aHWNode.name.equals("Integration")) {
                            Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                            int presentParams = 0;
                            Enumeration ce = baselinesParms.elements();
                            while( ce.hasMoreElements()  ) {
                                jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                                
                                //Time
                                
                                if (aCENode.leaf && aCENode.name.equals("Time")) {
                                    if ( currentDataForStep.getIntegrationTime() != -1){
                                        aCENode.limits = ""+currentDataForStep.getIntegrationTime();
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Frequency
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Freq")) {
                                    if ( currentDataForStep.getIntegrationFrequency() != -1){
                                        aCENode.limits = ""+currentDataForStep.getIntegrationFrequency();
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Integration are present, delete this node as well
                            if(presentParams==0){
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        
                        //Correlation
                        
                        else if (!aHWNode.leaf && aHWNode.name.equals("Correlation")) {
                            Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                            int presentParams = 0;
                            Enumeration ce = baselinesParms.elements();
                            while( ce.hasMoreElements()  ) {
                                jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                                
                                //Type
                                
                                if (aCENode.leaf && aCENode.name.equals("Type")) {
                                    if ( currentDataForStep.getCorrelationType() != null){
                                        aCENode.limits = this.getStringFromVector(currentDataForStep.getCorrelationType(),true);
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Selection
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Selection")) {
                                    if ( currentDataForStep.getCorrelationSelection() != null){
                                        aCENode.limits = currentDataForStep.getCorrelationSelection();
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Correlation are present, delete this node as well
                            if(presentParams==0){
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //Baseline Selection
                        
                        else if (!aHWNode.leaf && aHWNode.name.equals("Baselines")) {
                            Vector baselinesParms = this.retrieveChildDataForNode(aHWNode);
                            int presentParams = 0;
                            Enumeration ce = baselinesParms.elements();
                            while( ce.hasMoreElements()  ) {
                                jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                                
                                //Time
                                
                                if (aCENode.leaf && aCENode.name.equals("Station1")) {
                                    if ( currentDataForStep.getStation1Selection() != null){
                                        aCENode.limits = getStringFromVector(currentDataForStep.getStation1Selection(),true);
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Frequency
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Station2")) {
                                    if ( currentDataForStep.getStation2Selection() != null){
                                        aCENode.limits = getStringFromVector(currentDataForStep.getStation2Selection(),true);
                                        OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Baseline are present, delete this node as well
                            if(presentParams==0){
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //Operation
                        
                        else if(aHWNode.name.equals("Operation")){
                            if ( currentDataForStep.getOperationName() != null){
                                aHWNode.limits = String.valueOf(currentDataForStep.getOperationName()).toUpperCase();
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //persist all the specific operation attributes
                        else if (!aHWNode.leaf && aHWNode.name.equals(currentDataForStep.getOperationName())) {
                            int presentParams = 0;
                            if(currentDataForStep.getOperationAttributes()!=null){
                                Vector attributeParms = this.retrieveChildDataForNode(aHWNode);
                                Enumeration ce = attributeParms.elements();
                                while( ce.hasMoreElements()  ) {
                                    jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                                     /*
                                      * This operation only supports operation node attributes of up to two levels deep.
                                      *
                                      * For instance: Solve.DomainSize.Freq is supported
                                      *               Predict.DomainSize.Integration.Freq is NOT supported...
                                      */
                                    String toBeInsertedValue = currentDataForStep.getOperationAttribute(LofarUtils.keyName(aCENode.name));
                                    //direct parameter of Operation
                                    if (aCENode.leaf ){
                                        if ( toBeInsertedValue != null){
                                            aCENode.limits = toBeInsertedValue;
                                            OtdbRmi.getRemoteMaintenance().saveNode(aCENode);
                                            presentParams++;
                                        }else{
                                            OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                        }
                                        //parameter node inside Operation
                                    } else if (!aCENode.leaf){
                                        Vector attributeSubParms = this.retrieveChildDataForNode(aCENode);
                                        int presentSubParams = 0;
                                        Enumeration cse = attributeSubParms.elements();
                                        while( cse.hasMoreElements()  ) {
                                            jOTDBnode aCSENode = (jOTDBnode)cse.nextElement();
                                            String toBeInsertedSubValue = currentDataForStep.getOperationAttribute(LofarUtils.keyName(aCENode.name)+"."+LofarUtils.keyName(aCSENode.name));
                                            if ( toBeInsertedSubValue != null){
                                                aCSENode.limits = toBeInsertedSubValue;
                                                OtdbRmi.getRemoteMaintenance().saveNode(aCSENode);
                                                presentSubParams++;
                                            }else{
                                                OtdbRmi.getRemoteMaintenance().deleteNode(aCSENode);
                                            }
                                        }
                                        //no params inside Operation folder are present, delete this node as well
                                        if(presentSubParams==0){
                                            OtdbRmi.getRemoteMaintenance().deleteNode(aCENode);
                                        }
                                    }
                                }
                            }
                            //no params inside Operation are present, delete this node as well
                            if(presentParams==0){
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                            
                        }
                        //add other variables...
                        //add name pointers to the child steps
                        else if(aHWNode.name.equals("Steps")){
                            Vector<BBSStep> itsChildSteps = aBBSStep.getChildSteps();
                            if(itsChildSteps != null && itsChildSteps.size() > 0){
                                Vector<String> childStepNames = new Vector<String>();
                                for(BBSStep aChildStep : itsChildSteps){
                                    childStepNames.add(aChildStep.getName());
                                }
                                aHWNode.limits = this.getStringFromVector(childStepNames,true);
                                OtdbRmi.getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                    }
                    
                }
            } catch (RemoteException ex) {
                logger.error("PersistStep(): New step could not be generated using the DefaultBBSStep template as a component",ex);
            }
            
        }
        //persist any child steps
        
        for(BBSStep childStep : aBBSStep.getChildSteps()){
            persistStep(childStep);
        }
        
    }
    
    /**
     * Helper method that deletes all BBS Steps present in the OTDB template tree.
     */
    private void deleteAllSteps(){
        
        jOTDBnode parentStepsNode = this.getStrategyStepsNode(this.getStepContainerNode());
        
        //remove all references to steps from the strategy
        try{
            parentStepsNode.limits="[]";
            OtdbRmi.getRemoteMaintenance().saveNode(parentStepsNode);
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
                OtdbRmi.getRemoteMaintenance().deleteNode(aHWNode);
            } catch (RemoteException ex) {
                logger.error("deleteAllSteps() : Step could not be deleted!",ex);
            }
        }
    }
    /**
     * Helper method that recursively adds inherited data for a given BBS Step
     * to a worker object that should be complete with all needed data when
     * the bottom up BBS Step recursion is complete.
     *
     * @parm returnData the worker BBSStepData object that must be filled with data from the given BBSStep.
     * @parm aStep the BBSStep to check and get data from to add to the recursively built BBSStepData object
     * @return BBSStepData a (in)complete set of data to be processed further until the recursion is finished.
     */
    private BBSStepData getInheritedStepData(BBSStep aStep, BBSStepData returnData){
        if(returnData==null){
            returnData = new BBSStepData();
        }
        if(aStep!=null){
            
            BBSStepData itsStepData = new BBSStepData();
            if(aStep.hasParentStep()){
                itsStepData = getStepData(aStep.getParentStep().getName());
            }
            if(returnData.getStation1Selection()==null){
                if(itsStepData.getStation1Selection()!=null){
                    returnData.setStation1Selection(itsStepData.getStation1Selection());
                }
            }
            if(returnData.getStation2Selection()==null){
                if(itsStepData.getStation2Selection()!=null){
                    returnData.setStation2Selection(itsStepData.getStation2Selection());
                }
            }
            if(returnData.getSources()==null){
                if(itsStepData.getSources()!=null){
                    returnData.setSources(itsStepData.getSources());
                }
            }
            if(returnData.getExtraSources()==null){
                if(itsStepData.getExtraSources()!=null){
                    returnData.setExtraSources(itsStepData.getExtraSources());
                }
            }
            if(returnData.getInstrumentModel()==null){
                if(itsStepData.getInstrumentModel()!=null){
                    returnData.setInstrumentModel(itsStepData.getInstrumentModel());
                }
            }
            if(returnData.getIntegrationFrequency()==-1.0){
                if(itsStepData.getIntegrationFrequency()!=-1.0){
                    returnData.setIntegrationFrequency(itsStepData.getIntegrationFrequency());
                }
            }
            if(returnData.getIntegrationTime()==-1.0){
                if(itsStepData.getIntegrationTime()!=-1.0){
                    returnData.setIntegrationTime(itsStepData.getIntegrationTime());
                }
            }
            if(returnData.getCorrelationSelection()==null){
                if(itsStepData.getCorrelationSelection()!=null){
                    returnData.setCorrelationSelection(itsStepData.getCorrelationSelection());
                }
            }
            if(returnData.getCorrelationType()==null){
                if(itsStepData.getCorrelationType()!=null){
                    returnData.setCorrelationType(itsStepData.getCorrelationType());
                }
            }
            if(returnData.getOutputDataColumn()==null){
                if(itsStepData.getOutputDataColumn()!=null){
                    returnData.setOutputDataColumn(itsStepData.getOutputDataColumn());
                }
            }
            if(returnData.getOperationName()==null){
                if(itsStepData.getOperationName()!=null){
                    returnData.setOperationName(itsStepData.getOperationName());
                }
            }
            if(itsStepData.getOperationAttributes()!=null){
                for(String anAttribute : itsStepData.getOperationAttributes().keySet()){
                    if(!returnData.containsOperationAttribute(anAttribute) && itsStepData.getOperationAttribute(anAttribute) != null){
                        returnData.addOperationAttribute(anAttribute,itsStepData.getOperationAttribute(anAttribute));
                    }
                }
            }
            //add other values
            if(aStep.hasParentStep()){
                getInheritedStepData(aStep.getParentStep(),returnData);
            }
        }
        return returnData;
    }
    /**
     * Helper method that retrieves a Vector of strings out of a String representation thereof.
     *
     * @parm theList the String representation of a Vector to convert
     * @parm removeQuotes tells if quotes are/are not present in the String and should/should not be removed in the process.
     * @return Vector of Strings extrapolated from theList.
     */
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
    /**
     * Helper method that retrieves a String representation of a Vector of strings.
     *
     * @parm aStringVector the String Vector to convert to a String representation.
     * @parm createQuotes tells if quotes should/should not be added in the process.
     * @return String representation of aStringVector.
     */
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
    /**
     * Helper method that retrieves a Vector of jOTDBnode objects that are the child of aNode.
     *
     * @parm aNode the jOTDBnode to retrieve the child nodes for.
     * @return Vector of child jOTDBnode objects.
     */
    private Vector retrieveChildDataForNode(jOTDBnode aNode){
        Vector HWchilds = new Vector();
        try {
            HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
        } catch (RemoteException ex) {
            logger.error("Error during retrieveChildDataForNode!", ex);
        }
        return HWchilds;
    }
    /**
     * Helper method to fetch the BBS.Strategy.Steps jOTDBnode using the BBS.Step jOTDBnode as a guideline.
     *
     * @parm stepContainerNode the BBS.Step jOTDBnode
     * @return the BBS.Strategy.Steps jOTDBnode
     */
    private jOTDBnode getStrategyStepsNode(jOTDBnode stepContainerNode){
        jOTDBnode strategyStepsParameter=null;
        try {
            Vector HWchilds = null;
            HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(stepContainerNode.treeID(), stepContainerNode.parentID(), 2);
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
    /**
     * Helper method that retrieves the OTDB component ID using a node name to identify the component.
     *
     * @parm nodeName the OTDB node name to lookup the component ID for.
     * @return the component ID of the found component node, or 0 if no component was found.
     */
    private int getComponentForNode(String nodeName){
        
        int returnId = 0;
        try {
            if(BBSStepDataManager.OTDBcomponentCache==null){
                OTDBcomponentCache = OtdbRmi.getRemoteMaintenance().getComponentList("%",false);
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
    /**
     * Helper method that returns the BBS.Step 'Step Container' node.
     *
     * @return the BBS.Step 'Step Container' node.
     */
    private synchronized jOTDBnode getStepContainerNode(){
        return BBSStepDataManager.stepContainerNode;
    }
}
