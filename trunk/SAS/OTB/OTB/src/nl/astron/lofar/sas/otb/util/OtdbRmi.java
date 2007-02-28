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
import nl.astron.lofar.sas.otb.jotdb2.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb2.jTreeMaintenanceInterface;
import nl.astron.lofar.sas.otb.jotdb2.jTreeValueInterface;
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
    
    
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "1099";

    public static String RMIRegistryName    = jOTDBinterface.SERVICENAME;
    public static String RMIMaintenanceName = jTreeMaintenanceInterface.SERVICENAME;
    public static String RMIValName         = jTreeValueInterface.SERVICENAME;
    public static String RMIConverterName   = jConverterInterface.SERVICENAME; 
    public static String RMIRemoteFileName  = remoteFileInterface.SERVICENAME;
    
    private static boolean isOpened         = false;
    private static boolean isConnected      = false;
    private MainFrame itsMainFrame;
    
     // RMI interfaces
    private static jOTDBinterface remoteOTDB;    
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
     * Getter for property remoteMaintenance.
     * @return Value of property remoteMaintenance.
     */
    public jTreeMaintenanceInterface getRemoteMaintenance() {
        return remoteMaintenance;
    }
    /**
     * Getter for property remoteOTDB.
     * @return Value of property remoteOTDB.
     */
    public jOTDBinterface getRemoteOTDB() {

        return this.remoteOTDB;
    }

    /**
     * Getter for property remoteValue.
     * @return Value of property remoteValue.
     */
    public jTreeValueInterface getRemoteValue() {

        return this.remoteValue;
    }

    /**
     * Getter for property remoteFileTrans.
     * @return Value of property remoteValue.
     */
    public remoteFileInterface getRemoteFileTrans() {

        return this.remoteFileTrans;
    }

    /**
     * Getter for property remoteTypes.
     * @return Value of property remoteTypes.
     */
    public jConverterInterface getRemoteTypes() {

        return this.remoteTypes;
    }
    
    /**
     * Getter for property itsClassifs.
     * @return Value of property itsClassifs.
     */
    public TreeMap<Short,String> getClassif() {

        return this.itsClassifs;
    }

    /**
     * Getter for property itsParamTypes.
     * @return Value of property itsParamTypes.
     */
    public TreeMap<Short,String> getParamType() {

        return this.itsParamTypes;
    }

        /**
     * Getter for property itsTreeStates.
     * @return Value of property itsTreeStates.
     */
    public TreeMap<Short,String> getTreeState() {

        return this.itsTreeStates;
    }

        /**
     * Getter for property itsTreeTypes.
     * @return Value of property itsTreeTypes.
     */
    public TreeMap<Short,String> getTreeType() {

        return this.itsTreeTypes;
    }

        /**
     * Getter for property itsUnits.
     * @return Value of property itsUnits.
     */
    public TreeMap<Short,String> getUnit() {

        return this.itsUnits;
    }

    public boolean isConnected() {
        return isConnected;
    }
    
    public boolean isOpened() {
        return isOpened;
    }

    public boolean openConnections() {
        String aRC="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRegistryName;
        String aRMS="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIMaintenanceName;
        String aRMV="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIValName;
        String aRMC="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIConverterName;
        String aRFI="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRemoteFileName;
        

        isOpened=openRemoteConnection(aRC);
        if (isOpened){
            if (openRemoteMaintenance(aRMS) && openRemoteValue(aRMV) && openRemoteConverter(aRMC) && openRemoteFile(aRFI)) {  
                logger.debug("Remote connections opened");
                isConnected=true;
            } else {
                logger.debug("Error opening remote connections");           
            }
        } else {
            logger.debug("Failed to open remote connection");
        }
        return isConnected;
    }
    
    private boolean openRemoteConnection(String RMIRegHostName) {
        try {
            logger.debug("openRemoteConnection for "+RMIRegHostName);
        
            // create a remote object
            remoteOTDB = (jOTDBinterface) Naming.lookup (RMIRegHostName);     
            logger.debug(remoteOTDB);
					    
	    // do the test	
	    logger.debug("Trying to connect to the database");
	    assert remoteOTDB.connect() : "Connection failed";	
	    assert remoteOTDB.isConnected() : "Connnection flag failed";
	     
	    logger.debug("Connection succesful!");   
            return true;
          }
        catch (Exception e)
	  {
	     logger.debug("Open Remote Connection via RMI and JNI failed: " + e);
	  }
        return false;
    }   
    
    private boolean openRemoteMaintenance(String RMIMaintName) {
        try {
            logger.debug("openRemoteMaintenance for "+RMIMaintName);
        
            // create a remote object
            remoteMaintenance = (jTreeMaintenanceInterface) Naming.lookup (RMIMaintName);     
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
    
        
    private boolean openRemoteValue(String RMIValName) {
        try {
            logger.debug("OpenRemoteValue for "+RMIValName);
        
            // create a remote object
            remoteValue = (jTreeValueInterface) Naming.lookup (RMIValName);     
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

    private boolean openRemoteConverter(String RMIConverterName) {
        try {
            logger.debug("openRemoteConverter for "+RMIConverterName);
        
            // create a remote object\
            remoteTypes = (jConverterInterface) Naming.lookup (RMIConverterName); 
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
    
    private boolean openRemoteFile(String RMIRemoteFileName) {
        try {
            logger.debug("OpenRemoteFile for "+RMIRemoteFileName);
        
            // create a remote object
            remoteFileTrans = (remoteFileInterface) Naming.lookup (RMIRemoteFileName);     
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

    private boolean loadConversionTypes() {
        try {
            logger.debug("Get ConversionTypes");
            itsClassifs   =new TreeMap<Short,String>(this.remoteTypes.getClassif());
            itsParamTypes =new TreeMap<Short,String>(this.remoteTypes.getParamType());
            itsTreeStates =new TreeMap<Short,String>(this.remoteTypes.getTreeState());
            itsTreeTypes  =new TreeMap<Short,String>(this.remoteTypes.getTreeType());
            itsUnits      =new TreeMap<Short,String>(this.remoteTypes.getUnit());
            logger.debug("Got all conversiontypes");
            return true;
        } catch (Exception e) {
            logger.debug("Getting ConversionTypes via RMI and JNI failed");
        }
        return false;
    }


    
}
