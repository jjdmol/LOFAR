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

import java.util.HashMap;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;

/**
 * @version $Id$
 * @created July 25, 2006, 10:08 AM
 * @author pompert
 */
public class BBSStrategy{
    
    //Contained substeps
    private Vector<BBSStep> childSteps;
    //pointer to the Step Container in OTDB
    private jOTDBnode stepContainerNode = null;
    
    /** Creates a new instance of BBSStep */
    public BBSStrategy() {
        childSteps = new Vector<BBSStep>();
    }
    
    public Vector<BBSStep> getChildSteps(){
        return childSteps;
    }
    
    public BBSStep getChildStep(String name) throws IllegalArgumentException{
        BBSStep returnStep = null;
        for(BBSStep aStep : childSteps){
            if(aStep.getName().equals(name)){
                returnStep = aStep;
            }
        }
        if(returnStep==null){
            throw new IllegalArgumentException("No Child Step was found with name: "+name);
        }
        return returnStep;
    }
    public void addChildStep(BBSStep childStep){
        childSteps.add(childStep);
    }
    public void removeAllChildSteps(){
        for(BBSStep childStep : this.childSteps){
            childStep.removeAllChildSteps();
        }
        childSteps.clear();
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
