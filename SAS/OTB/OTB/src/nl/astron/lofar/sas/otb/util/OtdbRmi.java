/*
 * OtdbRmi.java
 *
 * Created on January 16, 2006, 4:33 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import nl.astron.lofar.sas.otb.jotdb2.*;
import java.rmi.Naming;
import java.util.TreeMap;
import nl.astron.lofar.sas.otb.panels.MainPanel;
import org.apache.log4j.Logger;

/**
 *
 * @author blaakmeer
 */
public class OtdbRmi {
    
    static Logger logger = Logger.getLogger(MainPanel.class);
    static String name = "OtdbRmi";
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "1091";
    public static String RMIRegistryName    = jOTDBinterface.SERVICENAME;
    public static String RMIMaintenanceName = jTreeMaintenanceInterface.SERVICENAME;
    public static String RMIValName         = jTreeValueInterface.SERVICENAME;
    public static String RMIConverterName   = jConverterInterface.SERVICENAME; 
    public static String OTDBUserName       = "paulus";
    public static String OTDBPassword       = "boskabouter";
    public static String OTDBDBName         = "coolen";
    
    private static boolean isOpened         = false;
    private static boolean isConnected      = false;
    
     // RMI interfaces
    private static jOTDBinterface remoteOTDB;    
    private static jTreeMaintenanceInterface remoteMaintenance;
    private static jTreeValueInterface remoteValue;
    private static jConverterInterface remoteTypes;
    
    private static TreeMap<Short,String> itsClassifs;
    private static TreeMap<Short,String> itsParamTypes;
    private static TreeMap<Short,String> itsTreeStates;
    private static TreeMap<Short,String> itsTreeTypes;
    private static TreeMap<Short,String> itsUnits;
    
    /** Creates a new instance of OtdbRmi */
    public OtdbRmi() {          
        isOpened=false;
        isConnected=false;
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

        isOpened=openRemoteConnection(aRC);
        if (isOpened){
            if (openRemoteMaintenance(aRMS) && openRemoteValue(aRMV) && openRemoteConverter(aRMC)) {  
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
