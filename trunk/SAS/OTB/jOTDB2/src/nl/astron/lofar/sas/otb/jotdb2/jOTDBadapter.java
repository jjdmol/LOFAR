//#  jOTDBadapter.java: The RMI adapter of the OTDB database.
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
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;

public class jOTDBadapter extends UnicastRemoteObject implements jOTDBinterface {
    // Constructor
    public jOTDBadapter (jOTDBconnection adaptee) throws RemoteException   {
        this.adaptee = adaptee;
     }
   
    // To test if we are (still) connected.
    public boolean isConnected () throws RemoteException {
        boolean aB=false;
        try {
            aB=adaptee.isConnected ();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI isConnected error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aB;  
     }
   
   // To connect or reconnect in case the connection was lost
   public boolean connect () throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.connect ();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI connect error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aB;
   }
   
   // get OTDBtree of one specific tree
   public jOTDBtree getTreeInfo (int atreeID,boolean isMomID) throws RemoteException {
        jOTDBtree aT=null;
        try {
            aT = adaptee.getTreeInfo (atreeID,isMomID);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeInfo error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aT;  

     }

     
   
    // To get a list of all StateChanges
    public Vector getStateList (int treeID, boolean isMomID, String beginDate, String endDate) throws RemoteException {
        Vector aV=null;
	try {
            aV = adaptee.getStateList (treeID, isMomID, beginDate, endDate);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getStateList error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aV;  
     }
      
    // To get a list of all OTDB trees available in the database.
    public Vector getTreeList (short treeType, short classifiType) throws RemoteException {
        Vector aV=null;
        try {
            aV = adaptee.getTreeList (treeType, classifiType);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeList error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aV;            
    }
      
    public String errorMsg () throws RemoteException {
        String aS=null;
        try {
            aS = adaptee.errorMsg ();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI errorMsg error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;             
    }
   
    public  int getAuthToken () throws RemoteException {
        int anI;
        try {
            anI = adaptee.getAuthToken ();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getAuthToken error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return anI;              
     }
   	
    public String getDBName () throws RemoteException {
        String aS=null;
        try {
            aS = adaptee.getDBName ();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getDBName error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;             
    }
   
   protected jOTDBconnection adaptee;   
}
