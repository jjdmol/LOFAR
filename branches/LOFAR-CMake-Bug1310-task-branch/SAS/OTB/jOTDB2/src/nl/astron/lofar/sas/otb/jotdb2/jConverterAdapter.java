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

public class jConverterAdapter extends UnicastRemoteObject implements jConverterInterface
{
   // Constructor
   public jConverterAdapter (jConverter adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
    public short getClassif (String aConv) throws RemoteException
    {
        try {
            return adaptee.getClassif (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public String getClassif (short aConv) throws RemoteException
    {
        try {
            return adaptee.getClassif (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public HashMap<Short,String> getClassif() throws RemoteException
    {
        try {
            return adaptee.getClassif();
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public short getParamType (String aConv) throws RemoteException
    {
        try {
            return adaptee.getParamType (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }

    public String getParamType (short aConv) throws RemoteException
    {
        try {
            return adaptee.getParamType (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public HashMap<Short,String> getParamType() throws RemoteException
    {
        try {
            return adaptee.getParamType();
        } catch (RemoteException ex) {
            throw ex;
        }
    }

    public short getTreeState (String aConv) throws RemoteException
    {
        try {
            return adaptee.getTreeState (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public String getTreeState (short aConv) throws RemoteException
    {
        try {
            return adaptee.getTreeState (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public HashMap<Short,String> getTreeState() throws RemoteException
    {
        try {
            return adaptee.getTreeState();
        } catch (RemoteException ex) {
            throw ex;
        }
    }

    public short getTreeType (String aConv) throws RemoteException
    {
        try {
            return adaptee.getTreeType (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public String getTreeType (short aConv) throws RemoteException
    {
        try {
            return adaptee.getTreeType (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public HashMap<Short,String> getTreeType() throws RemoteException
    {
        try {
            return adaptee.getTreeType();
        } catch (RemoteException ex) {
            throw ex;
        }
    }

    public short getUnit (String aConv) throws RemoteException
    {
        try {
            return adaptee.getUnit (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public String getUnit (short aConv) throws RemoteException
    {
        try {
            return adaptee.getUnit (aConv);
        } catch (RemoteException ex) {
            throw ex;
        }
    }
    
    public HashMap<Short,String> getUnit() throws RemoteException
    {
        try {
            return adaptee.getUnit();
        } catch (RemoteException ex) {
            throw ex;
        }
    }

   
    protected jConverter adaptee;   
}
