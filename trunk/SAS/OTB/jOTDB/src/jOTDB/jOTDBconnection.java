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

import java.util.Vector;
import java.util.Date;

public class jOTDBconnection
{
    // Just creates an object and registers the connection parameters.
    public jOTDBconnection (String username, String passwd, String database)
    {
	initOTDBconnection (username, passwd, database);
    }

   public jOTDBconnection()
     {
	initOTDBconnection("paulus", "boskabouter", "otdbtest");
     }
   
   
    // Create a OTDBconnection instance
    private native void initOTDBconnection (String username, String passwd, String database);

    // To test if we are (still) connected.
    public native boolean isConnected();

    // To connect or reconnect in case the connection was lost
    public native boolean connect();
    
    // get OTDBtree of one specific tree
    public native jOTDBtree getTreeInfo (int atreeID, boolean isMomID);
    
    public native Vector getStateList(int atreeID, boolean isMomID ,String beginDate, String endDate);


    // To get a list of all OTDB trees available in the database.
    public native Vector getTreeList(short treeType, short classifiType);
    
    public native String errorMsg();

    public native int getAuthToken();
}
