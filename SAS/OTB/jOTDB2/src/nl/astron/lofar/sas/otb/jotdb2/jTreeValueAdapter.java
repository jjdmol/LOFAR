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

public class jTreeValueAdapter extends UnicastRemoteObject implements jTreeValueInterface
{
   // Constructor
   public jTreeValueAdapter (jTreeValue adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
    public void setTreeID(int aTreeID) throws RemoteException 
    {
	adaptee.setTreeID(aTreeID);
    }

    public boolean addKVT( String key, String value, String time) throws RemoteException
    {
	return adaptee.addKVT( key, value, time) ;
    }

    public boolean addKVT(jOTDBvalue aKVT) throws RemoteException
    {
	return adaptee.addKVT(aKVT);
    }

    public boolean addKVTlist(Vector<jOTDBvalue> aValueList) throws RemoteException
    {
	return adaptee.addKVTlist(aValueList);
    }
    //    public  boolean addKVTparamSet(jParamterSet aPS) throws RemoteException
    //    {
    //	return adaptee.addKVTparamSet(aPS);
    //    }


    public Vector searchInPeriod (int topNode, int depth, String beginDate,
				  String endDate, boolean mostRecentlyOnly) throws RemoteException
    {
	return adaptee.searchInPeriod (topNode, depth, beginDate, endDate, mostRecentlyOnly);
    }

    public Vector<jOTDBvalue> getSchedulableItems (int topNode) throws RemoteException
    {
	return adaptee.getSchedulableItems(topNode);
    }

    public String  errorMsg() throws RemoteException
    {
	return errorMsg();
    }

    protected jTreeValue adaptee;   
}
