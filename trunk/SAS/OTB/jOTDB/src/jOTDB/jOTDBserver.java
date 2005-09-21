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

import jOTDB.jOTDBconnection;
import jOTDB.jOTDBadapter;
import jOTDB.jOTDBinterface;
import java.rmi.Naming;
import java.rmi.registry.*; 
import java.rmi.RMISecurityManager;

public class jOTDBserver
{
   private static jOTDBconnection adaptee;
   private static jOTDBadapter adapter;

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
	     
	     System.out.println ("jOTDBserver creating a local RMI registry on the default port...");
	     Registry localRegistry = LocateRegistry.createRegistry (Registry.REGISTRY_PORT);
	     
	     System.out.println ("jOTDBserver creating local object and remote adapter...");	     
	     if (args.length != 3) 
		 {
		     System.out.println ("Usage: java -Djava.rmi.server.hostname=<hostname> jOTDB.jOTDBserver <username> <password> <database>");
		     System.exit(0);
		 }
	     adaptee = new jOTDBconnection (args[0], args[1], args[2]);
	     adapter = new jOTDBadapter (adaptee);
	     
	     System.out.println ("jOTDBserver publishing service " + jOTDBinterface.SERVICENAME + " in local registry...");
	     localRegistry.rebind (jOTDBinterface.SERVICENAME, adapter);
	     
	     System.out.println ("Published jOTDBinterface as service " + jOTDBinterface.SERVICENAME + ". Ready...");	     
	  }
	
	catch (Exception e)
	  {
	     System.out.println ("jOTDB server failed: " + e);
	  }
     }   	
}
