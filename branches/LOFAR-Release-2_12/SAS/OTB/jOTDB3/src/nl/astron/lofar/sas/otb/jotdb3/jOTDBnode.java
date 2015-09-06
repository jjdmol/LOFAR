//#  jOTDBnode.java: Structure containing a tree node.
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

package nl.astron.lofar.sas.otb.jotdb3;

public class jOTDBnode implements java.io.Serializable
{
    public jOTDBnode (int treeID, int nodeID, int parentID, int paramDefID)
    {
	index = 0;
	leaf = false;
	instances = 0;
	itsTreeID = treeID;
	itsNodeID = nodeID;
	itsParentID = parentID;
	itsParamDefID = paramDefID;
    }
    
    public int treeID()
    { 
	return (itsTreeID); 
    }
    
    public int nodeID()
    { 
	return (itsNodeID); 
    }

    public int parentID()
    { 
	return (itsParentID); 
    }

    public int paramDefID()
    {
	return (itsParamDefID); 
    }
    
    public String name;
    public short index;
    public boolean leaf;
    public short instances;		//# only VICtemplate
    public String limits;			//# only VICtemplate
    public String description;	//# only VICtemplate

   // private members
   private int itsTreeID;
   private int itsNodeID;
   private int itsParentID;
   private int itsParamDefID;
}
