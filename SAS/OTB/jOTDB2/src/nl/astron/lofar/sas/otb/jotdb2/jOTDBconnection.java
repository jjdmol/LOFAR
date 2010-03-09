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

package nl.astron.lofar.sas.otb.jotdb2;

import java.util.Vector;

public class jOTDBconnection
{
    // Just creates an object and registers the connection parameters.
    public jOTDBconnection (String username, String passwd, String database, String hostname)
    {
        try {
            initOTDBconnection (username, passwd, database, hostname);
        } catch (Exception ex) {
            System.out.println("Error during connection init :" + ex);
        }
    }

   public jOTDBconnection()
     {
        try {
            initOTDBconnection("paulus", "boskabouter", "otdbtest" , "dop50.astron.nl");
        } catch (Exception ex) {
            System.out.println("Error during connection init :" + ex);
        }
     }
   
   
    // Create a OTDBconnection instance
    private native void initOTDBconnection (String username, String passwd, String database, String hostname) throws Exception;

    // To test if we are (still) connected.
    public native boolean isConnected() throws Exception;

    // To connect or reconnect in case the connection was lost
    public native boolean connect() throws Exception;
    
    // get OTDBtree of one specific tree
    public native jOTDBtree getTreeInfo (int atreeID, boolean isMomID)throws Exception ;
    
    public native Vector<jTreeState> getStateList(int atreeID, boolean isMomID ,String beginDate, String endDate) throws Exception;


    // To get a list of all OTDB trees available in the database.
    public native Vector<jOTDBtree> getTreeList(short treeType, short classifiType) throws Exception;
    
    public native String errorMsg() throws Exception;

    public native int getAuthToken() throws Exception;

    public native String getDBName() throws Exception;
}
