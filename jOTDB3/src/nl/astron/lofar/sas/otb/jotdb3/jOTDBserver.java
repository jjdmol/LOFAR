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

import com.darwinsys.lang.GetOpt;
import com.darwinsys.lang.GetOptDesc;
import java.io.File;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;


public class jOTDBserver {

    static
    {
        System.loadLibrary("jotdb3");
    }
    
    static {
 
        int delay = 1000 * 60; // delay for 1 minute
        int repeat = delay * 5; // repeat every 5 minutes
 
        Timer gcTimer = new Timer();
        gcTimer.scheduleAtFixedRate(new TimerTask() {
 
        @Override
        public void run() {
//          System.out.println("Running Gargabe-Collector");
            System.gc();
        }
   
        }, delay, repeat);
  
 }

    static Logger logger = Logger.getLogger(jOTDBserver.class);

    // Keep a static reference to the access object to prevent the server from exitting.
    static jOTDBaccessInterface access = null;

    public static void main(String[] argv)  {

        String logConfig = "jOTDB3.log_prop";

        try {
            File f = new File(logConfig);
            if (f.exists()) {
                PropertyConfigurator.configure(logConfig);
            } else {
                logConfig = File.separator+"opt"+File.separator+"sas"+File.separator+"otb"+File.separator+"etc"+File.separator+logConfig;
                f = new File(logConfig);
                if (f.exists()) {
                    PropertyConfigurator.configure(logConfig);
                } else {
                    logger.error("jOTDB3.log_prop not found.");
                }
            }
            logger.info("jOTDBServer started. LogPropFile: "+ logConfig);
            logger.info("java.library.path:"+ System.getProperty("java.library.path"));
            
            
            try {
                jInitCPPLogger aCPPLogger= new jInitCPPLogger(logConfig);
            } catch (Exception ex) {
                System.out.println("Error: "+ ex);
            }

/*
	    if (System.getSecurityManager () == null) {
            System.out.println ("No security mananger is running, will start one now...");
            System.setProperty("java.security.policy","client.policy");
            System.setSecurityManager (new RMISecurityManager ());
	       }
*/

            // needed args for an rmi connection:

            // given to this server:
            // 1) host where database runs
            // 2) hostname where this server runs
            // 3) rmi port used by the server for client contact(OPTIONAL)
            // 4) rmi port used for firewall/tunneling purposes (OPTIONAL)


            // given by client that wants to connect :
            // 1) user name
            // 2) user pwd

            //

            
            String dbHostName     = "";
            String serverHostName = "";
            int rmiPort           = 0;
            int rmiObjectPort     = 0;
            boolean errs          = false;


            // try to resolve the arguments
            GetOptDesc options[] = {
                new GetOptDesc('s', "serverHostName", true),
                new GetOptDesc('d', "dbHostName", true),
                new GetOptDesc('p', "rmiPort", true),
                new GetOptDesc('o', "rmiObjectPort", true),
                new GetOptDesc('h', "help", false)
            };

            GetOpt parser = new GetOpt(options);
            Map optionsFound = parser.parseArguments(argv);
            Iterator it = optionsFound.keySet().iterator();
            while (it.hasNext()) {
                String key = (String)it.next();
                char c = key.charAt(0);
                switch (c) {
                    case 's':
                        serverHostName = (String)optionsFound.get(key);
                        break;
                    case 'd':
                        dbHostName = (String)optionsFound.get(key);
                        break;
                    case 'p':
                        rmiPort = Integer.parseInt((String)optionsFound.get(key));
                        break;
                    case 'o':
                        rmiObjectPort = Integer.parseInt((String)optionsFound.get(key));
                        break;
                    case 'h':
                        errs = true;
                        break;
                    default:
                        throw new IllegalStateException(
                                "Unexpected option character: "+ c);
                }
            }
            if (errs) {
                System.out.println("Usage: java -jar jOTDBnewServer.jar  -p rmiPort -o rmiObjectPort [-s server] [-d database server]  [-h]");
                System.out.println("");
                System.out.println("       The rmi port is needed for rmi traffic and must be unique fior each server");
                System.out.println("       The rmiObjectPort is needed for firewall/tunneling java objects and must be unique for each server");
                System.out.println("       When the servername is not provided the server will try to resolve one itself for the local machine");
                System.out.println("       When the database server name is not provided it is assumed to run on the localmachine");
            }

            if (rmiPort <= 0 || rmiObjectPort <= 0 || rmiPort==rmiObjectPort) {
                logger.fatal("Invalid rmiPort or rmiObjectPort provided");
                System.exit(0);
            }

            if (serverHostName.equals("")) {
                try {
                    java.net.InetAddress localMachine = java.net.InetAddress.getLocalHost();
                    logger.info("Hostname of local machine: " + localMachine.getHostName());
                    serverHostName=localMachine.getHostName();
                }
                catch (java.net.UnknownHostException ex) {
                    logger.error("Couldn't resolve hostname" + ex);
                    logger.fatal("restart using -h hostname switch");
                    System.exit(0);
                }
            }

            System.setProperty("java.rmi.server.hostname", serverHostName);

            // if dbHostname not provided, the server runs on same machin as the database
            if (dbHostName.equals("")) dbHostName=serverHostName;

            // Create the access object that will take care of registering to rmi
            logger.info("jOTDBserver creating a local RMI registry on port "+rmiPort+" ...");
            Registry registry=null;
            try {
                registry = LocateRegistry.createRegistry(rmiPort);
            } catch (RemoteException ex) {
                logger.fatal("Error creating registry port:" + ex);
                System.exit(0);
            }
            logger.info("jOTDBserver setting up RMI server objects on port "+rmiObjectPort+" ...");

            access = new jOTDBaccess(dbHostName, serverHostName, rmiPort, rmiObjectPort, registry);
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
