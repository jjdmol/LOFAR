//#  jOTDBserver.java: The RMI server of the OTDB database.
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
import java.rmi.registry.*;
import java.rmi.server.RMISocketFactory;
import nl.astron.lofar.lofarutils.remoteFileAdapter;
import nl.astron.lofar.lofarutils.remoteFileInterface;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;



public class jOTDBserver {
    private static jOTDBconnection jOTDBconnAdaptee;
    private static jOTDBadapter jOTDBconnAdapter;
    private static jTreeMaintenance jTreeMainAdaptee;
    private static jTreeMaintenanceAdapter jTreeMainAdapter;
    private static jTreeValue jTreeValueAdaptee;
    private static jTreeValueAdapter jTreeValueAdapter;
    private static jConverter jConverterAdaptee;
    private static jConverterAdapter jConverterAdapter;
    private static remoteFileAdapter remoteFileAdapter;
    
    static
    {
        System.loadLibrary("jotdb2");
    }
    
    static Logger logger = Logger.getLogger(jOTDBserver.class);
    
    public static void main(String[] args)  {
        try {
            String logConfig = "jOTDB2.log_prop";
            
            PropertyConfigurator.configure(logConfig);
            jInitCPPLogger aCPPLogger=new jInitCPPLogger(logConfig);
            logger.info("jOTDBServer started. LogPropFile: "+ logConfig);
            
//	     if (System.getSecurityManager () == null)
//	       {
//		  System.out.println ("No security mananger is running, will start one now...");
//		  System.setSecurityManager (new RMISecurityManager ());
//	       }
            
            if (args.length < 5) {
                System.out.println("Usage: java -jar jOTDBserver.jar <username> <password> <database> <db-hostname> <hostname> <rmiportnumber-OPTIONAL> <rmi objects portnumber for firewall/tunneling purposes-OPTIONAL>");
                System.exit(0);
            }
            
            
            String aHostName = new String(args[4]);
            
            logger.info("Running on: "+aHostName);
            System.setProperty("java.rmi.server.hostname", aHostName);
            
            
            Registry localRegistry = null;
            int objectPort = 0;
            
            if (args.length == 5){
                logger.info("jOTDBserver creating a local RMI registry on port "+Registry.REGISTRY_PORT+" ...");
                localRegistry = LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
            }else if (args.length > 5) {
                Integer rmiPort = new Integer(args[5]);
                logger.info("jOTDBserver creating a local RMI registry on port "+rmiPort+" ...");
                localRegistry = LocateRegistry.createRegistry(rmiPort.intValue());
                if (args.length ==7){
                    Integer rmiObjectsPort = new Integer(args[6]);
                    logger.info("jOTDBserver setting up RMI server objects on port "+rmiObjectsPort+" ...");
//                    RMISocketFactory socketFactory = RMISocketFactory.getDefaultSocketFactory();
//                    socketFactory.createServerSocket(rmiObjectsPort);
                    objectPort = rmiObjectsPort;
                }
            }
            
            logger.info("jOTDBserver creating local object and remote adapter...");
            
            // Export jOTDBconnection
            jOTDBconnAdaptee = new jOTDBconnection(args[0], args[1], args[2], args[3]);
            jOTDBconnAdapter = new jOTDBadapter(jOTDBconnAdaptee);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                jOTDBconnAdapter.unexportObject(jOTDBconnAdapter,true);
                jOTDBconnAdapter.exportObject(jOTDBconnAdapter,objectPort);
            }
            logger.info("jOTDBserver publishing service " + jOTDBinterface.SERVICENAME + " in local registry...");
            
            localRegistry.rebind(jOTDBinterface.SERVICENAME, jOTDBconnAdapter);
            
            logger.info("Published jOTDBinterface as service " + jOTDBinterface.SERVICENAME + ". Ready...");
            
            // Export jTreeMaintenance
            jTreeMainAdaptee = new jTreeMaintenance();
            jTreeMainAdapter = new jTreeMaintenanceAdapter(jTreeMainAdaptee);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                jTreeMainAdapter.unexportObject(jTreeMainAdapter,true);
                jTreeMainAdapter.exportObject(jTreeMainAdapter,objectPort);
            }
            logger.info("jOTDBserver publishing service " + jTreeMaintenanceInterface.SERVICENAME + " in local registry...");
            localRegistry.rebind(jTreeMaintenanceInterface.SERVICENAME, jTreeMainAdapter);
            
            // Export jTreeValue
            jTreeValueAdaptee = new jTreeValue();
            jTreeValueAdapter = new jTreeValueAdapter(jTreeValueAdaptee);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                jTreeValueAdapter.unexportObject(jTreeValueAdapter,true);
                jTreeValueAdapter.exportObject(jTreeValueAdapter,objectPort);
            }
            logger.info("jOTDBserver publishing service " + jTreeValueInterface.SERVICENAME + " in local registry...");
            localRegistry.rebind(jTreeValueInterface.SERVICENAME, jTreeValueAdapter);
            
            logger.info("Published jTreeValueInterface as service " + jTreeValueInterface.SERVICENAME + ". Ready...");
            
            // Export jConverter
            jConverterAdaptee = new jConverter();
            jConverterAdapter = new jConverterAdapter(jConverterAdaptee);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                jConverterAdapter.unexportObject(jConverterAdapter,true);
                jConverterAdapter.exportObject(jConverterAdapter,objectPort);
            }
            logger.info("jOTDBserver publishing service " + jConverterInterface.SERVICENAME + " in local registry...");
            localRegistry.rebind(jConverterInterface.SERVICENAME, jConverterAdapter);
            
            logger.info("Published jConverterInterface as service " + jConverterInterface.SERVICENAME + ". Ready...");

            // Export remoteFile
            remoteFileAdapter = new remoteFileAdapter(remoteFileInterface.SERVICENAME);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                remoteFileAdapter.unexportObject(remoteFileAdapter,true);
                remoteFileAdapter.exportObject(remoteFileAdapter,objectPort);
            }
            logger.info("jOTDBserver publishing service " + remoteFileInterface.SERVICENAME + " in local registry...");
            localRegistry.rebind(remoteFileInterface.SERVICENAME, remoteFileAdapter);
            
            logger.info("Published remoteFileInterface as service " + remoteFileInterface.SERVICENAME + ". Ready...");
            
            
            
            
            
            String statusmessage = "jOTDBserver is ready for incoming calls";
            if (args.length > 5) {
                Integer rmiPort = new Integer(args[5]);
                statusmessage += " on rmi registry port "+rmiPort;
                if (args.length ==7){
                    Integer rmiObjectsPort = new Integer(args[6]);
                    statusmessage += " and rmi server object port "+rmiObjectsPort +". Please tunnel/forward both ports for your client to work";
                }
            }
            statusmessage+="...";
            
            logger.info(statusmessage);
        }
        
        catch (Exception e) {
            //e.printStackTrace();
            logger.fatal("jOTDB server failed: " + e);
        }
    }
}
