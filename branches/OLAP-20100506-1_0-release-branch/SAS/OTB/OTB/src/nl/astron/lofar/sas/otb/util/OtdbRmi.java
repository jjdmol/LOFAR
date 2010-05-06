/*
 * OtdbRmi.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otb.util;

import java.rmi.Naming;
import java.util.TreeMap;
import nl.astron.lofar.lofarutils.remoteFileInterface;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.exceptions.ConnectionFailedException;
import nl.astron.lofar.sas.otb.exceptions.NoAccessException;
import nl.astron.lofar.sas.otb.jotdb3.jCampaignInterface;
import nl.astron.lofar.sas.otb.jotdb3.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBaccessInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb3.jTreeMaintenanceInterface;
import nl.astron.lofar.sas.otb.jotdb3.jTreeValueInterface;

import nl.astron.lofar.sas.otbcomponents.SetServerDialog;
import org.apache.log4j.Logger;

/**
 * This class provides the interface with the OTDB server.
 *
 * @see nl.astron.lofar.sas.otb.jotdb2
 *
 * @created 16-01-2006, 16:33
 *
 * @author blaakmeer
 *
 * @version $Id$
 *
 * @updated coolen 16-06-2006,  added support for remotefiletransfer
 * @updated coolen 27-02-2007,  added support for server/port setting
 */
public class OtdbRmi {
    
    static Logger logger = Logger.getLogger(OtdbRmi.class);
    static String name = "OtdbRmi";
    
    
    private static String RMIServerName      = "lofar17.astron.nl";
    private static String RMIServerPort      = "1099";

    private static String RMIAccessName      = jOTDBaccessInterface.SERVICENAME;
    private static String RMIRegistryName    = "";


    private static boolean isOpened         = false;
    private static boolean isConnected      = false;
    private MainFrame itsMainFrame;
    
     // RMI interfaces
    private static jOTDBaccessInterface remoteOTDBaccess;
    private static jOTDBinterface remoteOTDB;
    private static jCampaignInterface remoteCampaign;
    private static jTreeMaintenanceInterface remoteMaintenance;
    private static jTreeValueInterface remoteValue;
    private static jConverterInterface remoteTypes;
    private static remoteFileInterface remoteFileTrans;
    
    private static TreeMap<Short,String> itsClassifs;
    private static TreeMap<Short,String> itsParamTypes;
    private static TreeMap<Short,String> itsTreeStates;
    private static TreeMap<Short,String> itsTreeTypes;
    private static TreeMap<Short,String> itsUnits;
    
    /** Creates a new instance of OtdbRmi */
    public OtdbRmi(MainFrame mainFrame) {
        itsMainFrame=mainFrame;
        isOpened=false;
        isConnected=false;
        setServer();
    }
    
    public void setServer() {
        SetServerDialog serverDialog = new SetServerDialog(itsMainFrame,true);
        serverDialog.setLocationRelativeTo(itsMainFrame);
        serverDialog.setVisible(true);
        if(serverDialog.isOk()) {
            RMIServerName = serverDialog.getServer();
            RMIServerPort = serverDialog.getPort();
            itsMainFrame.setServer(RMIServerName);
            itsMainFrame.setPort(RMIServerPort);
        }
    }
    
    /**
     * Getter for property RMIServerName.
     * @return Value of property RMIServerName.
     */
    public static String getRMIServerName() {
        return RMIServerName;
    }

    /**
     * Getter for property RMIServerPort.
     * @return Value of property RMIServerPort.
     */
    public static String getRMIServerPort() {
        return RMIServerPort;
    }

    /**
     * Getter for property RMIRegistryName.
     * @return Value of property RMIRegistryName.
     */
    public static String getRMIAccessName() {
        return RMIAccessName;
    }

    /**
     * Getter for property RMIRegistryName.
     * @return Value of property RMIRegistryName.
     */
    public static String getRMIRegistryName() {
        return RMIRegistryName;
    }

    /**
     * Setter for property RMIRegistryName.
     */
    public static void setRMIRegistryName(String aName) {
        OtdbRmi.RMIRegistryName=aName;
    }



