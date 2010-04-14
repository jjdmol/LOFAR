//#  jOTDBinterface.java: The RMI interface to the OTDB database.
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


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Vector;

public interface jOTDBinterface extends Remote 
{
   // Constants
   public static final String SERVICENAME = "jOTDB";
   
   // To test if we are (still) connected.
   public boolean isConnected () throws RemoteException;
   
   // To connect or reconnect in case the connection was lost
   public boolean connect () throws RemoteException;
   
   // get OTDBtree of one specific tree
   public jOTDBtree getTreeInfo (int atreeID,boolean isMomID) throws RemoteException;
   
   // To get a list of all OTDB trees available in the database.
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType) throws RemoteException;

   // To get a list of all OTDB trees available in the database.
   public Vector<jTreeState> getStateList (int treeID, boolean isMomID, String beginDate, String endDate) throws RemoteException;
   
   public String errorMsg () throws RemoteException;
   
   public  int getAuthToken () throws RemoteException;

   public String getDBName () throws RemoteException;

}
