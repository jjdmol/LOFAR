import jOTDB.jOTDBtree;
//import jOTDB.jOTDBconnection;
import java.util.Vector;
import java.lang.Integer;
import java.rmi.registry.*;
import jOTDB.jOTDBinterface;
import java.rmi.RMISecurityManager;

class tRemoteConnection
{
   private static jOTDBinterface remoteOTDB;
   
   public static void main (String[] args)
     {
	try
	  {
	     if (args.length != 1)
	       {
		  System.out.println ("Usage: java tRemoteConnection <rmi-registry_hostname>");
		  System.exit(0);
	       }

	     System.out.println("Starting... ");

	     // create a remote object
	     Registry remoteRegistry = LocateRegistry.getRegistry (args[0]);
	     remoteOTDB = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
	     
	     System.out.println (remoteOTDB);
					    
	     // do the test	
	     System.out.println("Trying to connect to the database");
	     assert remoteOTDB.connect() : "Connection failed";	
	     assert remoteOTDB.isConnected() : "Connnection flag failed";
	     
	     System.out.println("Connection succesful!");
	     
	     System.out.println("getTreeList(0,0)");
	     Vector treeList;
	     treeList = remoteOTDB.getTreeList((short)0, (short)0);
	     if (treeList.size() == 0) 
	       {
		  System.out.println("Error:" + remoteOTDB.errorMsg());
		  System.exit (0);
	       }
	     else 
	       {
		  System.out.println("Collected tree list");
		  //showTreeList(treeList);
	       }
	     
	     System.out.println("getTreeInfo(treeList.elementAt(1))");
	     Integer i = new Integer((Integer)treeList.elementAt(1));
	     jOTDBtree tInfo = remoteOTDB.getTreeInfo(i.intValue());
	     if (tInfo.treeID()==0) 
	       {
		  System.out.println("No such tree found!");
	       }
	     else 
	       {
		  System.out.println(tInfo.classification);
		  System.out.println(tInfo.creator);
		  System.out.println(tInfo.creationDate);	
		  System.out.println(tInfo.type);
		  System.out.println(tInfo.state);
		  System.out.println(tInfo.originalTree);
		  System.out.println(tInfo.campaign);	
		  System.out.println(tInfo.starttime);
		  System.out.println(tInfo.stoptime);
		  System.out.println(tInfo.TreeID());	   
	       }
	  }
	catch (Exception e)
	  {
	     System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	  }
	
     }
}
