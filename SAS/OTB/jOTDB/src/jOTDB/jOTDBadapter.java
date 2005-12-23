//#  jOTDBadapter.java: The RMI adapter of the OTDB database.
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

import jOTDB.jOTDBtree;
import jOTDB.jOTDBconnection;
import jOTDB.jOTDBinterface;
import java.util.Vector;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;

public class jOTDBadapter extends UnicastRemoteObject implements jOTDBinterface
{
   // Constructor
   public jOTDBadapter (jOTDBconnection adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
   // To test if we are (still) connected.
   public boolean isConnected () throws RemoteException
     {
	return adaptee.isConnected ();
     }
   
   // To connect or reconnect in case the connection was lost
   public boolean connect () throws RemoteException
     {
	return adaptee.connect ();
     }
   
   // get OTDBtree of one specific tree
   public jOTDBtree getTreeInfo (int atreeID,boolean isMomID) throws RemoteException
     {
	return adaptee.getTreeInfo (atreeID,isMomID);
     }

     
   
   // To get a list of all StateChanges
   public Vector getStateList (int treeID, boolean isMomID, String beginDate, String endDate) throws RemoteException
     {
	try {
		return adaptee.getStateList (treeID, isMomID, beginDate, endDate);
	}catch (Exception e){
		throw new RemoteException();
	}
     }
      
   // To get a list of all OTDB trees available in the database.
   public Vector getTreeList (short treeType, short classifiType) throws RemoteException
     {
	return adaptee.getTreeList (treeType, classifiType);
     }
      
   public String errorMsg () throws RemoteException
     {
	return adaptee.errorMsg ();
     }
   
   public  int getAuthToken () throws RemoteException
     {
	return adaptee.getAuthToken ();
     }
   	
   protected jOTDBconnection adaptee;   
}
