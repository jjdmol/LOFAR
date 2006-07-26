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
import java.util.Vector;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import org.apache.log4j.Logger;

/**
 * @version $Id$
 * @created July 26, 2006, 10:04 AM
 * @author pompert
 */
public class BBSStepDataManager{
    
    private static BBSStepDataManager instance;
    private static Logger logger = Logger.getLogger(BBSStepDataManager.class);
    
    /** Creates a new instance of BBSStepDataManager */
    private BBSStepDataManager() {
        
    }
    
    public static synchronized BBSStepDataManager getInstance(){
        if(instance==null){
            instance = new BBSStepDataManager();
        }
        return instance;
    }
    
    public Vector<BBSStepNode> buildStepTree(BBSStepNode aRootNode, boolean buildStrategyStepTree) throws RemoteException{
        Vector<BBSStepNode> returnNode = new Vector<BBSStepNode>();
        
        //Fetch the step names that are mentioned in the strategy tree (Strategy.Steps)
        
        //The following otdbnode should be the Step container node (Step)
        jOTDBnode rootNode = aRootNode.getOTDBNode();
        
        jOTDBnode strategyStepsParameter=null;
        Vector HWchilds = null;
        if(buildStrategyStepTree){
            //retrieve 2 levels of parameters to locate Strategy.Steps
            //using the parent node of Step, which is the BBS root node itself
            HWchilds = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.parentID(), 2);
        }else{
            //retrieve all subnodes of the node given to look for Step.XXX.Steps
            HWchilds = SharedVars.getOTDBrmi().getRemoteMaintenance().getItemList(rootNode.treeID(), rootNode.nodeID(), 1);
        }
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
            //retrieving Strategy.Steps
            if (aHWNode.leaf && aHWNode.name.equals("Steps")) {
                strategyStepsParameter = aHWNode;
                logger.trace("Strategy Steps defined :"+strategyStepsParameter.limits);
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
                        buildStep(newStep,aHWNode);
                        parentNode.addChildStep(newStep);
                        logger.trace("Strategy Step defined : "+newStep.getName());
                    }
                }
            }
        }
    }
    
    public void persistStepTree(BBSStepNode aBBSStepTree, boolean isStrategyStepTree){
        
    }
    
    public void persistStep(BBSStepNode aBBSStepTree, boolean isStrategyStep){
        
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
}
