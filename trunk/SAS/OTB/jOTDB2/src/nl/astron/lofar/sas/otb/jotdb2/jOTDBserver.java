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
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;



public class jOTDBserver
{
   private static jOTDBconnection jOTDBconnAdaptee;
   private static jOTDBadapter jOTDBconnAdapter;
   private static jTreeMaintenance jTreeMainAdaptee;
   private static jTreeMaintenanceAdapter jTreeMainAdapter;
   private static jTreeValue jTreeValueAdaptee;
   private static jTreeValueAdapter jTreeValueAdapter;
   private static jConverter jConverterAdaptee;
   private static jConverterAdapter jConverterAdapter;

   static
     {
	System.loadLibrary("jotdb2");
     }

   static Logger logger = Logger.getLogger(jOTDBserver.class);
   
    public static void main (String[] args)  {
    try {   
        String logConfig = args[0]+".log_prop";
       
        PropertyConfigurator.configure(logConfig);
        logger.info("jOTDBServer started. LogPropFile: "+ logConfig);
        jInitCPPLogger aCPPLogger=new jInitCPPLogger(logConfig);
        
//	     if (System.getSecurityManager () == null) 
//	       {
//		  System.out.println ("No security mananger is running, will start one now...");
//		  System.setSecurityManager (new RMISecurityManager ());
//	       }

	     if (args.length < 4) 
		 {
		     System.out.println ("Usage: java -jar jOTDBserver.jar <username> <password> <database> <hostname> <portnumber-OPTIONAL>");
		     System.exit(0);
		 }
	     

	     String aHostName = new String(args[3]);
	     
	     logger.info("Running on: "+aHostName);
	     System.setProperty ("java.rmi.server.hostname", aHostName);

	     System.out.println ("jOTDBserver creating a local RMI registry...");
	     Registry localRegistry;
	     if (args.length == 4) 
		 localRegistry = LocateRegistry.createRegistry (Registry.REGISTRY_PORT);
	     else {
		 Integer i = new Integer (args[4]);
		 localRegistry = LocateRegistry.createRegistry (i.intValue());
	     }
	     
	     logger.info("jOTDBserver creating local object and remote adapter...");	     
	     
	     // Export jOTDBconnection
	     jOTDBconnAdaptee = new jOTDBconnection (args[0], args[1], args[2]);
	     jOTDBconnAdapter = new jOTDBadapter (jOTDBconnAdaptee);
	     
	     logger.info("jOTDBserver publishing service " + jOTDBinterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jOTDBinterface.SERVICENAME, jOTDBconnAdapter);
	     
	     logger.info("Published jOTDBinterface as service " + jOTDBinterface.SERVICENAME + ". Ready...");	     

	     // Export jTreeMaintenance    	     
	     jTreeMainAdaptee = new jTreeMaintenance ();
	     jTreeMainAdapter = new jTreeMaintenanceAdapter (jTreeMainAdaptee);
	     
	     logger.info("jOTDBserver publishing service " + jTreeMaintenanceInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jTreeMaintenanceInterface.SERVICENAME, jTreeMainAdapter);
	     
	     // Export jTreeValue
	     jTreeValueAdaptee = new jTreeValue ();
	     jTreeValueAdapter = new jTreeValueAdapter (jTreeValueAdaptee);
	     
	     logger.info("jOTDBserver publishing service " + jTreeValueInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jTreeValueInterface.SERVICENAME, jTreeValueAdapter);

	     logger.info("Published jTreeValueInterface as service " + jTreeValueInterface.SERVICENAME + ". Ready...");	     

	     // Export jConverter
	     jConverterAdaptee = new jConverter ();
	     jConverterAdapter = new jConverterAdapter (jConverterAdaptee);
	     
	     logger.info("jOTDBserver publishing service " + jConverterInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jConverterInterface.SERVICENAME, jConverterAdapter);

	     logger.info("Published jConverterInterface as service " + jConverterInterface.SERVICENAME + ". Ready...");
	  }
	
	catch (Exception e)
	  {
	     logger.fatal("jOTDB server failed: " + e);
	  }
     }   	
}
