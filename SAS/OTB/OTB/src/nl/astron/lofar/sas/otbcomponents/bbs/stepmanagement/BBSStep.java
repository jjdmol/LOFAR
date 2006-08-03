/*
 * BBSStep.java
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

import java.util.HashMap;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;

/**
 * @version $Id$
 * @created July 25, 2006, 10:08 AM
 * @author pompert
 */
public class BBSStep{
    
    //Possible parent step
    private BBSStep parentStep;
    //Contained substeps
    private Vector<BBSStep> childSteps;
    //pointer to the Step Container in OTDB
    private jOTDBnode stepContainerNode = null;
    //Step Name
    private String name;
    //Step Baseline Selection
    private Vector<String> station1Selection = null;
    private Vector<String> station2Selection = null;
    //Step Sources
    private Vector<String> sources = null;
    private Vector<String> extraSources = null;
    //Step Instrument Model
    private Vector<String> instrumentModel = null;
    //Step Integration
    private double integrationFrequency = -1;
    private double integrationTime = -1;
    //Step Correlation
    private String correlationSelection = null;
    private Vector<String> correlationType = null;
    //Step Output Data Column
    private String outputDataColumn = null;
    
    //TODO: Step Operation Types
    
    
    /** Creates a new instance of BBSStep */
    public BBSStep(String name) {
        this.name=name;
        childSteps = new Vector<BBSStep>();
        parentStep = null;
    }
    
    public String getName(){
        return name;
    }
    
    public void setName(String name){
        this.name = name;
    }
    
    public Vector<String> getStation1Selection(){
        if(parentStep!=null && station1Selection == null){
            return parentStep.getStation1Selection();
        }
        return station1Selection;
    }
    
    public void setStation1Selection(Vector<String> station1Selection){
        this.station1Selection = station1Selection;
    }
    public Vector<String> getStation2Selection(){
        if(parentStep!=null  && station2Selection == null){
            return parentStep.getStation2Selection();
        }
        return station2Selection;
    }
    
    public void setStation2Selection(Vector<String> station2Selection){
        this.station2Selection = station2Selection;
    }
    public Vector<String> getSources(){
        if(parentStep!=null && sources == null){
            return parentStep.getSources();
        }
        return sources;
    }
    
    public void setSources(Vector<String> sources){
        this.sources = sources;
    }
    
    public Vector<String> getExtraSources(){
        if(parentStep!=null && extraSources == null){
            return parentStep.getExtraSources();
        }
        return extraSources;
    }
    
    public void setExtraSources(Vector<String> extraSources){
        this.extraSources = extraSources;
    }
    
    public Vector<String> getInstrumentModel(){
        if(parentStep!=null && instrumentModel == null){
            return parentStep.getInstrumentModel();
        }
        return instrumentModel;
    }
    
    public void setInstrumentModel(Vector<String> instrumentModel){
        this.instrumentModel = instrumentModel;
    }
    
    public double getIntegrationFrequency(){
        if(parentStep!=null && integrationFrequency == -1){
            return parentStep.getIntegrationFrequency();
        }
        return integrationFrequency;
    }
    
    public void setIntegrationFrequency(double integrationFrequency){
        this.integrationFrequency = integrationFrequency;
    }
    
    public double getIntegrationTime(){
        if(parentStep!=null && integrationTime == -1){
            return parentStep.getIntegrationTime();
        }
        return integrationTime;
    }
    
    public void setIntegrationTime(double integrationTime){
        this.integrationTime = integrationTime;
    }
    
    public String getCorrelationSelection(){
        if(parentStep!=null && correlationSelection == null){
            return parentStep.getCorrelationSelection();
        }
        return correlationSelection;
    }
    
    public void setCorrelationSelection(String correlationSelection){
        this.correlationSelection = correlationSelection;
    }
    
    public Vector<String> getCorrelationType(){
        if(parentStep!=null && correlationType == null){
            return parentStep.getCorrelationType();
        }
        return correlationType;
    }
    
    public void setCorrelationType(Vector<String> correlationType){
        this.correlationType = correlationType;
    }
    
    public String getOutputDataColumn(){
        if(parentStep!=null && outputDataColumn == null){
            return parentStep.getOutputDataColumn();
        }
        return outputDataColumn;
    }
    
    public void setOutputDataColumn(String outputDataColumn){
        this.outputDataColumn = outputDataColumn;
    }
    
    public BBSStep getParentStep(){
        return parentStep;
    }
    
    public void setParentStep(BBSStep parentStep){
        this.parentStep = parentStep;
    }
    
    public jOTDBnode getStepContainerPointer(){
        return stepContainerNode;
    }
    
    public void setStepContainerPointer(jOTDBnode stepContainerNode){
        this.stepContainerNode = stepContainerNode;
    }
    
    public boolean hasParentStep(){
        return parentStep != null;
    }
    
    public Vector<BBSStep> getChildSteps(){
        return childSteps;
    }
    
    public BBSStep getChildStep(String name) throws IllegalArgumentException{
        BBSStep returnStep = null;
        for(BBSStep aStep : childSteps){
            if(aStep.name.equals(name)){
                returnStep = aStep;
            }
        }
        if(returnStep==null){
            throw new IllegalArgumentException("No Child Step was found with name: "+name);
        }
        return returnStep;
    }
    
    public BBSStep getChildStepAtIndex(int index) throws IllegalArgumentException{
        BBSStep returnStep = null;
        
        if(childSteps.elementAt(index)!=null){
            returnStep = childSteps.get(index);
        }else{
            throw new IllegalArgumentException("No Child Step was found at index: "+index);
        }
        return returnStep;
    }
    public void addChildStep(BBSStep childStep){
        childStep.setParentStep(this);
        childSteps.add(childStep);
    }
    public void setChildStepAtIndex(BBSStep childStep, int index){
        childStep.setParentStep(this);
        childSteps.add(index,childStep);
    }
    public void removeChildStepAtIndex(int index){
        childSteps.get(index).setParentStep(null);
        childSteps.remove(index);
        childSteps.trimToSize();
    }
    
    public void removeChildStep(BBSStep childStep){
        if(childSteps.contains(childStep)){
            childSteps.remove(childStep);
            childSteps.trimToSize();
            childStep.setParentStep(null);            
        }        
    }
    
    public boolean hasChildSteps(){
        return childSteps.size()>0;
    }
    
    public void finalize(){
        if(parentStep!=null){
            parentStep.removeChildStep(this);
        }
    }    
}