    /**
     * Getter for property remoteMaintenance.
     * @return Value of property remoteMaintenance.
     */
    public static jTreeMaintenanceInterface getRemoteMaintenance() {
        return remoteMaintenance;
    }

    /**
     * Getter for property remoteCampaign.
     * @return Value of property remoteCampaign.
     */
    public static jCampaignInterface getRemoteCampaign() {
        return remoteCampaign;
    }

    /**
     * Getter for property remoteOTDB.
     * @return Value of property remoteOTDB.
     */
    public static jOTDBinterface getRemoteOTDB() {

        return OtdbRmi.remoteOTDB;
    }

    /**
     * Getter for property remoteOTDBaccess.
     * @return Value of property remoteOTDBaccess.
     */
    public static jOTDBaccessInterface getRemoteOTDBaccess() {

        return OtdbRmi.remoteOTDBaccess;
    }

    /**
     * Getter for property remoteValue.
     * @return Value of property remoteValue.
     */
    public static jTreeValueInterface getRemoteValue() {

        return OtdbRmi.remoteValue;
    }

    /**
     * Getter for property remoteFileTrans.
     * @return Value of property remoteValue.
     */
    public static remoteFileInterface getRemoteFileTrans() {

        return OtdbRmi.remoteFileTrans;
    }

    /**
     * Getter for property remoteTypes.
     * @return Value of property remoteTypes.
     */
    public static jConverterInterface getRemoteTypes() {

        return OtdbRmi.remoteTypes;
    }
    
    /**
     * Getter for property itsClassifs.
     * @return Value of property itsClassifs.
     */
    public static TreeMap<Short,String> getClassif() {

        return OtdbRmi.itsClassifs;
    }

    /**
     * Getter for property itsParamTypes.
     * @return Value of property itsParamTypes.
     */
    public static TreeMap<Short,String> getParamType() {

        return OtdbRmi.itsParamTypes;
    }

        /**
     * Getter for property itsTreeStates.
     * @return Value of property itsTreeStates.
     */
    public static TreeMap<Short,String> getTreeState() {

        return OtdbRmi.itsTreeStates;
    }

        /**
     * Getter for property itsTreeTypes.
     * @return Value of property itsTreeTypes.
     */
    public static TreeMap<Short,String> getTreeType() {

        return OtdbRmi.itsTreeTypes;
    }

        /**
     * Getter for property itsUnits.
     * @return Value of property itsUnits.
     */
    public static TreeMap<Short,String> getUnit() {

        return OtdbRmi.itsUnits;
    }

    public boolean isConnected() {
        return isConnected;
    }
    
    public boolean isOpened() {
        return isOpened;
    }

    public boolean openAccessConnection() throws NoAccessException {
        String aRa="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIAccessName;
        isOpened=openRemoteAccess(aRa);
        if (isOpened){
            logger.debug("Remote access connection opened");
            isConnected=true;
        } else {
            throw new NoAccessException("Couldn't open remote access object: "+aRa);
        }
        return isConnected;
    }
    
    public static boolean openConnections() throws ConnectionFailedException {
        if (openRemoteConnection() && openRemoteMaintenance() && openRemoteValue() && openRemoteConverter() && openRemoteFile()&& openRemoteCampaign()) {
            logger.debug("Remote connections opened");
            isConnected=true;
        } else {
            throw new ConnectionFailedException("Couldn't open all connections");
        }
        return isConnected;
    }

    private boolean openRemoteAccess(String RMIRegHostName) {
        try {
            logger.debug("openRemoteAccess for "+RMIRegHostName);

            // create a remote object
            remoteOTDBaccess = (jOTDBaccessInterface) Naming.lookup (RMIRegHostName);
            logger.debug(remoteOTDBaccess);

	    logger.debug("Connection to RemoteAccess succesful!");
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Open Remote Access via RMI and JNI failed: " + e);
	  }
        return false;
    }

