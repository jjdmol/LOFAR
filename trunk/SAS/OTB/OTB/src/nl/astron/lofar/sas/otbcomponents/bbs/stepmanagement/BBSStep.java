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

/**
 * The BBSStep represents an instance of a BBS Step with a given name. For each
 * BBS Step with a given name, multiple BBSStep objects can exist.
 *
 * @version $Id$
 * @created July 25, 2006, 10:08 AM
 * @author pompert
 * @see BBSStrategy
 * @see BBSStepData
 * @see BBSStepDataManager
 */
public class BBSStep implements Cloneable, Comparable{
    
    //Possible parent step
    private BBSStep parentStep = null;
    //Contained substeps
    private Vector<BBSStep> childSteps;
    //Step Name
    private String name;
    
    /** 
     * Creates a new instance of BBSStep 
     * 
     * @param name the Step Name to be given to the BBSStep object instance
     */
    public BBSStep(String name) {
        this.name=name;
        childSteps = new Vector<BBSStep>();
        parentStep = null;
    }
    /**
     * Returns the name of the BBS Step this BBSStep object represents
     * 
     * @return the BBS Step name
     */
    public String getName(){
        return name;
    }
    /**
     * Sets the name of the BBS Step this BBSStep object should represent
     * @param name the name of the BBS Step to be representing
     */
    public void setName(String name){
        this.name = name;
    }
    /**
     * Returns the parent BBSStep/Multistep (null if this BBSStep is a BBSStrategy childstep)
     * 
     * @return the BBSStep parent object
     */
    public BBSStep getParentStep(){
        return parentStep;
    }
    /**
     * Sets the parent BBSStep/Multistep (null if this BBSStep is to be a BBSStrategy childstep)
     * 
     * @parm parentStep the BBSStep parent object
     */
    public void setParentStep(BBSStep parentStep){
        this.parentStep = parentStep;
    }
    /**
     * Returns true if there is a parent BBSStep/Multistep associated with this BBSStep
     * 
     * @return <i>true</i> - if a parent BBSStep is associated, <i>false</i> - if no parent BBSStep
     * was found and this BBSStep should be a BBSStrategy child step.
     */
    public boolean hasParentStep(){
        return parentStep != null;
    }
    /**
     * Returns a Vector of BBSStep objects that are childs of this BBSStep<br><br>
     * -Returns null if the child steps have never been set<br>
     * -Returns empty vector is no child steps are associated.
     *
     * @return Vector of BBSStep objects that are children of this BBSStep
     */
    public Vector<BBSStep> getChildSteps(){
        return childSteps;
    }
    /**
     * Adds a BBSStep to this BBSStep's children collection.
     * 
     * @parm childStep the BBSStep to associate as a child
     */
    protected void addChildStep(BBSStep childStep){
        childStep.setParentStep(this);
        childSteps.add(childStep);
    }
    /**
     * Removes all associations with the childsteps (recursively) and vice versa.
     */
    public void removeAllChildSteps(){
        for(BBSStep childStep : this.childSteps){
            childStep.removeAllChildSteps();
            childStep.setParentStep(null);
        }
        childSteps.clear();
    }
    /**
     * Answers if this BBSStep has any children BBSStep object(s).
     * 
     * @return <i>true</i> - if one or more BBSStep children is/are associated, <i>false</i> - if no children BBSStep
     * objects were found.
     */
    public boolean hasChildSteps(){
        return childSteps.size()>0;
    }
    /**
     * Prints the name of the BBS Step this object represents.
     * @return the name of the represented BBS Step
     */
    @Override
    public String toString(){
        return getName();
    }
    /**
     * Comparable method implementation that can compare an Object with this BBSStep
     * 
     * @parm otherObject the Object to compare with
     * @return <i>-1</i> - if the Object to be compared with is not a BBSStep object<br>
     * <i>0</i> - if the BBSStep being compared with shares the same BBS Step name as this BBSStep object<br>
     */
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
    /**
     * Clones this BBSStep instance including attributes and includes 
     * clones of the child BBSStep objects.
     * 
     * @return complete clone of this BBSStep
     */
    @Override
    public BBSStep clone(){
        BBSStep newStep = new BBSStep(getName());
        newStep.setParentStep(null);
        for(BBSStep childStep : this.getChildSteps()){
            BBSStep newChildStep = childStep.clone();
            newChildStep.setParentStep(newStep);
            newStep.addChildStep(newChildStep);
        }
        return newStep;
    }
    /**
     * Inserts a child BBSStep into this BBSStep object, should the parent String
     * argument be equal to the name of this BBSStep object. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same amount of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to insert the BBSStep into.
     * @parm child the BBSStep object to insert as a child step.
     */
    public void cascadingStepInsertion(String parent,BBSStep child){
        
        if(parent != null && parent.equals(this.getName())){
            BBSStep newStep = child.clone();
            addChildStep(newStep);
        }
        for(BBSStep childStep : this.childSteps){
            childStep.cascadingStepInsertion(parent,child);
        }
        
    }
    /**
     * Deletes a child BBSStep from this BBSStep object, should the parent String
     * argument be equal to the name of this BBSStep object. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same amount of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to delete the BBSStep from.
     * @parm child the BBSStep object to delete.
     * @parm indexOfChild the index of the child step in its parent, to make sure only one 
     * BBSStep is deleted when more than one BBSStep objects with the same name are present in the 
     * parent's child step collection.
     */
    public void cascadingStepDeletion(BBSStep parent,BBSStep child, int indexOfChild){
        
        if(parent != null && parent.getName().equals(this.getName())){
            if(indexOfChild >= 0 && indexOfChild < childSteps.size()){
                BBSStep currentStepInIndex = childSteps.get(indexOfChild);
                if(child.getName().equals(currentStepInIndex.getName())){
                    this.childSteps.removeElementAt(indexOfChild);
                    childSteps.trimToSize();
                }
            }
        }
        for(BBSStep childStep : this.childSteps){
            childStep.cascadingStepDeletion(parent,child,indexOfChild);
        }
        
    }
    /**
     * Moves a child BBSStep to a new place in this BBSStep object, should the parent String
     * argument be equal to the name of this BBSStep object. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same order of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to move the BBSStep in.
     * @parm child the BBSStep object to move.
     * @parm oldIndexOfChild the current index of the child step in its parent.
     * @parm newIndexOfChild the new index of the child step in its parent.
     */
    public void cascadingStepMove(BBSStep parent,BBSStep child, int oldIndexOfChild, int newIndexOfChild){
        
        if(parent != null && parent.getName().equals(this.getName())){
            if(oldIndexOfChild >= 0 && oldIndexOfChild < childSteps.size()){
                BBSStep currentStepInIndex = childSteps.get(oldIndexOfChild);
                if(child.getName().equals(currentStepInIndex.getName())){
                    this.childSteps.removeElementAt(oldIndexOfChild);
                    this.childSteps.add(newIndexOfChild,currentStepInIndex);
                    childSteps.trimToSize();
                }
            }
        }
        for(BBSStep childStep : this.childSteps){
            childStep.cascadingStepMove(parent,child,oldIndexOfChild,newIndexOfChild);
        }
        
    }
}
