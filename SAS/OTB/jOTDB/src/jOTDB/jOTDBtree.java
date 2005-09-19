//#  jOTDBconnection.java: Manages the connection with the OTDB database.
//#
//#  Copyright (C) 2002-2004
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

package jOTDB;

import java.util.Date;

public class jOTDBtree implements java.io.Serializable
{
   public jOTDBtree()
     {
	itsTreeID = 0;
     }
   
   public int treeID()
     {
	return (itsTreeID); 
     }
   
   public short classification; // development / test / operational
   public String creator;
   public String creationDate;	
   public short type;			// hardware / VItemplate / VHtree
   public short state;			// idle / configure / ... / active / ...
   // -- VIC only --
   public int originalTree;
   public String campaign;
   public String starttime;
   public String stoptime;
   public int itsTreeID;

   //# Prevent changing the database keys
   private jOTDBtree (int aTreeID) 
     {
	 itsTreeID = aTreeID;
     }   
}
