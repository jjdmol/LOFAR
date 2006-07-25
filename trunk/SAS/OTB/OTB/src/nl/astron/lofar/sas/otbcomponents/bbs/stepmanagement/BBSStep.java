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
    //Step Name
    private String name;
    //Step Baseline Selection
    private String[] station1Selection;
    private String[] station2Selection;
    //Step Sources
    private String[] peelSources;
    private String[] predictSources;
    //Step Instrument Model
    private String[] instrumentModel;
    //Step Integration
    private double integrationFrequency;
    private double integrationTime;
    //Step Correlation
    private String correlationSelection;
    private String[] correlationType;
    //Step Output Data Column
    private String outputDataColumn;
    
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
    
    public BBSStep getParentStep(){
        return parentStep;
    }
    
    public void setParentStep(BBSStep parentStep){
        this.parentStep = parentStep;
    }
    
    public boolean hasParentStep(){
        return parentStep == null;
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
            childStep.setParentStep(null);
            childSteps.remove(childStep);
            childSteps.trimToSize();
        }        
    }
    
    public boolean hasChildSteps(){
        return childSteps.size()>0;
    }
    
}
