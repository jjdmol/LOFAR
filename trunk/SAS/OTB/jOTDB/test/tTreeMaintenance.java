import jOTDB.jOTDBnode;
import jOTDB.jVICnodeDef;
import jOTDB.jTreeMaintenance;
import jOTDB.jOTDBconnection;
import jOTDB.jOTDBtree;
import java.util.Vector;
import java.lang.Integer;

class tTreeMaintenance
{
   static
     {
	System.loadLibrary ("jotdb");
     }
   
   
   public static void main (String[] args)
     {
	tTreeMaintenance ttreemain = new tTreeMaintenance ();
	ttreemain.test ();
     }
   
   public void test ()
     {
	// do the test
	System.out.println ("Starting... ");
	
	// create a jOTDBconnection
	jOTDBconnection conn = new jOTDBconnection ("paulus","boskabouter","otdbtest");
	
	System.out.println ("Trying to connect to the database");
	assert conn.connect () : "Connection failed";	
	assert conn.isConnected () : "Connnection flag failed";

	System.out.println ("Connection succesful!");

	System.out.println ("Trying to construct a TreeMaintenance object");
	jTreeMaintenance tm = new jTreeMaintenance ();
	
	System.out.println ("Searching for a Template tree");
	Vector treeList;
	treeList = conn.getTreeList ((short)20, (short)0);  // 20 = template, 0 = all
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
//	showNodeList(nodeList);

	System.out.println("Trying to get a collection of items on depth=2");
	nodeList = tm.getItemList (VTtreeID.intValue (), topNode.nodeID(), 2);
//	showNodeList(nodeList);
		
	int elemNr = nodeList.size () - 1;
	System.out.println ("Zooming in on last element");
	jOTDBnode node = (jOTDBnode)nodeList.elementAt (elemNr);
	jOTDBnode aNode = tm.getNode (VTtreeID.intValue (), node.nodeID ());
	System.out.println (aNode.name);

	System.out.println ("Trying to classify the tree to operational");
	boolean actionOK = tm.setClassification (VTtreeID.intValue (), (short)2);
	if (actionOK) 
	  {
	     System.out.println("Setting classification was succesful");
	  }
	else 
	  {
	     System.out.println("Setting classification was NOT succesful");
	  }
	treeInfo = conn.getTreeInfo (VTtreeID.intValue ());
	System.out.println ("Classification of tree: " + treeInfo.classification);
		
	short aTreeState = 400;		
	System.out.println ("Trying to change the state of the tree to active");
	actionOK = tm.setTreeState (VTtreeID.intValue (), aTreeState);
	assert actionOK : "Changing the state to active should have NOT have failed!";
	treeInfo = conn.getTreeInfo (VTtreeID.intValue ());
	System.out.println ("State of tree: " + treeInfo.state);
   
	System.out.println ("========== Testing manipulation of nodes ==========");
	
	System.out.println ("Searching for node 'Virt Telescope'");
	Vector VtelCol = tm.getItemList (VTtreeID.intValue (), "%Telescope");
	System.out.println ("Found " + VtelCol.size() + " nodes");
	jOTDBnode VtelDef = (jOTDBnode) VtelCol.firstElement ();
	assert VtelDef.nodeID () != 0 : "Node 'Virt Telescope' not found";
	System.out.println ("Found definition: " + VtelDef.name);
	
/*	// Test the manipulations on the VT
	System.out.println("Trying to duplicate the subtree");
	nodeIDType	nodeID = tm.dupNode(VTtreeID, VtelDef.nodeID(), 1);
	System.out.println_STR("New subtree starts at node: " << nodeID);
	
	System.out.println("Trying to retrieve one node");
	aNode = tm.getNode (VTtreeID, nodeID);
		System.out.println_STR(aNode);
	System.out.println("Modifying the instances and limits");
	aNode.instances = 5;
	aNode.limits = "no more limits";
	tm.saveNode(aNode);
	System.out.println_STR(aNode);
	
	System.out.println("Trying to retrieve one node");
	aNode = tm.getNode (VTtreeID, nodeID);
	System.out.println_STR(aNode);
	System.out.println("Removing the just created subtree");
	System.out.println_STR("nodeID before removal:" << aNode.nodeID());
	nodeIDType		orgNodeID = aNode.nodeID();
	tm.deleteNode(aNode);
	System.out.println_STR("nodeID after removal :" << aNode.nodeID());
	
	System.out.println("Trying to retrieve the deleted node");
	aNode = tm.getNode (VTtreeID, orgNodeID);
	System.out.println_STR(aNode);

	// Test the manipulations off the parameters
	System.out.println("Duplicating node Beamformer for index=3");
	vector<OTDBnode>	BformCol = tm.getItemList(VTtreeID, "Beamformer");
	OTDBnode	BformDef = BformCol[0];
		System.out.println_STR("Beamformer has ID " << BformDef.nodeID());
	nodeIDType	dupNodeID = tm.dupNode(VTtreeID, BformDef.nodeID(), 3);
	System.out.println_STR("New subtree starts at node: " << dupNodeID);
	
	System.out.println_STR("Getting param info for " << dupNodeID+2 << " and " 
			       << dupNodeID+3);
	OTDBnode	param1 = tm.getNode (VTtreeID, dupNodeID+2);
	OTDBnode	param2 = tm.getNode (VTtreeID, dupNodeID+3);
	System.out.println_STR(param1);
		System.out.println_STR(param2);
	param1.limits = "1.33333";
	param2.limits = "---1---";
	System.out.println_STR("Changing param " << param1.name << " to " << param1.limits);
	System.out.println_STR("Changing param " << param2.name << " to " << param2.limits);
	tm.saveNode(param1);
		tm.saveNode(param2);

	// Setting nr instances to some nice values
	System.out.println("Setting up tree counts")
	  vector<OTDBnode>	aNodeCol = tm.getItemList(VTtreeID, "RFI dete%");
	aNode = *(aNodeCol.begin());
	aNode.instances = 40;
	tm.saveNode(aNode);
	System.out.println("RFI detectors  : 40");
	aNodeCol = tm.getItemList(VTtreeID, "Correlator%");
	aNode = *(aNodeCol.begin());
	aNode.instances = 130;
	tm.saveNode(aNode);
	System.out.println("Correlators    : 130");
	aNodeCol = tm.getItemList(VTtreeID, "Storage");
	aNode = *(aNodeCol.begin());
	aNode.instances = 86;
	tm.saveNode(aNode);
	System.out.println("Storage        : 86");
	aNodeCol = tm.getItemList(VTtreeID, "Visua%");
	aNode = *(aNodeCol.begin());
	aNode.instances = 24;
	tm.saveNode(aNode);
	System.out.println("Visualisation  : 24");
	aNodeCol = tm.getItemList(VTtreeID, "Virt Tel%");
	aNode = *(aNodeCol.begin());
	aNode.instances = 8;
	tm.saveNode(aNode);
	System.out.println("Virt.Telescopes: 8");
	aNodeCol = tm.getItemList(VTtreeID, "Beamfor%");
	aNode = *(aNodeCol.begin());
	aNode.instances = 4;
	tm.saveNode(aNode);
	System.out.println("Beamformers    : 4");

	// Test copying a template
	System.out.println("Trying to copy the template tree");
	treeIDType	 secondVTtreeID = tm.copyTemplateTree(VTtreeID);
	System.out.println_STR("ID of new tree is " << secondVTtreeID);
	if (!secondVTtreeID) {
	   LOG_ERROR(tm.errorMsg());
	}
	OTDBtree	VTtree = conn.getTreeInfo(secondVTtreeID);
	System.out.println_STR(VTtree);
	
	// Test creating a full tree of the template tree
	System.out.println("Trying to instanciate the copied tree");
	treeIDType	 VHtreeID = tm.instanciateTree(secondVTtreeID);
	System.out.println_STR("ID of new tree is " << VHtreeID);
	if (!VHtreeID) {
	   LOG_ERROR(tm.errorMsg());
	}
		OTDBtree	VHtree = conn.getTreeInfo(VHtreeID);
	System.out.println_STR(VHtree);
*/
     }
}




