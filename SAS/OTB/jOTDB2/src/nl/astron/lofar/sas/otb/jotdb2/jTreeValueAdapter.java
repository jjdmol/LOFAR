//#  jTreeValueAdapter.java: 
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
import org.apache.log4j.Logger;

public class jTreeValueAdapter extends UnicastRemoteObject implements jTreeValueInterface
{

   // Create a Log4J logger instance
   static Logger logger = Logger.getLogger(jTreeMaintenanceAdapter.class);

   // Constructor
   public jTreeValueAdapter (jTreeValue adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
    public void setTreeID(int aTreeID) throws RemoteException 
    {
	adaptee.setTreeID(aTreeID);
    }

    public boolean addKVT( String key, String value, String time) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.addKVT( key, value, time) ;
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI addKVT error",ex);
            throw anEx;            
        }
        return aB;            
    }

    public boolean addKVT(jOTDBvalue aKVT) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.addKVT(aKVT);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI addKVT error",ex);
            throw anEx;            
        }
        return aB;
    }

    public boolean addKVTlist(Vector<jOTDBvalue> aValueList) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.addKVTlist(aValueList);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI addKVTlist error",ex);
            throw anEx;            
        }
        return aB;            
    }
    //    public  boolean addKVTparamSet(jParamterSet aPS) throws RemoteException {
    //        boolean aB=false;
    //        try {
    //	          aB = adaptee.addKVTparamSet(aPS);
    //        } catch (Exception ex) {
    //            RemoteException anEx=new RemoteException("JNI addKVTparamSet error");
    //            anEx.initCause(ex);
    //            throw anEx;            
    //        }
    //        return aB;
    //    }


    public Vector<jOTDBvalue> searchInPeriod (int topNode, int depth, String beginDate,
				  String endDate, boolean mostRecentlyOnly) throws RemoteException {
        Vector<jOTDBvalue> aV=null;
        try {
            aV = adaptee.searchInPeriod (topNode, depth, beginDate, endDate, mostRecentlyOnly);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI searchInPeriod error",ex);
            throw anEx;            
        }
        return aV;            
    }

    public Vector<jOTDBvalue> getSchedulableItems (int topNode) throws RemoteException {
        Vector<jOTDBvalue> aV=null;
        try {
            aV = adaptee.getSchedulableItems(topNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getSchedulableItems error",ex);
            throw anEx;            
        }
        return aV;            
    }

    public String  errorMsg() throws RemoteException {
        String aS=null;
        try {
            aS = errorMsg();
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI errorMsg error",ex);
            throw anEx;            
        }
        return aS;
    }

    protected jTreeValue adaptee;   
}
