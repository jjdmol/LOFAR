//#  jParmDBnode.java: Structure containing a tree node.
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

package nl.astron.lofar.sas.otb.util;

public class jParmDBnode implements java.io.Serializable {
    
    // private members
    private String name;
    private boolean leaf;
    private boolean rootNode;
    private String itsNodeID;
    private String itsParentID;
    private String parmDBlocation;
    private String tableIdentifier;
    
    public jParmDBnode(String nodeID, String parentID) {
        leaf = false;
        itsNodeID = nodeID;
        itsParentID = parentID;
        rootNode = false;
        tableIdentifier = "ParmDB";
    }
    public String getName() {
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public boolean isLeaf() {
        return leaf;
    }
    public void setLeaf(boolean leaf) {
        this.leaf = leaf;
    }
    public boolean isRootNode() {
        return rootNode;
    }
    public void setRootNode(boolean isRootNode) {
        this.rootNode = isRootNode;
    } 
    public String getNodeID() {
        return (itsNodeID);
    }
    public void setNodeID(String nodeId) {
        itsNodeID = nodeId;
    }    
    public String getParentID() {
        return (itsParentID);
    }
    public void setParentID(String parentID) {
        itsParentID = parentID;
    }    
    public String getParmDBLocation() {
        return (parmDBlocation);
    }
    public void setParmDBLocation(String location) {
        parmDBlocation = location;
    }
    public String getParmDBIdentifier() {
        return tableIdentifier;
    }
    public void setParmDBIdentifier(String tableIdentifier) {
        this.tableIdentifier = tableIdentifier;
    }
}
