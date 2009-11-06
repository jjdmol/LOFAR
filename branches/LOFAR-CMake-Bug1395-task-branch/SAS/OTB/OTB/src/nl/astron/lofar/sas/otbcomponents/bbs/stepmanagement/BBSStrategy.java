/*
 * BBSStrategy.java
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
 * Representation of the BBS Strategy to contain all BBS Strategy steps.
 *
 * @version $Id$
 * @created July 25, 2006, 10:08 AM
 * @author pompert
 */
public class BBSStrategy{
    
    //Contained substeps
    private Vector<BBSStep> childSteps;
    
    /** 
     * Creates a new instance of BBSStrategy 
     */
    public BBSStrategy() {
        childSteps = new Vector<BBSStep>();
    }
    
    /**
     * Returns a Vector of BBSStep objects that are childs of this BBSStrategy<br><br>
     * -Returns null if the child steps have never been set<br>
     * -Returns empty vector is no child steps are associated.
     *
     * @return Vector of BBSStep objects that are children of this BBSStep
     */
    public Vector<BBSStep> getChildSteps(){
        return childSteps;
    }    
    
    /**
     * Adds a BBSStep to this BBSStrategy's children collection.
     * 
     * @parm childStep the BBSStep to associate as a child
     */
    public void addChildStep(BBSStep childStep){
        childSteps.add(childStep);
    }
    
    /**
     * Removes all associations with the childsteps (recursively) and vice versa.
     */
    public void removeAllChildSteps(){
        for(BBSStep childStep : this.childSteps){
            childStep.removeAllChildSteps();
        }
        childSteps.clear();
    }
    
    /**
     * Answers if this BBSStrategy has any children BBSStep object(s).
     * 
     * @return <i>true</i> - if one or more BBSStep children is/are associated, <i>false</i> - if no children BBSStep
     * objects were found.
     */
    public boolean hasChildSteps(){
        return childSteps.size()>0;
    }
    
    /**
     * Inserts a child BBSStep into this BBSStrategy object, should parent be null. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same amount of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to insert the BBSStep into.
     * null if it is to be inserted in this BBSStrategy child steps collection
     * @parm child the BBSStep object to insert as a child step.
     */
    public void cascadingStepInsertion(String parent,BBSStep child){
        //strategy step
        if(parent==null){
            addChildStep(child);
        }else{
            for(BBSStep childStep : this.childSteps){
                childStep.cascadingStepInsertion(parent,child);
            }
        }
    }    
    
    /**
     * Deletes a child BBSStep from this BBSStrategy object, should parent be null. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same amount of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to delete the BBSStep from.
     *  null if it is to be deleted from this BBSStrategy child steps collection
     * @parm child the BBSStep object to delete.
     * @parm indexOfChild the index of the child step in its parent, to make sure only one 
     * BBSStep is deleted when more than one BBSStep objects with the same name are present in the 
     * parent's child step collection.
     */
    public void cascadingStepDeletion(BBSStep parent,BBSStep child, int indexOfChild){
        //strategy step
       if(parent==null){
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
     * Moves a child BBSStep to a new place in this BBSStrategy object, should parent be null. It then calls the same method
     * on all child steps, to make sure all BBSStep objects that share the same name will
     * contain the same order of child steps.
     * 
     * @parm parent the Step name of the parent BBSStep to move the BBSStep in.
     *  null if it is to be moved inside this BBSStrategy's child steps collection
     * @parm child the BBSStep object to move.
     * @parm oldIndexOfChild the current index of the child step in its parent.
     * @parm newIndexOfChild the new index of the child step in its parent.
     */
    public void cascadingStepMove(BBSStep parent,BBSStep child, int oldIndexOfChild, int newIndexOfChild){
        
        if(parent == null){
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
