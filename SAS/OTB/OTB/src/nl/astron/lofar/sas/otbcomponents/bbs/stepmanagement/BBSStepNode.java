//#  BBSStepNode.java: Structure containing a tree node.
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;

public class BBSStepNode implements java.io.Serializable {
    
    // private members
    private String name;
    private boolean rootNode;
    protected boolean leaf;
    private BBSStep containedStep;
    private jOTDBnode representedOTDBnode;
    
    public BBSStepNode(BBSStep aBBSStep) {
        containedStep = aBBSStep;
        rootNode = false;
        leaf = true;
    }
    public String getName() {
        if(containedStep!=null){
            return containedStep.getName();
        }
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public boolean isLeaf() {
        boolean returnBool = true;
        if(containedStep != null && !rootNode){
            returnBool = !containedStep.hasChildSteps();
        }else{
            returnBool = leaf;
        }
        return returnBool;
    }
    public boolean isRootNode() {
        return rootNode;
    }
    public void setRootNode(boolean isRootNode) {
        this.rootNode = isRootNode;
    }       
    public BBSStep getContainedStep(){
        return containedStep;
    }
    public void setContainedStep(BBSStep containedStep){
        this.containedStep = containedStep;
    }
    public jOTDBnode getOTDBNode(){
        return representedOTDBnode;
    }
    public void setOTDBNode(jOTDBnode representedOTDBnode){
        this.representedOTDBnode = representedOTDBnode;
    }
}
