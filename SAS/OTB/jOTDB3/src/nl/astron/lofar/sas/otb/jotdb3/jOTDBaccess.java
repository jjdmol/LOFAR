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

import java.rmi.NoSuchObjectException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
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
    private String itsRMIhost="";
    private int itsRMIobjectPort=0;
    private int itsRMIport = 0;
    private Registry itsLocalRegistry = null;

    // these vars need to be static otherwise the server will destroy them via garbage collection
    // no idea however if by doing that here it will overwrite them for every new user....
    private static Map<String,jOTDBconnection> connection = new HashMap<String,jOTDBconnection>();
    private static Map<String,jTreeMaintenanceInterface> treeMaintenance= new HashMap<String,jTreeMaintenanceInterface>();
    private static Map<String,jCampaignInterface> campaign= new HashMap<String,jCampaignInterface>();
    private static Map<String,jTreeValueInterface> treeValue= new HashMap<String,jTreeValueInterface>();
    private static Map<String,jConverterInterface> converter= new HashMap<String,jConverterInterface>();
    private static Map<String,remoteFileInterface> aRemoteFile= new HashMap<String,remoteFileInterface>();

    static
    {
        System.loadLibrary("jotdb3");
    }

   public jOTDBaccess( String aDBhost, String anRMIhost,int anRMIport, int anRMIobjectPort,Registry aLocalRegistry) throws RemoteException
   {
       itsDBhost=aDBhost;
       itsRMIhost=anRMIhost;
       itsRMIport=anRMIport;
       itsRMIobjectPort=anRMIobjectPort;
       itsLocalRegistry=aLocalRegistry;
       itsLocalRegistry = LocateRegistry.getRegistry(aDBhost, itsRMIport);
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


    public void logout(String name) throws RemoteException {
        logger.info("jOTDBaccess: logout");


        // Remove in backward order

        if (!unExportRemoteFile(name)) {
            logger.fatal("Error closing remoteFile");
        } else {
            logger.debug("remoteFile "+ name+" closed");
        }

        if (!unExportConverter(name)) {
            logger.fatal("Error closing jConverter");
        } else {
            logger.debug("jConverter "+ name+" closed");
        }

        if (!unExportTreeValue(name)) {
            logger.fatal("Error closing jTreeValue");
        } else {
            logger.debug("jTreeValue "+ name+" closed");
        }

        if (!unExportCampaign(name)) {
            logger.fatal("Error closing jCampaign");
        } else {
            logger.debug("jCampaign "+ name+" closed");
        }

        if (!unExportTreeMaintenance(name)) {
            logger.fatal("Error closing jTreeMaintenance");
        } else {
            logger.debug("jTreeMaintenance "+ name+" closed");
        }

        if (!unExportConnection(name)) {
            logger.fatal("Error closing jOTDBconnection");
        } else {
            logger.debug("jOTDBconnection "+ name+" closed");
        }
    }
    
    private boolean exportConnection(String ext,String name, String pwd, String dbName) {
        String serviceName=jOTDBinterface.SERVICENAME+"_"+ext;

        try {
            // Export jOTDBconnection
            jOTDBconnection aC = new jOTDBconnection("paulus", "boskabouter", dbName,itsDBhost,ext);
            connection.put(serviceName,aC);

            //A custom port was specified, export the object using the port specified
            jOTDBinterface stub =
                    (jOTDBinterface) UnicastRemoteObject.exportObject((jOTDBinterface)connection.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportConnection(String ext) {
        try {
            String serviceName = jOTDBinterface.SERVICENAME + "_" + ext;
            // get connection from mapping
            jOTDBinterface aC = connection.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aC, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                connection.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }
 

    private boolean exportTreeMaintenance(String ext) {
        String serviceName=jTreeMaintenanceInterface.SERVICENAME+"_"+ext;

        try {
            // Export jTreeMaintenance
            treeMaintenance.put(serviceName,new jTreeMaintenance(ext));

            //A custom port was specified, export the object using the port specified
            jTreeMaintenanceInterface stub =
                    (jTreeMaintenanceInterface) UnicastRemoteObject.exportObject(treeMaintenance.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportTreeMaintenance(String ext) {
        try {
            String serviceName = jTreeMaintenanceInterface.SERVICENAME + "_" + ext;
            // get TreeMaintenance from mapping
            jTreeMaintenanceInterface aTM = treeMaintenance.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aTM, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                treeMaintenance.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }

    private boolean exportCampaign(String ext) {
        String serviceName=jCampaignInterface.SERVICENAME+"_"+ext;

        try {
            // Export jCampaign
            campaign.put(serviceName, new jCampaign(ext));

            //A custom port was specified, export the object using the port specified
            jCampaignInterface stub =
                    (jCampaignInterface) UnicastRemoteObject.exportObject(campaign.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportCampaign(String ext) {
        try {
            String serviceName = jCampaignInterface.SERVICENAME + "_" + ext;
            // get Campaign from mapping
            jCampaignInterface aC = campaign.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aC, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                campaign.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }

    private boolean exportTreeValue(String ext) {
        String serviceName=jTreeValueInterface.SERVICENAME+"_"+ext;

        try {
            // Export jTreeValue
            treeValue.put(serviceName,new jTreeValue(ext));

            //A custom port was specified, export the object using the port specified
            jTreeValueInterface stub =
                    (jTreeValueInterface) UnicastRemoteObject.exportObject(treeValue.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportTreeValue(String ext) {
        try {
            String serviceName = jTreeValueInterface.SERVICENAME + "_" + ext;
            // get TreeValue from mapping
            jTreeValueInterface aTV = treeValue.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aTV, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                treeValue.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }

    private boolean exportConverter(String ext) {
        String serviceName=jConverterInterface.SERVICENAME+"_"+ext;

        try {
            // Export jConverter
            converter.put(serviceName, new jConverter(ext));

            //A custom port was specified, export the object using the port specified
            jConverterInterface stub =
                    (jConverterInterface) UnicastRemoteObject.exportObject(converter.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportConverter(String ext) {
        try {
            String serviceName = jConverterInterface.SERVICENAME + "_" + ext;
            // get Converter from mapping
            jConverterInterface aC = converter.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aC, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                converter.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }

    private boolean exportRemoteFile(String ext) {
        String serviceName=remoteFileInterface.SERVICENAME+"_"+ext;

        try {
            // Export remoteFile
            aRemoteFile.put(serviceName, new remoteFile(ext));

            //A custom port was specified, export the object using the port specified
            remoteFileInterface stub =
                    (remoteFileInterface) UnicastRemoteObject.exportObject(aRemoteFile.get(serviceName), itsRMIobjectPort);
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

    private boolean unExportRemoteFile(String ext) {
        try {
            String serviceName = remoteFileInterface.SERVICENAME + "_" + ext;
            // get TreeMaintenance from mapping
            remoteFileInterface aRF = aRemoteFile.get(serviceName);

            if (UnicastRemoteObject.unexportObject(aRF, true)) {
                logger.info("jOTDBserver removed " + serviceName + " from local registry...");
                aRemoteFile.remove(serviceName);
                return true;
            } else {
                logger.info("removing " + serviceName + " from local registry FAILED");
                return false;
            }
        } catch (NoSuchObjectException ex) {
            logger.error(ex);
            return false;
        }
    }

}