    private static boolean openRemoteConnection() {
        try {
            logger.debug("openRemoteConnection for jOTDB_"+RMIRegistryName);
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+"jOTDB_"+RMIRegistryName;
        
            // create a remote object
            remoteOTDB = (jOTDBinterface) Naming.lookup (aRegName);
            logger.debug(remoteOTDB);
					    
	    // do the test	
	    logger.debug("Trying to connect to the database");
            boolean c = remoteOTDB.connect();
	    assert c : "Connection failed";
            c = remoteOTDB.isConnected();
	    assert c : "Connnection flag failed";
	     
	    logger.debug("Connection succesful!");   
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Open Remote Connection via RMI and JNI failed: " + e);
	  }
        return false;
    }   
    
    private static boolean openRemoteMaintenance() {
        try {
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+ "jTreeMaintenance_" + RMIRegistryName;
            logger.debug("openRemoteMaintenance for "+aRegName);
        
            // create a remote object
            remoteMaintenance = (jTreeMaintenanceInterface) Naming.lookup (aRegName);
            logger.debug(remoteMaintenance);

     	    logger.debug("Connection succesful!");
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Getting Remote Maintenance via RMI and JNI failed: " + e);
	  }
        return false;
    }
        
    private static boolean openRemoteCampaign() {
        try {
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+ "jCampaign_" + RMIRegistryName;
            logger.debug("openRemoteCampaign for "+aRegName);

            // create a remote object
            remoteCampaign = (jCampaignInterface) Naming.lookup (aRegName);
            logger.debug(remoteCampaign);

     	    logger.debug("Connection succesful!");
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Getting Remote Campaign via RMI and JNI failed: " + e);
	  }
        return false;
    }

    private static boolean openRemoteValue() {
        try {
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+ "jTreeValue_" + RMIRegistryName;
            logger.debug("OpenRemoteValue for "+aRegName);
        
            // create a remote object
            remoteValue = (jTreeValueInterface) Naming.lookup (aRegName);
            logger.debug(remoteValue);
					    
     	    logger.debug("Connection succesful!");   
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Getting Remote Value via RMI and JNI failed: " + e);
	  }
        return false;
    }

    private static boolean openRemoteConverter() {
        try {
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+ "jConverter_" + RMIRegistryName;
            logger.debug("openRemoteConverter for "+aRegName);
        
            // create a remote object
            remoteTypes = (jConverterInterface) Naming.lookup (aRegName);
            logger.debug(remoteValue);
					    
     	    logger.debug("Connection succesful!");  
            if (loadConversionTypes() ) {
                return true;
            }
          }
        catch (Exception e)
	  {
	   logger.debug("Getting remote Converter via RMI and JNI failed: " + e);
	  }
        return false;
    }
    
    private static boolean openRemoteFile() {
        try {
            String aRegName = "rmi://"+RMIServerName+":"+RMIServerPort+"/"+ "remoteFile_" + RMIRegistryName;
            logger.debug("OpenRemoteFile for "+aRegName);
        
            // create a remote object
            remoteFileTrans = (remoteFileInterface) Naming.lookup (aRegName);
            logger.debug(remoteFileTrans);
					    
     	    logger.debug("Connection succesful!");   
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Getting RemoteFileTransfer via RMI and JNI failed: " + e);
	  }
        return false;
    }

    private static boolean loadConversionTypes() {
        try {
            logger.debug("Get ConversionTypes");
            itsClassifs   =new TreeMap<Short,String>(OtdbRmi.remoteTypes.getClassif());
            itsParamTypes =new TreeMap<Short,String>(OtdbRmi.remoteTypes.getParamType());
            itsTreeStates =new TreeMap<Short,String>(OtdbRmi.remoteTypes.getTreeState());
            itsTreeTypes  =new TreeMap<Short,String>(OtdbRmi.remoteTypes.getTreeType());
            itsUnits      =new TreeMap<Short,String>(OtdbRmi.remoteTypes.getUnit());
            logger.debug("Got all conversiontypes");
            return true;
        } catch (Exception e) {
            logger.debug("Getting ConversionTypes via RMI and JNI failed");
        }
        return false;
    }
}
