//#  jCampaignAdapter.java: The RMI adapter of the Campaign database.
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



public class jCampaignAdapter extends UnicastRemoteObject implements jCampaignInterface
{

   // Create a Log4J logger instance
   static Logger logger = Logger.getLogger(jCampaignAdapter.class);

   // Constructor
   public jCampaignAdapter (jCampaign adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
      
    // Get one campaign record.
    public jCampaignInfo   getCampaign(String name) throws RemoteException {
        jCampaignInfo aC=null;
        try {
            aC = adaptee.getCampaign (name);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getCampaign(name)",ex);
            throw anEx;
        }
        return aC;
    }

    public jCampaignInfo   getCampaign(int ID) throws RemoteException {
        jCampaignInfo aC=null;
        try {
            aC = adaptee.getCampaign (ID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getCampaign(ID)",ex);
            throw anEx;
        }
        return aC;
    }

    // Get all campaign records
    public Vector<jCampaignInfo> getCampaignList() throws RemoteException {
        Vector<jCampaignInfo> aV=null;
        try {
            aV = adaptee.getCampaignList();
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getCampaignList() error",ex);
            throw anEx;
        }
        return aV;

    }

    // Update or insert a campaign record
    public int saveCampaign(jCampaignInfo aCampaign) throws RemoteException {
        int anI;
        try {
            anI = adaptee.saveCampaign(aCampaign);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI saveCampaign error",ex);
            throw anEx;
        }
        return anI;

    }
    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public String errorMsg() throws RemoteException {
        String aS=null;
        try {
            aS = adaptee.errorMsg();
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI errorMsg error",ex);
            throw anEx;            
        }
        return aS;            
     }

   protected jCampaign adaptee;
}
