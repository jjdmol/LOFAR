import java.rmi.Naming;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBvalue;
import nl.astron.lofar.sas.otb.jotdb2.jTreeMaintenanceInterface;
import nl.astron.lofar.sas.otb.jotdb2.jTreeValueInterface;

class tRemoteTreeMaintenance
{
    private static jOTDBinterface conn;
    private static jTreeMaintenanceInterface tm;
    private static jTreeValueInterface tv;
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "1091";
    public static String RMIRegistryName    =  jOTDBinterface.SERVICENAME;
    public static String RMIMaintenanceName =  jTreeMaintenanceInterface.SERVICENAME; 
    public static String RMIValName         = jTreeValueInterface.SERVICENAME;


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
			aTree = (jOTDBtree)trees.elementAt(1);
			System.out.printf ("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s", aTree.treeID(), aTree.classification, 
					   aTree.creator, aTree.creationDate, aTree.type, aTree.campaign, aTree.starttime); 
			System.out.println ();
		    }	
		System.out.println (trees.size () + " records\n");
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
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
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }

    }

    public static void showValueList (Vector items) 
    {
	try 
	    {
		System.out.println ("name                                         |value |time");
		System.out.println ("---------------------------------------------+------+--------------------");

		jOTDBvalue aValue;

		for (int i = 0; i < items.size(); ++i) 
		    {
			aValue = (jOTDBvalue)items.elementAt (i);
			System.out.printf ("%-45.45s|%-7.7s|%s", aValue.name, aValue.value, aValue.time);
			System.out.println ();
		    }		
		System.out.println (items.size() + " records\n");
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }

    }


   public static void main (String[] args)
     {
	try
	  {

	     System.out.println("Starting... ");

	     // create a remote object
             conn = (jOTDBinterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRegistryName); 
	     tm = (jTreeMaintenanceInterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIMaintenanceName); 

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
	     
	     jOTDBtree treeInfo = (jOTDBtree)treeList.elementAt(treeList.size()-1);
             Integer VTtreeID = treeInfo.treeID();
             System.out.println ("Using tree " + VTtreeID + " for the tests.");
             if (treeInfo.treeID () == 0) {
		 System.out.println ("No such tree found!");
		 System.exit (0);
	     }
	     
	     System.out.println("Trying to get the topnode of the tree");
	     jOTDBnode topNode = tm.getTopNode (treeInfo.treeID());
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
	     treeInfo = conn.getTreeInfo (VTtreeID.intValue (), false);
	     System.out.println ("Classification of tree: " + treeInfo.classification);
	     
	     short aTreeState = 400;		
	     System.out.println ("Trying to change the state of the tree to active");
	     actionOK = tm.setTreeState (VTtreeID.intValue (), aTreeState); // 400 = active
	     assert actionOK : "Changing the state to active should have NOT have failed!";
	     treeInfo = conn.getTreeInfo (VTtreeID.intValue (),false);
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
	     
	     jOTDBnode node1 = tm.getNode (VTtreeID.intValue (), dupNodeID+2);
	     jOTDBnode node2 = tm.getNode (VTtreeID.intValue (), dupNodeID+3);
	     System.out.println ("Getting attributes info for " + node1.nodeID () + " and " + node2.nodeID ());
	     System.out.println (node1.name);
	     System.out.println (node2.name);
	     node1.limits = "1.33333";
	     node2.limits = "---1---";
	     System.out.println ("Changing node " + node1.name + " limits to " + node1.limits);
	     System.out.println ("Changing node " + node2.name + " limits to " + node2.limits);
	     tm.saveNode (node1);
	     tm.saveNode (node2);

	     // Get parameters for a node
	     System.out.println ("Getting param info for " + node1.treeID () + ", " + node1.nodeID ());
	     jOTDBparam param1 = tm.getParam (node1.treeID (), node1.paramDefID ());
	     System.out.println (param1.name);
	     System.out.println (param1.type);
	     System.out.println (param1.pruning);
	     System.out.println (param1.unit);
	     param1.type = 104;
	     tm.saveParam (param1);
	     
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
	     jOTDBtree VTtree = conn.getTreeInfo (secondVTtreeID,false);
	     System.out.println (VTtree.creator);
	     
	     // Test creating a full tree of the template tree
	     System.out.println ("Trying to instanciate the copied tree");
	     int VHtreeID = tm.instanciateTree (secondVTtreeID);
	     System.out.println ("ID of new tree is " + VHtreeID);
	     if (VHtreeID == 0) {
		 System.out.println (tm.errorMsg());
	     }
	     jOTDBtree VHtree = conn.getTreeInfo (VHtreeID,false);
	     System.out.println (VHtree.creator);

	     // Test values
	     tv = (jTreeValueInterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIValName); 
	     tv.setTreeID(4);
	     Vector valueList = tv.searchInPeriod (1868, 0, 
						   "2001-12-31 00:00:00", "2005-12-31 23:59:59", false);
	     if (valueList.size() == 0) {
		 System.out.println ("No items found");
	     }
	     else {
		 showValueList (valueList);
	     }
      
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }

	
     }
}
