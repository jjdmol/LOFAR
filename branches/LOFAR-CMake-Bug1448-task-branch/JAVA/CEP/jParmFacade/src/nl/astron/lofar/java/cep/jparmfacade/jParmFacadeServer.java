//#  jParmFacadeServer.java: The RMI server of the OTDB database.
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

package nl.astron.lofar.java.cep.jparmfacade;
import java.rmi.registry.*;
import java.rmi.server.RMISocketFactory;
import nl.astron.lofar.sas.otb.jotdb2.jInitCPPLogger;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;




public class jParmFacadeServer {
    private static jParmFacade jParmFacadeAdaptee;
    private static jParmFacadeAdapter jParmFacadeAdapter;
    
    static
    {
        System.loadLibrary("jparmfacade");
        System.loadLibrary("jotdb2");
    }
    
    static Logger logger = Logger.getLogger(jParmFacadeServer.class);
    
    public static void main(String[] args)  {
        try {
            String logConfig = "jParmFacade.log_prop";

            PropertyConfigurator.configure(logConfig);
            jInitCPPLogger aCPPLogger=new jInitCPPLogger(logConfig);
            logger.info("jParmFacadeServer started. LogPropFile: "+ logConfig);
            
//	     if (System.getSecurityManager () == null)
//	       {
//		  System.out.println ("No security mananger is running, will start one now...");
//		  System.setSecurityManager (new RMISecurityManager ());
//	       }
            
            if (args.length < 1) {
                System.out.println("Usage: java -jar jParmFacadeServer.jar <hostname> <rmiportnumber-OPTIONAL> <rmi objects portnumber for firewall/tunneling purposes-OPTIONAL>");
                System.exit(0);
            }
            
            
            String aHostName = new String(args[0]);
            
            logger.info("Running on: "+aHostName);
            System.setProperty("java.rmi.server.hostname", aHostName);
            
            
            Registry localRegistry = null;
            int objectPort = 0;
            boolean currentRMIserver = false;
            
            if (args.length == 2){
                logger.info("jParmFacadeServer creating a local RMI registry on port "+Registry.REGISTRY_PORT+" ...");
                localRegistry = LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
            }else if (args.length > 1) {
                Integer rmiPort = new Integer(args[1]);
                logger.info("jParmFacadeServer creating a local RMI registry on port "+rmiPort+" ...");
                localRegistry = LocateRegistry.createRegistry(rmiPort.intValue());
                if (args.length ==3){
                    Integer rmiObjectsPort = new Integer(args[2]);
                    logger.info("jParmFacadeServer setting up RMI server objects on port "+rmiObjectsPort+" ...");
                    RMISocketFactory socketFactory = RMISocketFactory.getDefaultSocketFactory();
                    socketFactory.createServerSocket(rmiObjectsPort);
                    objectPort = rmiObjectsPort;
                }
            }
            
            logger.info("jParmFacadeServer creating local object and remote adapter...");
            
            // Export jParmFacade
            jParmFacadeAdaptee = new jParmFacade();
            logger.info("jParmFacadeServer started");
            
            jParmFacadeAdapter = new jParmFacadeAdapter(jParmFacadeAdaptee);
            //A custom port was specified, export the object using the port specified
            if(objectPort!=0){
                jParmFacadeAdapter.unexportObject(jParmFacadeAdapter,true);
                jParmFacadeAdapter.exportObject(jParmFacadeAdapter,objectPort);
            }
            logger.info("jParmFacadeServer publishing service " + jParmFacadeInterface.SERVICENAME + " in local registry...");
            if(currentRMIserver){
                localRegistry.bind(jParmFacadeInterface.SERVICENAME, jParmFacadeAdapter);
            }else{
                localRegistry.rebind(jParmFacadeInterface.SERVICENAME, jParmFacadeAdapter);
            }           
            logger.info("Published jParmFacadeInterface as service " + jParmFacadeInterface.SERVICENAME + ". Ready...");
            
            String statusmessage = "jParmFacadeserver is ready for incoming calls";
            if (args.length > 1) {
                Integer rmiPort = new Integer(args[1]);
                statusmessage += " on rmi registry port "+rmiPort;
                if (args.length ==3){
                    Integer rmiObjectsPort = new Integer(args[2]);
                    statusmessage += " and rmi server object port "+rmiObjectsPort +". Please tunnel/forward both ports for your client to work";
                }
            }
            statusmessage+="...";
            
            logger.info(statusmessage);
        }
        
        catch (Exception e) {
            logger.fatal("jParmFacadeServer failed: " + e);
        }
    }
}
