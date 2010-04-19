//#  jTreeValueInterface.java: The RMI interface to the OTDB database.
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


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Vector;

public interface jTreeValueInterface extends Remote 
{
    // Constants
    public static final String SERVICENAME = "jTreeValue";

    public void setTreeID(int aTreeID) throws RemoteException;

    public boolean addKVT( String key, String value, String time) throws RemoteException;

    public boolean addKVT(jOTDBvalue aKVT) throws RemoteException;

    public boolean addKVTlist(Vector<jOTDBvalue> aValueList) throws RemoteException;

    //    public boolean addKVTparamSet(jParamterSet aPS) throws RemoteException;

    public Vector<jOTDBvalue> searchInPeriod (int topNode, int depth, String beginDate,
				  String endDate, boolean mostRecentlyOnly) throws RemoteException;


    public Vector<jOTDBvalue> getSchedulableItems (int topNode) throws RemoteException;

    public String  errorMsg() throws RemoteException;
}
