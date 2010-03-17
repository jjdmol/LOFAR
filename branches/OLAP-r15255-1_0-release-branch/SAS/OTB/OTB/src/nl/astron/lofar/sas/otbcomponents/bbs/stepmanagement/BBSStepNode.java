/*
 * BBSStepNode.java
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

import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
/**
 * Represents a specific BBSStep object inside the Steps Tree of the BBS Strategy panel.
 *
 * @created 11-07-2006, 13:37
 * @author  pompert
 * @version $Id$
 */
public class BBSStepNode implements java.io.Serializable {
    
    // private members
    private String name;
    private boolean rootNode;
    protected boolean leaf;
    private BBSStep containedStep;
    private jOTDBnode representedOTDBnode;
    
    /**
     * Builds a new BBSStepNode containing a given BBSStep object
     *
     * @parm aBBSStep the BBSStep to contain in the JTree structure.
     */
    public BBSStepNode(BBSStep aBBSStep) {
        containedStep = aBBSStep;
        rootNode = false;
        leaf = true;
    }
    
    /**
     * Returns a friendly name of the contained BBSStep object
     *
     * @return friendly name of the contained BBSStep object.
     */
    public String getName() {
        if(containedStep!=null){
            return containedStep.getName();
        }
        return name;
    }
    
    /**
     * Sets a friendly name of this BBSStepNode, which is only displayed if 
     * this BBSStepNode does not contain a BBSStep object (the root node of the tree for instance)
     *
     * @parm name the friendly name of the BBSStepNode.
     */
    
    public void setName(String name) {
        this.name = name;
    }
    
    /**
     * Answers if the BBSStepNode has any child nodes, 
     * which is determined by the child steps in the contained BBSStep object.
     *
     * @return <i>true</i> - if the contained BBSStep has child steps, <i>false</i> - if the contained BBSStep does not have child steps. 
     */
    public boolean isLeaf() {
        boolean returnBool = true;
        if(containedStep != null && !rootNode){
            returnBool = !containedStep.hasChildSteps();
        }else{
            returnBool = leaf;
        }
        return returnBool;
    }
    
    /**
     * Answers if this BBSStepNode is the Root node of the JTree structure.
     * @return <i>true</i> - if this node represents the root node, <i>false</i> - if this node does not represent the root node. 
     */
    public boolean isRootNode() {
        return rootNode;
    }
    
    /**
     * Sets this BBSStepNode as to be/not to be the representation of the JTree root node.
     *
     * @parm isRootNode <i>true</i> - if this node should represent the root node, <i>false</i> - if this node should not represent the root node. 
     */
    public void setRootNode(boolean isRootNode) {
        this.rootNode = isRootNode;
    }
    
    /**
     * Returns the contained BBSStep object.
     *
     * @return the contained BBSStep object.
     */
    public BBSStep getContainedStep(){
        return containedStep;
    }
    
    /**
     * Sets the contained BBSStep object.
     *
     * @parm containedStep the BBSStep object to contain.
     */
    public void setContainedStep(BBSStep containedStep){
        this.containedStep = containedStep;
    }
    
    /**
     * Returns the OTDB node if needed. (only used in the root node of the JTree)
     *
     * @return the contained OTDB node.
     */
    public jOTDBnode getOTDBNode(){
        return representedOTDBnode;
    }
    
    /**
     * Sets the OTDB node if needed. (only used in the root node of the JTree)
     *
     * @parm representedOTDBnode the to be contained OTDB node.
     */
    public void setOTDBNode(jOTDBnode representedOTDBnode){
        this.representedOTDBnode = representedOTDBnode;
    }
}
