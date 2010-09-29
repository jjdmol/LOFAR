//#  jConverterAdapter.java: 
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


import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.util.HashMap;
import org.apache.log4j.Logger;

public class jConverterAdapter extends UnicastRemoteObject implements jConverterInterface
{
   // Create a Log4J logger instance
   static Logger logger = Logger.getLogger(jTreeMaintenanceAdapter.class);

   // Constructor
   public jConverterAdapter (jConverter adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
    public short getClassif (String aConv) throws RemoteException
    {
        short aS;
        try {
            aS=adaptee.getClassif (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getClassif(String) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public String getClassif (short aConv) throws RemoteException
    {
        String aS="";
        try {
            aS =  adaptee.getClassif (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getClassif(short) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public HashMap<Short,String> getClassif() throws RemoteException
    {
        HashMap<Short,String> aM=null;
        try {
            aM = adaptee.getClassif();
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getClassif() error",ex);
            throw anEx;
        }
        return aM;
    }
    
    public short getParamType (String aConv) throws RemoteException
    {
        short aS;
        try {
            aS = adaptee.getParamType (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getParamType(String) error",ex);
            throw anEx;
        }
        return aS;
    }

    public String getParamType (short aConv) throws RemoteException
    {
        String aS="";
        try {
            aS = adaptee.getParamType (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getParamType(short) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public HashMap<Short,String> getParamType() throws RemoteException
    {
        HashMap<Short,String> aM=null;
        try {
            aM = adaptee.getParamType();
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getParamType() error",ex);
            throw anEx;
        }
        return aM;
    }

    public short getTreeState (String aConv) throws RemoteException
    {
        short aS;
        try {
            aS = adaptee.getTreeState (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeState(String) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public String getTreeState (short aConv) throws RemoteException
    {
        String aS="";
        try {
            aS = adaptee.getTreeState (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeState(short) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public HashMap<Short,String> getTreeState() throws RemoteException
    {
        HashMap<Short,String> aM=null;
        try {
            aM  = adaptee.getTreeState();
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeState() error",ex);
            throw anEx;
        }
        return aM;
    }

    public short getTreeType (String aConv) throws RemoteException
    {
        short aS;
        try {
            aS =  adaptee.getTreeType (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeType(String) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public String getTreeType (short aConv) throws RemoteException
    {
        String aS="";
        try {
            aS = adaptee.getTreeType (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeType(short) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public HashMap<Short,String> getTreeType() throws RemoteException
    {
        HashMap<Short,String> aM=null;
        try {
            aM = adaptee.getTreeType();
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTreeType() error",ex);
            throw anEx;
        }
        return aM;
    }

    public short getUnit (String aConv) throws RemoteException
    {
        short aS;
        try {
            aS = adaptee.getUnit (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getUnit(String) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public String getUnit (short aConv) throws RemoteException
    {
        String aS="";
        try {
            aS = adaptee.getUnit (aConv);
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getUnit(short) error",ex);
            throw anEx;
        }
        return aS;
    }
    
    public HashMap<Short,String> getUnit() throws RemoteException
    {
        HashMap<Short,String> aM=null;
        try {
            aM = adaptee.getUnit();
        } catch (RemoteException ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getUnit() error",ex);
            throw anEx;
        }
        return aM;
    }

   
    protected jConverter adaptee;   
}
