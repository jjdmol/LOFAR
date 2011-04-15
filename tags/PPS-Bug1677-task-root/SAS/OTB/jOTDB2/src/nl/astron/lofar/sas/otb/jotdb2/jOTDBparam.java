//#  jOTDBparam.java:Structure describing one parameter.
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

package nl.astron.lofar.sas.otb.jotdb2;

public class jOTDBparam implements java.io.Serializable
{
    public jOTDBparam (int treeID, int paramID, int NodeID)
    {
	itsTreeID = treeID;
	itsParamID = paramID;
	itsNodeID = NodeID;
    }
    
    public int treeID()
    {
	return (itsTreeID); 
    }

    public int paramID()
    { 
	return (itsParamID); 
    }
	
    public int nodeID()
    { 
	return (itsNodeID); 
    }

    public String name;
    public short index;
    public short type;			// node / bool / int / long / float / etc.
    public short unit;
    public short pruning;
    public short valMoment;
    public boolean runtimeMod;
    public String limits;
    public String description;

    private int	itsTreeID;
    private int itsParamID;
    private int itsNodeID;
}
