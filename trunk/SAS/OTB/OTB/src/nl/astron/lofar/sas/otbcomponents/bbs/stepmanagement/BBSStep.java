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

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;

/**
 * @version $Id$
 * @created July 25, 2006, 10:08 AM
 * @author pompert
 */
public class BBSStep implements Cloneable, Comparable{
    
    //Possible parent step
    private BBSStep parentStep = null;
    //Pointer to BBS Strategy
    private BBSStrategy itsStrategy = null;
    //Contained substeps
    private Vector<BBSStep> childSteps;
    //Step Name
    private String name;
    
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
    public void addChildStep(BBSStep childStep){
        childStep.setParentStep(this);
        childSteps.add(childStep);
    }
    public void removeAllChildSteps(){
        for(BBSStep childStep : this.childSteps){
            childStep.removeAllChildSteps();
            removeChildStep(childStep);
        }
        childSteps.trimToSize();
    }
    public void removeChildStep(BBSStep childStep){
        if(childSteps.contains(childStep)){
            childSteps.remove(childStep);
            childSteps.trimToSize();
            childStep.setParentStep(null);
        }
    }
    
    public void moveChildStep(BBSStep childStep, int newIndex){
        if(hasChildStep(childStep) && childSteps.size()>newIndex && newIndex >= 0){
            childSteps.remove(childStep);
            childSteps.add(newIndex,childStep);
        }
    }
    
    public boolean hasChildSteps(){
        return childSteps.size()>0;
    }
    
    public boolean hasChildStep(BBSStep aChildStep){
        return childSteps.contains(aChildStep);
    }
    
    public void finalize(){
        if(parentStep!=null){
            parentStep.removeChildStep(this);
        }
    }
    
    public String toString(){
        return getName();
    }
    
    public BBSStrategy getStrategy(){
        return itsStrategy;
    }
    
    public void setStrategy(BBSStrategy itsStrategy){
        this.itsStrategy = itsStrategy;
    }
    
    public int compareTo(Object otherObject){
        int returnInt = -1;
        if(otherObject instanceof BBSStep){
            BBSStep otherStep = (BBSStep)otherObject;
            if(otherStep.getName().equals(this.getName())){
                returnInt = 0;
            }
        }
        return returnInt;
    }
    
    public BBSStep clone(){
        BBSStep newStep = new BBSStep(getName());
        newStep.setParentStep(null);
        newStep.setStrategy(this.getStrategy());
        for(BBSStep childStep : this.getChildSteps()){
            BBSStep newChildStep = childStep.clone();
            newChildStep.setParentStep(newStep);
            newStep.addChildStep(newChildStep);
        }
        return newStep;
    }
    public void cascadingStepInsertion(String parent,BBSStep child){
        
        if(parent.equals(this.getName())){
            BBSStep newStep = child.clone();
            addChildStep(newStep);
        }
        for(BBSStep childStep : this.childSteps){
            childStep.cascadingStepInsertion(parent,child);
        }
        
    }
    
    public void cascadingStepDeletion(BBSStep parent,BBSStep child){
        
        if(parent.equals(this.getName())){
            removeChildStep(child);
        }
        for(BBSStep childStep : this.childSteps){
            childStep.cascadingStepDeletion(parent,child);
        }
        
    }
}
