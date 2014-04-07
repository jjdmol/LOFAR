//#  jTreeValue.java: Interface for access to the tree (KVT) values
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


import java.rmi.RemoteException;
import java.util.Vector;

public class jTreeValue implements jTreeValueInterface
{

    public String itsErrorMsg;
    public int itsTreeID;
    private String itsName = "";
 
    public jTreeValue (String ext)
    {
        itsName=ext;
    	itsErrorMsg="";
        itsTreeID=0;
    }

    public void setTreeID(int aTreeID) {
        itsTreeID=aTreeID;
    }

    // PVSS will continuously add value-changes to the offline PIC.
    // There two ways PVSS can do this.
    // The function returns false if the PIC node can not be found.
    public native boolean addKVT( String key, String value, String time) throws RemoteException;
    public native boolean addKVT(jOTDBvalue aKVT) throws RemoteException;

    // Note: This form will probably be used by SAS and OTB when committing
    // a list of modified node.
    public native boolean addKVTlist(Vector<jOTDBvalue> aValueList) throws RemoteException;
    //    public native boolean addKVTparamSet(jParamterSet aPS) throws Exception;

    //# SHM queries
    // With searchInPeriod a list of all valuechanges in the OTDB tree can
    // be retrieved from the database.
    // By chosing the topItem right one node or a sub tree of the whole tree
    // (you probably don't want this!) can be retrieved.
    // When the endDate is not specified all value changes from beginDate
    // till 'now' are retrieved, otherwise the selection is limited to
    // [beginDate..endDate>.
    public native Vector<jOTDBvalue> searchInPeriod (int topNode, 
						     int depth, 
						     String beginDate, 
						     String endDate, 
						     boolean mostRecentlyOnly) throws RemoteException;

    //# SAS queries
    // For scheduling the VIC tree on the OTDB tree SAS must know what
    // resources exist in the OTDB tree. This list can be retrieved with
    // this function.
    // TBW: Is this realy what SAS needs???
    public native Vector<jOTDBvalue> getSchedulableItems (int topNode) throws RemoteException;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public String  errorMsg() {
        return itsErrorMsg;
    }
}
