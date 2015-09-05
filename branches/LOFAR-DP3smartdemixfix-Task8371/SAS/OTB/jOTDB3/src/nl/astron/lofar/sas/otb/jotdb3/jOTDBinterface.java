//#  jOTDBinterface.java: The RMI interface to the OTDB database.
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
   
   // To disconnect the connection
   public void disconnect () throws RemoteException;

   // get OTDBtree of one specific tree
   public jOTDBtree getTreeInfo (int atreeID,boolean isMomID) throws RemoteException;
   public jOTDBtree getTreeInfo (int atreeID) throws RemoteException;
   
   // To get a list of all OTDB trees available in the database.
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType, int aGroupID, String aProcessType, String aProcessSubtype, String aStrategy) throws RemoteException;
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType, int aGroupID, String aProcessType, String aProcessSubtype) throws RemoteException;
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType, int aGroupID, String aProcessType) throws RemoteException;
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType, int aGroupID) throws RemoteException;
   public Vector<jOTDBtree> getTreeList (short treeType, short classifiType) throws RemoteException;
   public Vector<jOTDBtree> getTreeList (short treeType) throws RemoteException;

   // To get a list of all OTDB trees available in the database.
   public Vector<jTreeState> getStateList (int treeID, boolean isMomID, String beginDate, String endDate) throws RemoteException;
   public Vector<jTreeState> getStateList (int treeID, boolean isMomID, String beginDate) throws RemoteException;
   public Vector<jTreeState> getStateList (int treeID, boolean isMomID) throws RemoteException;
   public Vector<jTreeState> getStateList (int treeID) throws RemoteException;
   
   // To get a list of all DefaultTemplates available in the database.
   public Vector<jDefaultTemplate> getDefaultTemplates () throws RemoteException;

   // To get a list of all executable OTDB trees available in the database.
   public Vector<jOTDBtree> getExecutableTrees (short classifiType) throws RemoteException;
   public Vector<jOTDBtree> getExecutableTrees () throws RemoteException;

   // To get a list of the treeGroups fitting the bounds
   public Vector<jOTDBtree> getTreeGroup (short groupType,short periodInMinutes) throws RemoteException;

   // To get a list of the trees fitting the bounds
   public Vector<jOTDBtree> getTreesInPeriod (short treeType, String beginDate, String endDate) throws RemoteException;
   public Vector<jOTDBtree> getTreesInPeriod (short treeType, String beginDate) throws RemoteException;
   public Vector<jOTDBtree> getTreesInPeriod (short treeType) throws RemoteException;

       // To get a list of all OTDB trees modified after given timestamp
    public Vector<jOTDBtree> getModifiedTrees(String after, short treeType) throws RemoteException;
    public Vector<jOTDBtree> getModifiedTrees(String after) throws RemoteException;

   // Get a new unique groupID
   public int newGroupID() throws RemoteException;

   public String errorMsg () throws RemoteException;
   
   public  int getAuthToken () throws RemoteException;

   public String getDBName () throws RemoteException;

}
