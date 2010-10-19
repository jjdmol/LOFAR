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

package nl.astron.lofar.sas.otb.jotdb3;

import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;



public class jOTDBserver {
    
    static Logger logger = Logger.getLogger(jOTDBserver.class);

    // Keep a static reference to the access object to prevent the server from exitting.
    static jOTDBaccessInterface access = null;

    public static void main(String[] args)  {
        try {
            String logConfig = "jOTDB3.log_prop";
            
            PropertyConfigurator.configure(logConfig);
//            jInitCPPLogger aCPPLogger=new jInitCPPLogger(logConfig);
            logger.info("jOTDBServer started. LogPropFile: "+ logConfig);
            logger.info("java.library.path:"+ System.getProperty("java.library.path"));

/*
	    if (System.getSecurityManager () == null) {
            System.out.println ("No security mananger is running, will start one now...");
            System.setProperty("java.security.policy","client.policy");
            System.setSecurityManager (new RMISecurityManager ());
	       }
*/

            // needed args for an rmi connection:

            // given to this server:
            // 1) database username
            // 2) database username pwd
            // 4) host where database runs
            // 5) hostname where this server runs
            // 6) rmi port used by the server for client contact(OPTIONAL)
            // 7) rmi port used for firewall/tunneling purposes (OPTIONAL)


            // given by client that wants to connect :
            // 1) user name
            // 2) user pwd

            //

            if (args.length < 2) {
                System.out.println("Usage: java -jar jOTDBnewServer.jar <db-hostname> <hostname> <rmiportnumber-OPTIONAL> <rmi objects portnumber for firewall/tunneling purposes-OPTIONAL>");
                System.exit(0);
            }
            
            
            String aHostName = new String(args[1]);
            
            logger.info("Running on: "+aHostName);
            System.setProperty("java.rmi.server.hostname", aHostName);



            // Create the access object that will take care of registering to rmi
            int rmiPort = 0;
            int rmiObjectPort=0;
            Registry registry = null;
            try {
                rmiPort = Integer.parseInt(args[2]);
            } catch (NumberFormatException ex) {
                logger.error("jOTDBserver rmiPort error: "+args[2]+ "  " + ex);
            }
            try {
                rmiObjectPort = Integer.parseInt(args[3]);
            } catch (NumberFormatException ex) {
                logger.error("jOTDBserver rmiObjectPort error: "+args[3]+ "  " + ex);
            }

            if (rmiPort <= 0) {
                logger.info("jOTDBaccess creating a local RMI registry on port " + Registry.REGISTRY_PORT + " ...");
                registry = LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
            } else {
                logger.info("jOTDBserver creating a local RMI registry on port "+rmiPort+" ...");
                registry = LocateRegistry.createRegistry(rmiPort);
                logger.info("jOTDBserver setting up RMI server objects on port "+rmiObjectPort+" ...");
            }

            access = new jOTDBaccess(args[0], rmiPort,rmiObjectPort,registry);
            jOTDBaccessInterface stub =
                (jOTDBaccessInterface) UnicastRemoteObject.exportObject(access, rmiObjectPort);

            if (stub != null) {
                logger.info("set up new jOTDBaccess: " + stub);
                logger.info("jOTDBaccess publishing service " + jOTDBaccessInterface.SERVICENAME + " in local registry...");
                registry.rebind(jOTDBaccessInterface.SERVICENAME, stub);
                logger.info("Published jOTDBaccess as service " + jOTDBaccessInterface.SERVICENAME + ". Ready...");
            } else {
                logger.info("set up new jOTDBaccess FAILED");
            }
        } catch (RemoteException ex) {
            logger.info("jOTDBnewServer failed: " + ex);
        }       
    }
}
