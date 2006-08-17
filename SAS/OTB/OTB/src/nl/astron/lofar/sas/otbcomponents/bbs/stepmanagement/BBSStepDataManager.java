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
    
    //get the step data that is persisted
    public BBSStepData getStepData(String name){
        BBSStepData returnStep = null;
        returnStep = stepsCollection.get(name);
        if(returnStep == null){
            returnStep = new BBSStepData();
            stepsCollection.put(name,returnStep);
        }
        return returnStep;
    }
    public BBSStepData getInheritedStepData(BBSStep aStep){
        return getInheritedStepData(aStep,null);
    }
    
    //get the inherited step data
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
            //add other values
            if(aStep.hasParentStep()){
                getInheritedStepData(aStep.getParentStep(),returnData);
            }
        }
        return returnData;
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
            SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(parentStepsNode);
        } catch (RemoteException ex) {
            logger.error("persistStep() : Step could not be linked to its Parent Step/Strategy!",ex);
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
        //another iteration to collect operation type attributes
        if(stepDataObject.getOperationName() !=null){
            
            Vector HWchilds2 = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(parentOTDBnode.treeID(), parentOTDBnode.nodeID(), 1);
            // get all the params per child
            Enumeration e2 = HWchilds2.elements();
            while( e2.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e2.nextElement();
                
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
                if(currentDataForStep.getOperationName() != null && !currentDataForStep.getOperationName().equals("")){
                    stepTemplateStepOperationNodeId = this.getComponentForNode(currentDataForStep.getOperationName());
                }
                
                if(stepTemplateNodeId!=0){
                    //copy the template step tree
                    int newStepNodeID = SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateNodeId,stepContainerNode.treeID(),stepContainerNode.nodeID(),aBBSStep.getName());
                    
                    //fetch the generated node as the step node
                    jOTDBnode newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    
                    //add the subcomponents for the step as well (correlation, integration, baselines, and operation if needed)
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateBaselinesNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateCorrelationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateIntegrationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    if(stepTemplateStepOperationNodeId != -1){
                        Vector someThing = SharedVars.getOTDBrmi().getRemoteMaintenance().getComponentList(currentDataForStep.getOperationName(),true);
                        SharedVars.getOTDBrmi().getRemoteMaintenance().addComponent(stepTemplateStepOperationNodeId,stepContainerNode.treeID(),newStepNode.nodeID(),"");
                    }
                    //update the node name
                    newStepNode = SharedVars.getOTDBrmi().getRemoteMaintenance().getNode(stepContainerNode.treeID(),newStepNodeID);
                    Vector stepParametersVector = retrieveChildDataForNode(newStepNode);
                    Enumeration spe = stepParametersVector.elements();
                    
                    while( spe.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)spe.nextElement();
                        
                        //do all BBS Step parameters
                        
                        //sources
                        if(aHWNode.name.equals("Sources")){
                            if ( currentDataForStep.getSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getSources(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //extra sources
                        else if(aHWNode.name.equals("ExtraSources")){
                            if ( currentDataForStep.getExtraSources() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getExtraSources(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        
                        //output data column
                        else if(aHWNode.name.equals("OutputData")){
                            if ( currentDataForStep.getOutputDataColumn() != null){
                                aHWNode.limits = currentDataForStep.getOutputDataColumn();
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //instrument data model
                        else if(aHWNode.name.equals("InstrumentModel")){
                            if ( currentDataForStep.getInstrumentModel() != null){
                                aHWNode.limits = this.getStringFromVector(currentDataForStep.getInstrumentModel(),true);
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
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
                                    if ( currentDataForStep.getIntegrationTime() != -1.0){
                                        aCENode.limits = ""+currentDataForStep.getIntegrationTime();
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Frequency
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Freq")) {
                                    if ( currentDataForStep.getIntegrationFrequency() != -1.0){
                                        aCENode.limits = ""+currentDataForStep.getIntegrationFrequency();
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Integration are present, delete this node as well
                            if(presentParams==0){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
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
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Selection
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Selection")) {
                                    if ( currentDataForStep.getCorrelationSelection() != null){
                                        aCENode.limits = currentDataForStep.getCorrelationSelection();
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Correlation are present, delete this node as well
                            if(presentParams==0){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
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
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                    
                                    //Frequency
                                    
                                } else if (aCENode.leaf && aCENode.name.equals("Station2")) {
                                    if ( currentDataForStep.getStation2Selection() != null){
                                        aCENode.limits = getStringFromVector(currentDataForStep.getStation2Selection(),true);
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Integration are present, delete this node as well
                            if(presentParams==0){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //Operation
                        
                        else if(aHWNode.name.equals("Operation")){
                            if ( currentDataForStep.getOperationName() != null){
                                aHWNode.limits = String.valueOf(currentDataForStep.getOperationName()).toUpperCase();
                                SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aHWNode);
                            }else{
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
                            }
                        }
                        //persist all the specific operation attributes
                        else if (!aHWNode.leaf && aHWNode.name.equals(currentDataForStep.getOperationName())) {
                            Vector attributeParms = this.retrieveChildDataForNode(aHWNode);
                            int presentParams = 0;
                            Enumeration ce = attributeParms.elements();
                            while( ce.hasMoreElements()  ) {
                                jOTDBnode aCENode = (jOTDBnode)ce.nextElement();
                                
                                String toBeInsertedValue = currentDataForStep.getOperationAttribute(LofarUtils.keyName(aCENode.name));
                                //direct parameter of Operation
                                if (aCENode.leaf ){
                                    if ( toBeInsertedValue != null){
                                        aCENode.limits = toBeInsertedValue;
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCENode);
                                        presentParams++;
                                    }else{
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
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
                                            SharedVars.getOTDBrmi().getRemoteMaintenance().saveNode(aCSENode);
                                            presentSubParams++;
                                        }else{
                                            SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCSENode);
                                        }
                                    }
                                    //no params inside Operation folder are present, delete this node as well
                                    if(presentSubParams==0){
                                        SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aCENode);
                                    }
                                }
                            }
                            //no params inside Operation are present, delete this node as well
                            if(presentParams==0){
                                SharedVars.getOTDBrmi().getRemoteMaintenance().deleteNode(aHWNode);
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
        //persist any child steps
        
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
