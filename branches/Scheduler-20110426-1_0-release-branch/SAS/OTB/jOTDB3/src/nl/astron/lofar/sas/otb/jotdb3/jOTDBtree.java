//#  jOTDBconnection.java: Manages the connection with the OTDB database.
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

package nl.astron.lofar.sas.otb.jotdb3;

public class jOTDBtree implements java.io.Serializable
{
    public jOTDBtree ()
    {
	itsTreeID = 0;
	itsMomID=0;
    }

    public jOTDBtree (int treeID)
    {
	itsTreeID = treeID;
	itsMomID = 0;
    }

    public jOTDBtree (int treeID, int momID)
    {
	itsTreeID = treeID;
	itsMomID  = momID;
    }

   
    public int treeID()
    {
	return (itsTreeID); 
    }

    public int momID()
    {
	return (itsMomID);
    }
   
    public short classification; // development / test / operational
    public String creator;
    public String creationDate;	
    public short type;			// hardware / VItemplate / VHtree
    public short state;			// idle / configure / ... / active / ...
    public String description;          // free text

    // -- VIC only --
    public int originalTree;
    public String campaign;
    public String starttime;
    public String stoptime;
    private int itsTreeID;
    private int itsMomID;

}
