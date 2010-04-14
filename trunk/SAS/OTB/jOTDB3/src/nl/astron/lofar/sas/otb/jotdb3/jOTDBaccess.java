//#  jOTDBconnection.java: Manages the connection with the OTDB database.
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
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import nl.astron.lofar.lofarutils.remoteFile;
import nl.astron.lofar.lofarutils.remoteFileInterface;
import org.apache.log4j.Logger;


public class jOTDBaccess implements jOTDBaccessInterface
{
    
    private Logger logger = Logger.getLogger(jOTDBaccess.class);
    private static int seqNr = 0;
    private String itsDBname="";
    private String itsDBpwd="";
    private String itsDB="";
    private String itsDBhost="";
    private int itsRMIobjectPort=0;
    private int itsRMIport = 0;
    private Registry itsLocalRegistry = null;

    // these vars need to be static otherwise the server will destroy them via garbage collection
    // no idea however if by doing that here it will overwrite them for every new user....
    private static jOTDBconnection connection;
    private static jTreeMaintenanceInterface treeMaintenance;
    private static jCampaignInterface campaign;
    private static jTreeValueInterface treeValue;
    private static jConverterInterface converter;
    private static remoteFileInterface aRemoteFile;

    static
    {
        System.loadLibrary("jotdb3");
    }

   public jOTDBaccess( String aDBhost, int anRMIport, int anRMIobjectPort,Registry aLocalRegistry) throws RemoteException
   {
       itsDBhost=aDBhost;
       itsRMIport=anRMIport;
       itsRMIobjectPort=anRMIobjectPort;
       itsLocalRegistry=aLocalRegistry;
       // each server has an unique rmi port, so make seqNr equal 
       // to that to obtain more or less unique name_nr names
       seqNr=itsRMIport;
   }
   
   
    // To connect or reconnect in case the connection was lost
    public String login(String name,String pwd, String dbName) throws RemoteException {
        seqNr+=1;
        String nameExtention = name+"_"+Integer.toString(seqNr);
        logger.info("jOTDBaccess: login");
                
        if (!exportConnection(nameExtention, name, pwd, dbName)) {
            seqNr-=1;
            logger.fatal("Error opening jOTDBconnection");
            return "";
        } else {
            logger.debug("jOTDBconnection opened to "+itsDBhost+" Database: "+dbName);
        }

        if (! exportTreeMaintenance(nameExtention)) {
            seqNr-=1;
            logger.fatal("Error opening jTreeMaintenance");
            return "";
        } else {
            logger.debug("jTreeMaintenance opened to "+itsDBhost+" Database: "+dbName);
        }

        if (! exportCampaign(nameExtention)) {
            seqNr-=1;
            logger.fatal("Error opening jCampaign");
            return "";
        } else {
            logger.debug("jCampaign opened to "+itsDBhost+" Database: "+dbName);
        }

        if (! exportTreeValue(nameExtention)) {
            seqNr-=1;
            logger.fatal("Error opening jTreeValue");
            return "";
        } else {
            logger.debug("jTreeValue opened to "+itsDBhost+" Database: "+dbName);
        }

        if (! exportConverter(nameExtention)) {
            seqNr-=1;
            logger.fatal("Error opening converter ");
            return "";
        } else {
            logger.debug("converter opened to "+itsDBhost+" Database: "+dbName);
        }

        if (! exportRemoteFile(nameExtention)) {
            seqNr-=1;
            logger.fatal("Error opening remoteFile ");
            return "";
        } else {
            logger.debug("remoteFile opened to "+itsDBhost+" Database: "+dbName);
        }

        return nameExtention;

    }
    
    private boolean exportConnection(String ext,String name, String pwd, String dbName) {
        String serviceName=jOTDBinterface.SERVICENAME+"_"+ext;

        try {
            // Export jOTDBconnection
//            connection = new jOTDBconnection(name, pwd, dbName,itsDBhost);
            connection = new jOTDBconnection("paulus", "boskabouter", dbName,itsDBhost,ext);

            //A custom port was specified, export the object using the port specified
            jOTDBinterface stub =
                    (jOTDBinterface) UnicastRemoteObject.exportObject((jOTDBinterface)connection, itsRMIport);
            if (stub != null) {
                logger.info("set up new jOTDBconnection: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published jOTDBinterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new jOTDBconnection FAILED");
                return false ;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

    private boolean exportTreeMaintenance(String ext) {
        String serviceName=jTreeMaintenanceInterface.SERVICENAME+"_"+ext;

        try {
            // Export jTreeMaintenance
            treeMaintenance = new jTreeMaintenance(ext);

            //A custom port was specified, export the object using the port specified
            jTreeMaintenanceInterface stub =
                    (jTreeMaintenanceInterface) UnicastRemoteObject.exportObject(treeMaintenance, itsRMIport);
            if (stub != null) {
                logger.info("set up new jTreeMaintenance: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published jTreeMaintenanceInterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new jTreeMaintenance FAILED");
                return false;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

    private boolean exportCampaign(String ext) {
        String serviceName=jCampaignInterface.SERVICENAME+"_"+ext;

        try {
            // Export jCampaign
            campaign = new jCampaign(ext);

            //A custom port was specified, export the object using the port specified
            jCampaignInterface stub =
                    (jCampaignInterface) UnicastRemoteObject.exportObject(campaign, itsRMIport);
            if (stub != null) {
                logger.info("set up new jCampaign: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published jCampaignInterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new jCampaign FAILED");
                return false;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

    private boolean exportTreeValue(String ext) {
        String serviceName=jTreeValueInterface.SERVICENAME+"_"+ext;

        try {
            // Export jTreeValue
            treeValue = new jTreeValue(ext);

            //A custom port was specified, export the object using the port specified
            jTreeValueInterface stub =
                    (jTreeValueInterface) UnicastRemoteObject.exportObject(treeValue, itsRMIport);
            if (stub != null) {
                logger.info("set up new jTreeValue: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published jTreeValueInterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new jTreeMaintenance FAILED");
                return false;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

    private boolean exportConverter(String ext) {
        String serviceName=jConverterInterface.SERVICENAME+"_"+ext;

        try {
            // Export jConverter
            converter = new jConverter(ext);

            //A custom port was specified, export the object using the port specified
            jConverterInterface stub =
                    (jConverterInterface) UnicastRemoteObject.exportObject(converter, itsRMIport);
            if (stub != null) {
                logger.info("set up new jConverter: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published jConverterInterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new jTreeMaintenance FAILED");
                return false;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

    private boolean exportRemoteFile(String ext) {
        String serviceName=remoteFileInterface.SERVICENAME+"_"+ext;

        try {
            // Export remoteFile
            aRemoteFile = new remoteFile(ext);

            //A custom port was specified, export the object using the port specified
            remoteFileInterface stub =
                    (remoteFileInterface) UnicastRemoteObject.exportObject(aRemoteFile, itsRMIport);
            if (stub != null) {
                logger.info("set up new remoteFile: " + stub);
                logger.info("jOTDBserver publishing service " + serviceName + " in local registry...");
                itsLocalRegistry.rebind(serviceName, stub);
                logger.info("Published remoteFileInterface as service " + serviceName + ". Ready...");
                return true;
            } else {
                logger.info("set up new remoteFileInterface FAILED");
                return false;
            }
        } catch (RemoteException ex) {
            logger.fatal("RMI login failed " + ex);
            return false;
        }
    }

}
