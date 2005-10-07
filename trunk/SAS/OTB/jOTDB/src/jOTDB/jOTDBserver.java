//#  jOTDBserver.java: The RMI server of the OTDB database.
//#
//#  Copyright (C) 2002-2004
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
  
package jOTDB;

import jOTDB.jConverter;
import jOTDB.jConverterInterface;
import jOTDB.jConverterAdapter;
import jOTDB.jTreeValue;
import jOTDB.jTreeValueInterface;
import jOTDB.jTreeValueAdapter;
import jOTDB.jTreeMaintenance;
import jOTDB.jTreeMaintenanceInterface;
import jOTDB.jTreeMaintenanceAdapter;
import jOTDB.jOTDBconnection;
import jOTDB.jOTDBadapter;
import jOTDB.jOTDBinterface;
import java.rmi.Naming;
import java.rmi.registry.*; 
import java.rmi.RMISecurityManager;

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
	System.loadLibrary("jotdb");
     }
   
   public static void main (String[] args) 
     {
	try
	  {
//	     if (System.getSecurityManager () == null) 
//	       {
//		  System.out.println ("No security mananger is running, will start one now...");
//		  System.setSecurityManager (new RMISecurityManager ());
//	       }

	     if (args.length < 3) 
		 {
		     System.out.println ("Usage: java -Djava.rmi.server.hostname=<hostname> jOTDB.jOTDBserver <username> <password> <database> <portnumber-OPTIONAL>");
		     System.exit(0);
		 }
	     
	     System.out.println ("jOTDBserver creating a local RMI registry...");
	     Registry localRegistry;
	     if (args.length == 3) 
		 localRegistry = LocateRegistry.createRegistry (Registry.REGISTRY_PORT);
	     else {
		 Integer i = new Integer (args[3]);
		 localRegistry = LocateRegistry.createRegistry (i.intValue());
	     }
	     
	     System.out.println ("jOTDBserver creating local object and remote adapter...");	     
	     
	     // Export jOTDBconnection
	     jOTDBconnAdaptee = new jOTDBconnection (args[0], args[1], args[2]);
	     jOTDBconnAdapter = new jOTDBadapter (jOTDBconnAdaptee);
	     
	     System.out.println ("jOTDBserver publishing service " + jOTDBinterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jOTDBinterface.SERVICENAME, jOTDBconnAdapter);
	     
	     System.out.println ("Published jOTDBinterface as service " + jOTDBinterface.SERVICENAME + ". Ready...");	     

	     // Export jTreeMaintenance    	     
	     jTreeMainAdaptee = new jTreeMaintenance ();
	     jTreeMainAdapter = new jTreeMaintenanceAdapter (jTreeMainAdaptee);
	     
	     System.out.println ("jOTDBserver publishing service " + jTreeMaintenanceInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jTreeMaintenanceInterface.SERVICENAME, jTreeMainAdapter);
	     
	     // Export jTreeValue
	     jTreeValueAdaptee = new jTreeValue ();
	     jTreeValueAdapter = new jTreeValueAdapter (jTreeValueAdaptee);
	     
	     System.out.println ("jOTDBserver publishing service " + jTreeValueInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jTreeValueInterface.SERVICENAME, jTreeValueAdapter);

	     System.out.println ("Published jTreeValueInterface as service " + jTreeValueInterface.SERVICENAME + ". Ready...");	     

	     // Export jConverter
	     jConverterAdaptee = new jConverter ();
	     jConverterAdapter = new jConverterAdapter (jConverterAdaptee);
	     
	     System.out.println ("jOTDBserver publishing service " + jConverterInterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jConverterInterface.SERVICENAME, jConverterAdapter);

	     System.out.println ("Published jConverterInterface as service " + jConverterInterface.SERVICENAME + ". Ready...");
	  }
	
	catch (Exception e)
	  {
	     System.out.println ("jOTDB server failed: " + e);
	  }
     }   	
}
