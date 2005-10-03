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
   private static jOTDBinterface conn;
   private static jTreeMaintenanceInterface tm;
   

    public static void showTreeList (Vector trees)
    {
	try 
	    {
		System.out.println ("treeID|Classif|Creator   |Creationdate        |Type|Campaign|Starttime");
		System.out.println ("------+-------+----------+--------------------+----+--------+------------------");
		
		jOTDBtree aTree;
		Integer treeID;
		
		for (int i = 0; i < trees.size (); ++i) 
		    {
			treeID = (Integer)trees.elementAt (i);
			aTree = (jOTDBtree)conn.getTreeInfo(treeID.intValue ());
			System.out.printf ("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s", aTree.treeID(), aTree.classification, 
					   aTree.creator, aTree.creationDate, aTree.type, aTree.campaign, aTree.starttime); 
			System.out.println ();
		    }	
		System.out.println (trees.size () + " records\n");
	    }
	catch (Exception e)
	    {
		System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	    }
    }

    public static void showNodeList (Vector nodes) 
    {
	try
	    {
		System.out.println ("treeID|nodeID|parent|name           |index|leaf|inst|description");
		System.out.println ("------+------+------+---------------+-----+----+----+------------------");
		
		jOTDBnode aNode;
		
		for (int i = 0; i < nodes.size (); ++i) 
		    {
			aNode = (jOTDBnode)nodes.elementAt (i);
			System.out.printf ("%6d|%6d|%6d|%-15.15s|%5d|%s|%4d|%s",
					   aNode.treeID (), aNode.nodeID (), aNode.parentID (), aNode.name, aNode.index,
					   aNode.leaf, aNode.instances, aNode.description);
			System.out.println ();
		    }	
		System.out.println (nodes.size () + " records\n");
	    }
	catch (Exception e)
	    {
		System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	    }
    }

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
	     conn = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
	     tm = (jTreeMaintenanceInterface) remoteRegistry.lookup (jTreeMaintenanceInterface.SERVICENAME);	     	     

	     // do the test
	     System.out.println ("Starting... ");
	     
	     System.out.println ("Trying to connect to the database");
	     assert conn.connect () : "Connection failed";	
	     assert conn.isConnected () : "Connnection flag failed";
	     
	     System.out.println ("Connection succesful!");	     
	     	     	     
	     System.out.println ("Searching for a Template tree");
	     Vector treeList;
	     treeList = conn.getTreeList ((short)20, (short)0);  // 20 = template, 0 = all
	     showTreeList (treeList);
	     if (treeList.size () == 0) {
		 System.out.println ("No template tree found!");
		 System.exit (0);	   
	     }
	     
	     Integer VTtreeID = new Integer ((Integer)treeList.elementAt (treeList.size () - 1));
	     System.out.println ("Using tree " + VTtreeID + " for the tests.");
	     jOTDBtree treeInfo = conn.getTreeInfo (VTtreeID.intValue ());
	     if (treeInfo.treeID () == 0) {
		 System.out.println ("No such tree found!");
		 System.exit (0);
	     }
	     
	     System.out.println("Trying to get the topnode of the tree");
	     jOTDBnode topNode = tm.getTopNode (VTtreeID.intValue ());
	     System.out.println (topNode.name);
	     
	     System.out.println("Trying to get a collection of items on depth=1");
	     Vector nodeList = tm.getItemList (VTtreeID.intValue (), topNode.nodeID(), 1);
	     showNodeList (nodeList);
	     
	     System.out.println("Trying to get a collection of items on depth=2");
	     nodeList = tm.getItemList (VTtreeID.intValue (), topNode.nodeID(), 2);
	     showNodeList (nodeList);
	     
	     int elemNr = nodeList.size () - 1;
	     System.out.println ("Zooming in on last element");
	     jOTDBnode aNode = tm.getNode (VTtreeID.intValue (),  elemNr);
	     System.out.println (aNode.name);
	     
	     System.out.println ("Trying to classify the tree to operational");
	     boolean actionOK = tm.setClassification (VTtreeID.intValue (), (short)2); // 2 - operational
	     assert actionOK : "Setting classification was NOT succesful";
	     treeInfo = conn.getTreeInfo (VTtreeID.intValue ());
	     System.out.println ("Classification of tree: " + treeInfo.classification);
	     
	     short aTreeState = 400;		
	     System.out.println ("Trying to change the state of the tree to active");
	     actionOK = tm.setTreeState (VTtreeID.intValue (), aTreeState); // 400 = active
	     assert actionOK : "Changing the state to active should have NOT have failed!";
	     treeInfo = conn.getTreeInfo (VTtreeID.intValue ());
	     System.out.println ("State of tree: " + treeInfo.state);
	     
	     System.out.println ("========== Testing manipulation of nodes ==========");
	     
	     System.out.println ("Searching for node 'Virt Telescope'");
	     Vector VtelCol = tm.getItemList (VTtreeID.intValue (), "%Telescope");
	     System.out.println ("Found " + VtelCol.size () + " nodes");
	     jOTDBnode VtelDef = (jOTDBnode)VtelCol.firstElement ();
	     assert VtelDef.nodeID () != 0 : "Node 'Virt Telescope' not found";
	     System.out.println ("Found definition: " + VtelDef.name);
	     
	     // Test the manipulations on the VT
	     System.out.println ("Trying to duplicate the subtree");
	     int nodeID = tm.dupNode (VTtreeID.intValue (), VtelDef.nodeID (), (short)1);
	     System.out.println ("New subtree starts at node: " + nodeID);
	
	     System.out.println ("Trying to retrieve one node");
	     aNode = tm.getNode (VTtreeID.intValue (), nodeID);
	     System.out.println (aNode.instances);
	     System.out.println (aNode.limits);
	     System.out.println ("Modifying the instances and limits");
	     aNode.instances = 5;
	     aNode.limits = "no more limits";
	     tm.saveNode (aNode);
	     System.out.println (aNode.instances);
	     System.out.println (aNode.limits);
	     
	     System.out.println ("Trying to retrieve one node");
	     aNode = tm.getNode (VTtreeID.intValue (), nodeID);
	     System.out.println (aNode.name);
	     System.out.println ("Removing the just created subtree");
	     System.out.println ("nodeID before removal: " + aNode.nodeID ());
	     int orgNodeID = aNode.nodeID();
	     tm.deleteNode (aNode);
	     System.out.println ("nodeID after removal : " + aNode.nodeID ());
	     
	     System.out.println ("Trying to retrieve the deleted node");
	     aNode = tm.getNode (VTtreeID.intValue (), orgNodeID);
	     System.out.println (aNode.nodeID ());
	     
	     // Test the manipulations off the parameters
 	     System.out.println ("Duplicating node Beamformer for index + 1");
 	     Vector BformCol = tm.getItemList (VTtreeID.intValue (), "Beamformer");
 	     jOTDBnode BformDef = (jOTDBnode)BformCol.lastElement ();
 	     System.out.println ("Beamformer has ID " + BformDef.nodeID());
 	     System.out.println (VTtreeID.intValue () +" " + BformDef.nodeID ());
 	     int dupNodeID = BformDef.nodeID ();
 	     System.out.println ("New subtree starts at node: " + dupNodeID);
	     
	     System.out.println ("Getting param info for " + dupNodeID+2 + " and " + dupNodeID+3);
	     jOTDBnode param1 = tm.getNode (VTtreeID.intValue (), dupNodeID+2);
	     jOTDBnode param2 = tm.getNode (VTtreeID.intValue (), dupNodeID+3);
	     System.out.println (param1.name);
	     System.out.println (param2.name);
	     param1.limits = "1.33333";
	     param2.limits = "---1---";
	     System.out.println ("Changing param " + param1.name + " to " + param1.limits);
	     System.out.println ("Changing param " + param2.name + " to " + param2.limits);
	     tm.saveNode (param1);
	     tm.saveNode (param2);
	     
	     // Setting nr instances to some nice values
	     System.out.println ("Setting up tree counts");
	     Vector aNodeCol = tm.getItemList (VTtreeID.intValue (), "RFI dete%");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 40;
	     tm.saveNode (aNode);
	     System.out.println ("RFI detectors  : 40");
	     aNodeCol = tm.getItemList (VTtreeID.intValue (), "Correlator%");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 130;
	     tm.saveNode (aNode);
	     System.out.println ("Correlators    : 130");
	     aNodeCol = tm.getItemList (VTtreeID.intValue (), "Storage");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 86;
	     tm.saveNode (aNode);
	     System.out.println ("Storage        : 86");
	     aNodeCol = tm.getItemList (VTtreeID.intValue (), "Visua%");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 24;
	     tm.saveNode (aNode);
	     System.out.println ("Visualisation  : 24");
	     aNodeCol = tm.getItemList (VTtreeID.intValue (), "Virt Tel%");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 8;
	     tm.saveNode (aNode);
	     System.out.println ("Virt.Telescopes: 8");
	     aNodeCol = tm.getItemList (VTtreeID.intValue (), "Beamfor%");
	     aNode = (jOTDBnode)aNodeCol.firstElement ();
	     aNode.instances = 4;
	     tm.saveNode (aNode);
	     System.out.println ("Beamformers    : 4");
	     
	     // Test copying a template
	     System.out.println ("Trying to copy the template tree");
	     int secondVTtreeID = tm.copyTemplateTree (VTtreeID);
	     System.out.println ("ID of new tree is " + secondVTtreeID);
	     if (secondVTtreeID == 0) {
		 System.out.println (tm.errorMsg());
	     }
	     jOTDBtree VTtree = conn.getTreeInfo (secondVTtreeID);
	     System.out.println (VTtree.creator);
	     
	     // Test creating a full tree of the template tree
	     System.out.println ("Trying to instanciate the copied tree");
	     int VHtreeID = tm.instanciateTree (secondVTtreeID);
	     System.out.println ("ID of new tree is " + VHtreeID);
	     if (VHtreeID == 0) {
		 System.out.println (tm.errorMsg());
	     }
	     jOTDBtree VHtree = conn.getTreeInfo (VHtreeID);
	     System.out.println (VHtree.creator);
	  }
	catch (Exception e)
	    {
		System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	    }
	
     }
}
