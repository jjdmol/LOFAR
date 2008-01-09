//#  jConverterInterface.java: The RMI interface to the OTDB database.
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
import java.util.HashMap;

public interface jConverterInterface extends Remote 
{
    // Constants
    public static final String SERVICENAME = "jConverter";

    public short getClassif (String aConv) throws RemoteException;
    public String getClassif (short aConv) throws RemoteException;
    public HashMap<Short,String> getClassif() throws RemoteException;
    
    public short getParamType (String aConv) throws RemoteException;
    public String getParamType (short aConv) throws RemoteException;
    public HashMap<Short,String> getParamType() throws RemoteException;
    
    public short getTreeState (String aConv) throws RemoteException;
    public String getTreeState (short aConv) throws RemoteException;
    public HashMap<Short,String> getTreeState() throws RemoteException;
    
    public short getTreeType (String aConv) throws RemoteException;
    public String getTreeType (short aConv) throws RemoteException;
    public HashMap<Short,String> getTreeType() throws RemoteException;
    
    public short getUnit (String aConv) throws RemoteException;
    public String getUnit (short aConv) throws RemoteException;
    public HashMap<Short,String> getUnit() throws RemoteException;
}
