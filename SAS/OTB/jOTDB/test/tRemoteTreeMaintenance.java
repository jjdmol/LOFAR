import jOTDB.jOTDBtree;
import jOTDB.jOTDBnode;
import jOTDB.jVICnodeDef;
import java.util.Vector;
import java.lang.Integer;
import java.rmi.registry.*;
import jOTDB.jOTDBinterface;
import jOTDB.jTreeMaintenanceInterface;
import java.rmi.RMISecurityManager;

class tRemoteTreeMaintenance
{
   private static jOTDBinterface remoteOTDB;
   private static jTreeMaintenanceInterface remoteTreeMaintenance;
   
   public static void main (String[] args)
     {
	try
	  {
	     if (args.length != 1)
	       {
		  System.out.println ("Usage: java tRemoteTreeMaintenance <rmi-registry_hostname>");
		  System.exit(0);
	       }

	     System.out.println("Starting... ");

	     // create a remote object
	     Registry remoteRegistry = LocateRegistry.getRegistry (args[0]);
	     remoteOTDB = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
	     remoteTreeMaintenance = (jTreeMaintenanceInterface) remoteRegistry.lookup (jTreeMaintenanceInterface.SERVICENAME);
	     	     
	     // do the test	
	     System.out.println("Trying to connect to the database");
	     assert remoteOTDB.connect() : "Connection failed";	
	     assert remoteOTDB.isConnected() : "Connnection flag failed";
	     
	     System.out.println("Connection succesful!");
	     
	     System.out.println ("Searching for a Template tree");
	     Vector treeList;
	     treeList = remoteOTDB.getTreeList ((short)20, (short)0);  // 20 = template, 0 = all
	     if (treeList.size () == 0) {
		System.out.println ("No template tree found!");
		System.exit (0);	   
	     }
	     
	     Integer VTtreeID = new Integer ((Integer)treeList.elementAt (treeList.size () - 1));
	     System.out.println ("Using tree " + VTtreeID + " for the tests.");
	     jOTDBtree treeInfo = remoteOTDB.getTreeInfo (VTtreeID.intValue ());
	     if (treeInfo.treeID () == 0) {
		System.out.println ("No such tree found!");
		System.exit (0);
	     }
	     
	     System.out.println("Trying to get the topnode of the tree");
	     jOTDBnode topNode = remoteTreeMaintenance.getTopNode (VTtreeID.intValue ());
	     System.out.println (topNode.name);
	     
	     System.out.println("Trying to get a collection of items on depth=1");
	     Vector nodeList = remoteTreeMaintenance.getItemList (VTtreeID.intValue (), topNode.nodeID(), 1);
	     //	showNodeList(nodeList);
	     
	     System.out.println("Trying to get a collection of items on depth=2");
	     nodeList = remoteTreeMaintenance.getItemList (VTtreeID.intValue (), topNode.nodeID(), 2);
	     //	showNodeList(nodeList);
	     
	     int elemNr = nodeList.size () - 1;
	     System.out.println ("Zooming in on last element");
	     jOTDBnode node = (jOTDBnode)nodeList.elementAt (elemNr);
	     jOTDBnode aNode = remoteTreeMaintenance.getNode (VTtreeID.intValue (), node.nodeID ());
	     System.out.println (aNode.name);
	     
	     System.out.println ("Trying to classify the tree to operational");
	     boolean actionOK = remoteTreeMaintenance.setClassification (VTtreeID.intValue (), (short)2);
	     if (actionOK) 
	       {
		  System.out.println("Setting classification was succesful");
	       }
	     else 
	       {
		  System.out.println("Setting classification was NOT succesful");
	       }
	     treeInfo = remoteOTDB.getTreeInfo (VTtreeID.intValue ());
	     System.out.println ("Classification of tree: " + treeInfo.classification);
	     
	     short aTreeState = 400;		
	     System.out.println ("Trying to change the state of the tree to active");
	     actionOK = remoteTreeMaintenance.setTreeState (VTtreeID.intValue (), aTreeState);
	     assert actionOK : "Changing the state to active should have NOT have failed!";
	     treeInfo = remoteOTDB.getTreeInfo (VTtreeID.intValue ());
	     System.out.println ("State of tree: " + treeInfo.state);
	     
	  }
	catch (Exception e)
	  {
	     System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	  }
	
     }
}
